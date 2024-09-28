#include <stdio.h>

void shell() {
  char buf[100];
  while (1) {
    printf("echo> ");
    scanf("%99[^\n]", buf);
    getchar();
    printf("%s\n", buf);
  }
}
