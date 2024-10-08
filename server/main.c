#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "send_and_receive.h"
#include "server.h"

int main() {
  printf("server starting...\n");
  const char *cause = NULL;

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
  int sockets[MAXSOCK];
  int nsock = 0;
  int ss;
  for (struct addrinfo *info = infos; info && nsock < MAXSOCK;
       info = info->ai_next) {
    // Create socket
    sockets[nsock] =
        socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    ss = sockets[nsock];
    if (ss == -1) {
      cause = "failed to create socket";
      break;
    }

    // Socket Option
    int opt = 1;
    if (setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      cause = "setsockopt SO_REUSEADDR failed";
      goto cleanup;
    }

    // Bind
    if (bind(ss, info->ai_addr, info->ai_addrlen) == -1) {
      cause = "bind failed";
      goto cleanup;
    }

    // Listen
    if (listen(ss, BACKLOG) == -1) {
      cause = "listen failed";
      goto cleanup;
    }

    nsock++;
  }
  if (nsock == 0) {
    err(EXIT_FAILURE, "%s", cause);
  }
  freeaddrinfo(infos);
  printf("listening on port 80\n");

  while (1) {
    // Accept
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    int *cs = malloc(sizeof(int));
    if (!cs) {
      perror("malloc failed");
      continue;
    }
    *cs = accept(ss, (struct sockaddr *)&addr, &len);
    if (*cs == -1) {
      perror("accept failed");
      free(cs);
      continue;
    }

    // Create thread to handle request
    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_request, cs) != 0) {
      perror("pthread_create failed");
      close(*cs);
      free(cs);
      continue;
    }
    pthread_detach(thread);
  }

// この部分が実行されるのはエラーが起きた場合のみ
// （シグナルを使っていないので、Ctrl+Cでループの実行を中断したらこの部分は実行されないため）
cleanup:
  for (int i = 0; i < nsock; i++) {
    if (close(i) == -1) {
      err(EXIT_FAILURE, "failed to close");
    }
  }
  freeaddrinfo(infos);
  err(EXIT_FAILURE, "server socket closed: cause=%s", cause);
}

void *handle_request(void *arg) {
  int cs = *(int *)arg;
  free(arg);

  // Receive
  char *buf = NULL;
  int received = receive_all(cs, &buf);
  if (received == -1) {
    perror("recv failed");
    goto cleanup;
  }
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
  if (send_all(cs, response) == -1) {
    perror("send failed");
    goto cleanup;
  }
  printf("sent http reponse\n");

cleanup:
  close(cs);
  printf("client connection closed\n");
  pthread_exit(NULL);
}
