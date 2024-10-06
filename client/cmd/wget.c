#include "wget.h"

#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "send_and_receive.h"

int main(int argc, char *argv[]) {
  int error;
  const char *cause = NULL;

  // Get hostname & port
  if (argc != 2) {
    error = 1;
  }
  char *delimiter = strchr(argv[1], ':');
  if (delimiter == NULL) {
    error = 1;
  }
  if (error) {
    errx(EXIT_FAILURE, "Usage: %s <hostname>:<port>", argv[0]);
  }
  *delimiter = '\0';
  const char *hostname = argv[1];
  const char *port = delimiter + 1;

  // See `man getaddrinfo` Examples
  struct addrinfo hints, *infos;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo(hostname, port, &hints, &infos);
  if (error) {
    errx(EXIT_FAILURE, "%s", gai_strerror(error));
  }
  int s = -1;
  for (struct addrinfo *info = infos; info; info = info->ai_next) {
    // Create socket
    s = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (s == -1) {
      cause = "failed to create socket";
      continue;
    }

    // Connect
    if (connect(s, info->ai_addr, info->ai_addrlen) == -1) {
      cause = "failed to connect";
      close(s);
      s = -1;
      continue;
    }

    break;
  }
  if (s == -1) {
    err(EXIT_FAILURE, "%s", cause);
  }
  freeaddrinfo(infos);
  cause = NULL;

  // Send HTTP GET Request
  char request[1024] = {0};
  snprintf(request, sizeof(request),
           "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);
  if (send_all(s, request, strlen(request)) == -1) {
    error = 1;
    cause = "failed to send";
    goto cleanup;
  }

  // Receive HTTP Response
  char buf[1024];
  int received = receive_all(s, buf, sizeof(buf) - 1);
  if (received == -1) {
    error = 1;
    cause = "failed to receive";
    goto cleanup;
  }
  buf[received] = '\0';
  printf("\n%s\n", buf);

cleanup:
  if (close(s) == -1) {
    err(EXIT_FAILURE, "failed to close, cause=%s", cause);
  }
  if (error) {
    err(EXIT_FAILURE, "%s", cause);
  }
  return 0;
}
