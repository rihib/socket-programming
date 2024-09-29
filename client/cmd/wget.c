#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9000

int send_reliably(int s, char *buf, int len);
int receive_reliably(int s, char *buf, int len);

int main(int argc, char *argv[]) {
  char data[100] = {0};
  size_t data_size = sizeof(data);
  for (int i = 1; i < argc; i++) {
    int space = data_size - strlen(data) - 1;
    strncat(data, argv[i], space);

    space = data_size - strlen(data) - 1;
    if (i < argc - 1) {
      strncat(data, " ", space);
    }
  }

  // Create Socket
  int s = socket(PF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("failed to create socket");
    return -1;
  }

  // Configure Server Address & Port
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  if (inet_aton("127.0.0.1", &addr.sin_addr) == 0) {
    perror("invalid address");
    close(s);
    return -1;
  }
  addr.sin_port = htons(PORT);

  // Connect to the Server
  if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("failed to connect");
    close(s);
    return -1;
  }

  // Send
  if (send_reliably(s, data, 100) == -1) {
    perror("failed to send");
    close(s);
    return -1;
  }

  // Receive
  char buf[1024];
  if (receive_reliably(s, buf, 100) == -1) {
    perror("failed to receive");
    close(s);
    return -1;
  }
  printf("%s\n", buf);

  // Close
  if (close(s) == -1) {
    perror("failed to close");
    return -1;
  }
  return 0;
}

int send_reliably(int s, char *buf, int len) {
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

int receive_reliably(int s, char *buf, int len) {
  int received = 0;
  while (received < len) {
    int n = recv(s, buf + received, len - received, 0);
    if (n == -1) {
      perror("failed to receive");
      return -1;
    }
    if (n == 0) {
      fprintf(stderr, "EOF\n");
      return received;
    }
    received += n;
  }
  return received;
}
