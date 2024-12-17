#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    perror("Socket failed");
    return 1;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    close(server_fd);
    return 1;
  }

  std::cout << "Server listening on port " << PORT << "...\n";

  while (true) {
    int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_fd < 0) {
      perror("Accept failed");
      close(server_fd);
      return 1;
    }

    std::cout << "Client connected.\n";

    while (true) {
      memset(buffer, 0, BUFFER_SIZE);
      int bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
      if (bytes_read <= 0) {
        std::cout << "Client disconnected.\n";
        break;
      }
      std::string input(buffer);

      std::cout << "Received: " << input << "\n";

      std::string response = (input == "ping") ? "pong" : "error";
      send(client_fd, response.c_str(), response.size(), 0);
    }

    close(client_fd);
  }
  
  close(server_fd);

  return 0;
}
