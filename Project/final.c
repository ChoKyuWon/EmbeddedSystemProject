#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "LED.h"
#include "LCD.h"
#include "final.h"

int threshold[3];
int fdmem, lcd_fd, sensor_fd, bluetooth_fd;
void* gpio_ctr;

int main() {
  // GPIO for LED INIT
  fdmem = open("/dev/mem", O_RDWR);
  if (fdmem < 0) {
    printf("Error openning /dev/mem\n");
    return NULL;
  }
  void *gpio_ctr =
      mmap(0, 4096, PROT_READ + PROT_WRITE, MAP_SHARED, fdmem, GPIO_BASE);
  if (gpio_ctr == MAP_FAILED) {
    printf("mmap failed\n");
    return NULL;
  }

  set_gpio_output(gpio_ctr, RED);
  set_gpio_output(gpio_ctr, GREEN);
  set_gpio_output(gpio_ctr, BLUE);

  // First I2C(LCD Display) INIT
  lcd_fd = open("/dev/i2c-1", O_RDWR);
  if (lcd_fd < 0) {
    printf("err opening device.\n");
    return -1;
  }
  ssd1306_Init(lcd_fd);

  // Second I2C(sensor) INIT
  sensor_fd = open("/dev/i2c-2", O_RDWR);
  if (sensor_fd < 0) {
    printf("err opening device.\n");
    return -1;
  }

  // Bluetooth INIT
  int ret;
  bluetooth_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
  if (bluetooth_fd == -1) {
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

  tcflush(bluetooth_fd, TCIFLUSH);
  ret = tcsetattr(bluetooth_fd, TCSANOW, &config);
  if (ret < 0) {
    printf("Configuration error.\n");
    return -1;
  }
  // Bluetooth INIT finish

  char r_buf[256] = {0};
  char cmd[256] = {0};

  while(1){
    int index = 0;
    while(ret = read(bluetooth_fd, r_buf, 255) >= 0){
      if(strstr("\xde\xad", r_buf) != NULL){
        // Data on flight. so we put r_buf in cmd and clear the r_buf and wait other data.
        int l = 0;
        while(r_buf[l] != NULL){
          cmd[index] = r_buf[l];
          index += 1;
          l += 1;
        }
        memset(r_buf, 0, sizeof(r_buf));
      }
      else{
        // We got finish data. now process cmd and clear the cmd.
        process(cmd);
        memset(cmd, 0, sizeof(cmd));
        memset(r_buf, 0, sizeof(r_buf));
      }
    }
  }
  close(fdmem);
  close(lcd_fd);
  close(sensor_fd);
  close(bluetooth_fd);
}

int process(char *cmdline){
  struct _protocol *get_header = (struct _protocol*)&cmdline[0];
  union protocol send_header = {0};
  char buffer[256];
  int tmp = 0;

  if(get_header->type == ACK){
    return 1;
  }
  send_header.proto = *get_header;
  send_header.proto.type = ACK;

  switch(get_header->cmd){
    case GETTMP:
    tmp = gettmp();
    snprintf(buffer, sizeof(buffer), "%d", tmp); // itoa(tmp, buffer, 10);
    write(bluetooth_fd, send_header.data, 1);
    write(bluetooth_fd, buffer, strlen(buffer));
    write(bluetooth_fd, "\xde\xad", 2);
    break;

    case GETHUM:
    tmp = gethum();
    snprintf(buffer, sizeof(buffer), "%d", tmp);
    write(bluetooth_fd, send_header.data, 1);
    write(bluetooth_fd, buffer, strlen(buffer));
    write(bluetooth_fd, "\xde\xad", 2);
    break;

    case SETLED:
    write(bluetooth_fd, send_header.data, 1);
    for(int i=0;i<3;i++){
      threshold[i] = cmdline[i+1];
      write(bluetooth_fd, cmdline[i], 1);
    }
    write(bluetooth_fd, "\xde\xad", 2);
    break;

    case SETLCD:
    lcd_write(get_header->lcd, &cmdline[1]);
    send_header.proto = *get_header;
    send_header.proto.type = ACK;
    write(bluetooth_fd, send_header.data, 1);
    write(bluetooth_fd, "\xde\xad", 2);
    break;
  }
}

void lcd_write(int notion, char* data){
  return;
}
int gettmp(){
  return 0;
}

int gethum(){
  return 0;
}