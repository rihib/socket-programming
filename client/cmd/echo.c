#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  char buf[100] = {0};
  size_t buf_size = sizeof(buf);
  for (int i = 1; i < argc; i++) {
    int space = buf_size - strlen(buf) - 1;
    strncat(buf, argv[i], space);

    space = buf_size - strlen(buf) - 1;
    if (i < argc - 1) {
      strncat(buf, " ", space);
    }
  }
  printf("%s\n", buf);
  return 0;
}
