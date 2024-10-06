#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int send_all(int s, char *buf, int len) {
  int sent = 0;
  while (sent < len) {
    int n = send(s, buf + sent, len - sent, 0);
    if (n == -1) {
      perror("failed to send");
      return EXIT_FAILURE;
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
      return EXIT_FAILURE;
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
