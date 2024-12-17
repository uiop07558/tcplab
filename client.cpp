#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int client_socket;
  struct sockaddr_in server_address;
  char buffer[BUFFER_SIZE] = {0};
  std::string message = "ping";

  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    perror("Socket creation failed");
    return 1;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(SERVER_PORT);

  if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
    perror("Invalid address/Address not supported");
    close(client_socket);
    return 1;
  }

  if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
    perror("Connection failed");
    close(client_socket);
    return 1;
  }

  std::cout << "Connected to the server.\n";

  while (true) {
    if (send(client_socket, message.c_str(), message.size(), 0) < 0) {
      perror("Send failed");
      break;
    }
    std::cout << "Sent: " << message << "\n";

    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
      std::cout << "Server disconnected.\n";
      break;
    }

    std::cout << "Received: " << buffer << "\n";

    sleep(1);
  }

  close(client_socket);
  return 0;
}
