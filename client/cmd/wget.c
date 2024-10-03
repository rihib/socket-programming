#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void show_usage_and_exit(char *program_name);
int send_all(int s, char *buf, int len);
int receive_all(int s, char *buf, int len);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    show_usage_and_exit(argv[0]);
  }
  char *delimiter = strchr(argv[1], ':');
  if (delimiter == NULL) {
    show_usage_and_exit(argv[0]);
  }
  *delimiter = '\0';
  const char *ipaddr = argv[1];
  int port = atoi(delimiter + 1);  // not care if atoi could conver it or not

  // Create HTTP GET Request
  char request[1024] = {0};
  snprintf(request, sizeof(request),
           "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", ipaddr);

  // Create Socket
  int s = socket(PF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("failed to create socket");
    return -1;
  }

  // Configure Server Address & Port
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  if (inet_aton(ipaddr, &addr.sin_addr) == 0) {
    perror("invalid address");
    close(s);
    return -1;
  }
  addr.sin_port = htons(port);

  // Connect to the Server
  if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("failed to connect");
    close(s);
    return -1;
  }

  // Send HTTP GET Request
  if (send_all(s, request, strlen(request)) == -1) {
    perror("failed to send");
    close(s);
    return -1;
  }

  // Receive HTTP Response
  char buf[1024];
  int received = receive_all(s, buf, sizeof(buf));
  if (received == -1) {
    perror("failed to receive");
    close(s);
    return -1;
  }
  buf[received] = '\0';
  printf("\n%s\n", buf);

  // Close
  // FIXME: closeが失敗した場合を考慮すべきなのだろうか
  if (close(s) == -1) {
    perror("failed to close");
    return -1;
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

void show_usage_and_exit(char *program_name) {
  fprintf(stderr, "Usage: %s <ipaddress>:<port>\n", program_name);
  exit(1);
}
