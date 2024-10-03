#include "wget.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "send_and_receive.h"

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
    return EXIT_FAILURE;
  }

  // Configure Server Address & Port
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  if (inet_aton(ipaddr, &addr.sin_addr) == 0) {
    perror("invalid address");
    close(s);
    return EXIT_FAILURE;
  }
  addr.sin_port = htons(port);

  // Connect to the Server
  if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("failed to connect");
    close(s);
    return EXIT_FAILURE;
  }

  // Send HTTP GET Request
  if (send_all(s, request, strlen(request)) == -1) {
    perror("failed to send");
    close(s);
    return EXIT_FAILURE;
  }

  // Receive HTTP Response
  char buf[1024];
  int received = receive_all(s, buf, sizeof(buf) - 1);
  if (received == -1) {
    perror("failed to receive");
    close(s);
    return EXIT_FAILURE;
  }
  buf[received] = '\0';
  printf("\n%s\n", buf);

  // Close
  // FIXME: closeが失敗した場合を考慮すべきなのだろうか
  if (close(s) == -1) {
    perror("failed to close");
    return EXIT_FAILURE;
  }
  return 0;
}

void show_usage_and_exit(char *program_name) {
  fprintf(stderr, "Usage: %s <ipaddress>:<port>\n", program_name);
  exit(EXIT_FAILURE);
}
