#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void shell() {
  while (1) {
    char buf[100] = {0};
    printf("shell> ");
    scanf("%99[^\n]", buf);
    getchar();

    char cmd[100] = {0};
    char arg[100] = {0};
    unsigned long i = 0;
    while (buf[i] != ' ' && buf[i] != '\0' && i < (sizeof(cmd) - 1)) {
      cmd[i] = buf[i];
      i++;
    }
    cmd[i] = '\0';
    if (buf[i] == ' ') {
      strncpy(arg, buf + i + 1, sizeof(arg) - 1);
    }

    pid_t pid = fork();
    if (pid == -1) {
      perror("fork failed");
    }
    if (pid != 0) {
      int status;
      waitpid(pid, &status, 0);
    } else {
      if (strcmp(cmd, "wget") == 0) {
        execl("client/cmd/bin/wget", "wget", arg, (char *)NULL);
        perror("command execution failed");
      } else {
        fprintf(stderr, "command not found: %s\n", cmd);
      }
      exit(EXIT_FAILURE);
    }
  }
}
