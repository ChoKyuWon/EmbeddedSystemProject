#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int main() {
  int fd, ret;
  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
  if (fd == -1) {
    printf("Port open error\n");
    return -1;
  }
  struct termios config;
  memset(&config, 0, sizeof(config));

  config.c_iflag = IGNPAR;
  config.c_oflag = 0;
  config.c_cflag = CS8 | CLOCAL;
  config.c_lflag = 0;
  config.c_cc[VMIN] = 1;

  cfsetispeed(&config, B9600);
  cfsetospeed(&config, B9600);

  tcflush(fd, TCIFLUSH);
  ret = tcsetattr(fd, TCSANOW, &config);
  if(ret < 0){
    printf("Configuration error.\n");
    return -1;
  }

  const char w_buf[] = "Hello! Can you see this?\n";
  write(fd, w_buf, strlen(w_buf));

  char r_buf[256] = {0};
  while(ret = read(fd, r_buf, 255) >= 0){
    if(ret){
      printf("Recived %d bytes: %s\n", ret, r_buf);
    }
  }

  close(fd);
}