#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 9000

int send_all(int s, char *buf, int len);
int receive_all(int s, char *buf, int len);

int main() {
  printf("server starting...\n");

  // Create Socket
  int ss = socket(PF_INET, SOCK_STREAM, 0);
  if (ss == -1) {
    perror("failed to create socket");
    return EXIT_FAILURE;
  }

  // Socket Option
  int opt = 1;
  if (setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    perror("setsockopt SO_REUSEADDR failed");
    close(ss);
    return EXIT_FAILURE;
  }

  // Configure Address & Port
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  if (inet_aton("127.0.0.1", &addr.sin_addr) == 0) {
    perror("invalid address");
    close(ss);
    return EXIT_FAILURE;
  }
  addr.sin_port = htons(PORT);

  // Bind
  if (bind(ss, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind failed");
    close(ss);
    return EXIT_FAILURE;
  }

  // Listen
  if (listen(ss, 10) == -1) {
    perror("listen failed");
    close(ss);
    return EXIT_FAILURE;
  }
  printf("listening on port %d\n", PORT);

  while (1) {
    // Accept
    socklen_t len = sizeof(addr);
    int cs = accept(ss, (struct sockaddr *)&addr, &len);
    if (cs == -1) {
      perror("accept failed");
      close(ss);
      return EXIT_FAILURE;
    }
    printf("accept success\n");

    // Receive
    char buf[1024];
    int received = receive_all(cs, buf, sizeof(buf) - 1);
    if (received == -1) {
      perror("receive faild");
      close(cs);
      close(ss);
      return EXIT_FAILURE;
    }
    buf[received] = '\0';
    printf("\nreceived:\n%s\n", buf);

    // Create HTTP Response
    char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Hello from server</h1></body></html>"
        "\r\n\r\n";

    // Send HTTP Response
    if (send_all(cs, response, strlen(response)) == -1) {
      perror("send failed");
      close(cs);
      close(ss);
      return EXIT_FAILURE;
    }
    printf("sent http reponse\n");

    // Close client socket
    close(cs);
    printf("client connection closed\n");
  }
  // Close server socket
  close(ss);
  printf("server socket closed\n");
  return 0;
}
