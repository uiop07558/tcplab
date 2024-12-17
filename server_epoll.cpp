#include <iostream>
#include <string>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int set_non_blocking(int socket) {
  return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK);
}

int main() {
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    perror("Socket failed");
    return 1;
  }

  set_non_blocking(server_fd);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    perror("Listen failed");
    close(server_fd);
    return 1;
  }

  int epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    perror("Epoll create failed");
    close(server_fd);
    return 1;
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = server_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

  std::cout << "Server listening on port " << PORT << "...\n";

  struct epoll_event events[MAX_EVENTS];

  while (true) {
    int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    for (int i = 0; i < event_count; ++i) {
      if (events[i].data.fd == server_fd) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket >= 0) {
          set_non_blocking(new_socket);
          event.events = EPOLLIN;
          event.data.fd = new_socket;
          epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event);
          std::cout << "New client connected.\n";
        }
      } else {
        int client_fd = events[i].data.fd;
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);

        if (bytes_read <= 0) {
          std::cout << "Client disconnected.\n";
          close(client_fd);
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        } else {
          std::string input(buffer);

          std::cout << "Received: " << input << "\n";

          std::string response = (input == "ping") ? "pong" : "error";
          send(client_fd, response.c_str(), response.size(), 0);
        }
      }
    }
  }

  close(server_fd);
  close(epoll_fd);
  return 0;
}
