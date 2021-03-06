#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <time.h>
#define AM2320_I2C_DEV 0x5C

#include "LCD.h"
#include "LED.h"
#include "final.h"

int threshold[3];
int notation = 0;
int fdmem, lcd_fd, sensor_fd, bluetooth_fd;
void *gpio_ctr;

int main() {
  // GPIO for LED INIT
  fdmem = open("/dev/mem", O_RDWR);
  if (fdmem < 0) {
    printf("Error openning /dev/mem\n");
    return -1;
  }
  void *gpio_ctr =
      mmap(0, 4096, PROT_READ + PROT_WRITE, MAP_SHARED, fdmem, GPIO_BASE);
  if (gpio_ctr == MAP_FAILED) {
    printf("mmap failed\n");
    return -1;
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
  if (ioctl(sensor_fd, I2C_SLAVE, AM2320_I2C_DEV) < 0) {
    printf("err setting i2c slave address\n");
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

  pthread_t thread_pool[2];
  int thr_id;
  int status;
  thr_id = pthread_create(&thread_pool[0], NULL, watchdog, NULL);
  char r_buf[256] = {0};
  char cmd[256] = {0};

  while (1) {
    int index = 0;
    while (ret = read(bluetooth_fd, r_buf, 255) >= 0) {
      if (strstr("\xde\xad", r_buf) != NULL) {
        // Data on flight. so we put r_buf in cmd and clear the r_buf and wait
        // other data.
        int l = 0;
        while (r_buf[l] != 0) {
          cmd[index] = r_buf[l];
          index += 1;
          l += 1;
        }
        memset(r_buf, 0, sizeof(r_buf));
      } else {
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

int process(char *cmdline) {
  struct _protocol *get_header = (struct _protocol *)&cmdline[0];
  union protocol send_header = {0};
  char buffer[256];
  int tmp = 0;

  if (get_header->type == ACK) {
    return 1;
  }
  send_header.proto = *get_header;
  send_header.proto.type = ACK;

  switch (get_header->cmd) {
  case GETTMP:
    tmp = gettmp();
    snprintf(buffer, sizeof(buffer), "%d", tmp); // itoa(tmp, buffer, 10);
    write(bluetooth_fd, &send_header.data, 1);
    write(bluetooth_fd, buffer, strlen(buffer));
    write(bluetooth_fd, "\xde\xad", 2);
    break;

  case GETHUM:
    tmp = gethum();
    snprintf(buffer, sizeof(buffer), "%d", tmp);
    write(bluetooth_fd, &send_header.data, 1);
    write(bluetooth_fd, buffer, strlen(buffer));
    write(bluetooth_fd, "\xde\xad", 2);
    break;

  case SETLED:
    write(bluetooth_fd, &send_header.data, 1);
    for (int i = 0; i < 3; i++) {
      threshold[i] = cmdline[i + 1];
      write(bluetooth_fd, &cmdline[i], 1);
    }
    write(bluetooth_fd, "\xde\xad", 2);
    break;

  case SETLCD:
    notation = get_header->lcd;
    print_lcd_string(&cmdline[1]);
    send_header.proto = *get_header;
    send_header.proto.type = ACK;
    write(bluetooth_fd, &send_header.data, 1);
    write(bluetooth_fd, "\xde\xad", 2);
    break;
  }
  return 0;
}

void watchdog() {
  int tmp = 0, hum = 0;
  while (1) {
    // LED
    tmp = gettmp();
    hum = gethum();
    if (tmp > threshold[0])
      set_led_red(gpio_ctr);
    else if (tmp > threshold[1])
      set_led_green(gpio_ctr);
    else
      set_led_blue(gpio_ctr);
    
    // LCD
    print_lcd_status(tmp, hum);

    // sleep
    sleep(1);
  }
}

int getdata(uint8_t addr, struct sensor_data* output) {
  int res;
  uint8_t buffer[10] = {0};
  struct timespec time_before, time_after, time_diff; // step 1: Wakeup
  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &time_before);
    write(sensor_fd, buffer, 1);
    usleep(800);
    clock_gettime(CLOCK_MONOTONIC, &time_after);
    time_diff.tv_sec = time_after.tv_sec - time_before.tv_sec;
    time_diff.tv_nsec = time_after.tv_nsec - time_before.tv_nsec;
    long long diff_nsec = time_diff.tv_sec * 1e9 + time_diff.tv_nsec;
    if (diff_nsec < 3000000)
      break;
  }
  buffer[0] = 0x3;
  buffer[1] = addr;
  buffer[2] = 0x4;
  if ((res = write(sensor_fd, buffer, 3)) != 3) {
    printf("i2c write failed! %d\n", res);
  }
  usleep(1500);
  if (read(sensor_fd, buffer, 8) != 8) {
    printf("i2c read failed!\n");
  }
  uint16_t hum = buffer[2] << 8 | buffer[3];
  uint16_t temp = buffer[4] << 8 | buffer[5];
  output->hum = hum;
  output->temp = temp;
}
void print_lcd_string(char *data) {
  write_str(lcd_fd, data, 10, S_PAGES -1);
}
void print_lcd_status(uint16_t temp, uint16_t hum){
  char buffer[256] = {0};
  char not = 0;
  if(notation == 0){
    not = 'C';
  }
  else{
    not ='F';
  }
  snprintf(buffer, sizeof(buffer), "%d%c/%d%%", temp, not, hum);
  write_str(lcd_fd, buffer, 10, S_PAGES + 10);
}

int gettmp()
{
  struct sensor_data p;
  getdata(0x0, &p);
  return p.temp;
}
int gethum()
{
  struct sensor_data p;
  getdata(0x0, &p);
  return p.hum;
}