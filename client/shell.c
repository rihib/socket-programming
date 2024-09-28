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
    char arg_string[100] = {0};
    unsigned long i = 0;
    while (buf[i] != ' ' && buf[i] != '\0' && i < (sizeof(cmd) - 1)) {
      cmd[i] = buf[i];
      i++;
    }
    cmd[i] = '\0';
    if (buf[i] == ' ') {
      strncpy(arg_string, buf + i + 1, sizeof(arg_string) - 1);
    }

    __darwin_pid_t pid = fork();
    if (pid == -1) {
      perror("fork failed");
      exit(1);
    }
    if (pid != 0) {
      int status;
      waitpid(pid, &status, 0);
    }
    if (pid == 0) {
      if (strcmp(cmd, "echo") == 0) {
        execl("client/cmd/bin/echo", "echo", arg_string, (char *)NULL);
        perror("command execution failed");
      } else {
        fprintf(stderr, "command not found: %s\n", cmd);
      }
      exit(1);
    }
  }
}
