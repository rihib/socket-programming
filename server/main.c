#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "send_and_receive.h"

int main() {
  printf("server starting...\n");

  // See `man getaddrinfo` Examples
  struct addrinfo hints, *infos;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  int error = getaddrinfo(NULL, "http", &hints, &infos);
  if (error) {
    errx(EXIT_FAILURE, "%s", gai_strerror(error));
  }
  const int MAXSOCK = 1;
  int sockets[MAXSOCK];
  int nsock = 0;
  const char *cause = NULL;
  for (struct addrinfo *info = infos; info && nsock < MAXSOCK;
       info = info->ai_next) {
    // Create socket
    sockets[nsock] =
        socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sockets[nsock] == -1) {
      error = 1;
      cause = "failed to create socket";
      continue;
    }

    // Socket Option
    int opt = 1;
    if (setsockopt(sockets[nsock], SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) == -1) {
      cause = "setsockopt SO_REUSEADDR failed";
      close(sockets[nsock]);
      continue;
    }

    // Bind
    if (bind(sockets[nsock], info->ai_addr, info->ai_addrlen) == -1) {
      cause = "bind failed";
      close(sockets[nsock]);
      continue;
    }

    // Listen
    if (listen(sockets[nsock], 5) == -1) {
      cause = "listen failed";
      close(sockets[nsock]);
      continue;
    }

    nsock++;
  }
  if (nsock == 0) {
    err(EXIT_FAILURE, "%s", cause);
  }
  freeaddrinfo(infos);
  int ss = sockets[0];
  printf("listening on port 80\n");

  while (1) {
    // Accept
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    int cs = accept(ss, (struct sockaddr *)&addr, &len);
    if (cs == -1) {
      close(ss);
      err(EXIT_FAILURE, "accept failed");
    }

    // Receive
    char buf[1024];
    int received = receive_all(cs, buf, sizeof(buf) - 1);
    if (received == -1) {
      close(cs);
      close(ss);
      err(EXIT_FAILURE, "receive faild");
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
      close(cs);
      close(ss);
      err(EXIT_FAILURE, "send failed");
    }
    printf("sent http reponse\n");

    // Close client socket
    close(cs);
    printf("client connection closed\n");
    if (error == 1) {
      break;
    }
  }
  // Close server socket
  close(ss);
  printf("server socket closed\n");
  return 0;
}
