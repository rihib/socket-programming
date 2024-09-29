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
  while (1) {
    // Create Socket
    int ss = socket(PF_INET, SOCK_STREAM, 0);
    if (ss == -1) {
      perror("failed to create socket");
      return -1;
    }
    printf("create server socket\n");

    // Socket Option
    int opt = 1;
    if (setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      perror("setsockopt SO_REUSEADDR failed");
      close(ss);
      return 1;
    }

    // Configure Address & Port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    if (inet_aton("127.0.0.1", &addr.sin_addr) == 0) {
      perror("invalid address");
      close(ss);
      return -1;
    }
    addr.sin_port = htons(PORT);

    // Bind
    if (bind(ss, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
      perror("bind failed");
      close(ss);
      return 1;
    }
    printf("bind success\n");

    // Listen
    if (listen(ss, 10) == -1) {
      perror("listen failed");
      close(ss);
      return 1;
    }
    printf("listen success\n");

    // Accept
    socklen_t len = sizeof(addr);
    int cs = accept(ss, (struct sockaddr *)&addr, &len);
    if (cs == -1) {
      perror("accept failed");
      close(ss);
      return -1;
    }
    printf("accept success\n");

    // Receive
    char buf[1024];
    int received = receive_all(cs, buf, sizeof(buf));
    if (received == -1) {
      perror("receive faild");
      close(cs);
      close(ss);
      return -1;
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
      return -1;
    }
    printf("sent http reponse\n");

    // Close
    close(cs);
    close(ss);
    printf("close success\n");
  }
  return 0;
}

int send_all(int s, char *buf, int len) {
  int sent = 0;
  while (sent < len) {
    int n = send(s, buf + sent, len - sent, 0);
    if (n == -1) {
      perror("failed to send");
      return -1;
    }
    if (n == 0) {
      fprintf(stderr, "EOF\n");
      return sent;
    }
    sent += n;
  }
  return sent;
}

int receive_all(int s, char *buf, int len) {
  int received = 0;
  while (received < len) {
    int n = recv(s, buf + received, len - received, 0);
    if (n == -1) {
      perror("failed to receive");
      return -1;
    }
    if (n == 0) {
      fprintf(stderr, "EOF\n");
      break;
    }
    received += n;
    if (strstr(buf, "\r\n\r\n") != NULL) {
      break;
    }
  }
  return received;
}
