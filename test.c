#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int main() {
  int fd = 0;
  char buf[255] = {0};
  int status = read(fd, buf, 2);
  int status2 = read(fd, buf + status, 25400 - status);

  printf("Status: %d, status2: %d, content: %s", status, status2, buf);
}