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

int bluetooth_fd;
int threshold[3];
int notation;

int process(char *cmdline)
{
    printf("%s\n", cmdline);
    struct _protocol get_header = ((union protocol)cmdline[0]).proto;
    union protocol send_header = {0};
    char buffer[256];
    int tmp = 0;

    if (get_header.type == ACK)
    {
        return 1;
    }
    send_header.proto = get_header;
    send_header.proto.type = ACK;
    printf("get_header: %d\n", get_header);
    printf("get_header.type: %d ", get_header.type);
    printf("get_header.cmd: %d ", get_header.cmd);
    printf("get_header.len: %d ", get_header.len);
    printf("get_header.lcd: %d\n", get_header.lcd);
    switch (get_header.cmd)
    {
    case GETTMP:
        printf("GET TMP\n");
        tmp = gettmp();
        snprintf(buffer, sizeof(buffer), "%d", tmp); // itoa(tmp, buffer, 10);
        printf("GETTMP buffer: %s\n", buffer);
        printf("send_header.data: %c\n", send_header.data);
        write(bluetooth_fd, &send_header.data, 1);
        write(bluetooth_fd, buffer, strlen(buffer));
        write(bluetooth_fd, "\xde\xad", 2);
        break;

    case GETHUM:
        printf("GET HUM\n");
        tmp = gethum();
        snprintf(buffer, sizeof(buffer), "%d", tmp);
        printf("GETHUM buffer: %s\n", buffer);
        printf("send_header.data: %c\n", send_header.data);
        write(bluetooth_fd, &send_header.data, 1);
        write(bluetooth_fd, buffer, strlen(buffer));
        write(bluetooth_fd, "\xde\xad", 2);
        break;

    case SETLED:
        printf("SET LCD\n");
        write(bluetooth_fd, &send_header.data, 1);
        for (int i = 0; i < 3; i++)
        {
            threshold[i] = cmdline[i + 1];
            write(bluetooth_fd, &cmdline[i], 1);
        }
        write(bluetooth_fd, "\xde\xad", 2);
        break;

    case SETLCD:
        printf("SET LCD\n");
        notation = get_header.lcd;
        send_header.proto = get_header;
        send_header.proto.type = ACK;
        write(bluetooth_fd, &send_header.data, 1);
        write(bluetooth_fd, "\xde\xad", 2);
        break;
    }
    return 0;
}

int gettmp()
{
    return 10;
}

int gethum()
{
    return 50;
}

int main(void)
{
    bluetooth_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
    if (bluetooth_fd == -1)
    {
        printf("Bluetooth Port error\n");
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
    int ret = tcsetattr(bluetooth_fd, TCSANOW, &config);
    if (ret < 0)
    {
        printf("Bluetooth configuration error");
        return -1;
    }
    char r_buf[256] = {0};
    char cmd[256] = {0};
    while (1)
    {
        int index = 0;
        while (ret = read(bluetooth_fd, r_buf, 255) >= 0)
        {
            printf("r_buf: %s ", r_buf);
            printf("cmd: %s\n", cmd);
            if (strstr(cmd, "\xde\xad") == NULL || index == 0)
            {
                // Data on flight. so we put r_buf in cmd and clear the r_buf and wait
                // other data.
                int l = 0;
                while (r_buf[l] != 0)
                {
                    cmd[index] = r_buf[l];
                    index += 1;
                    l += 1;
                }
                memset(r_buf, 0, sizeof(r_buf));
                if (strstr(cmd, "\xde\xad") == NULL)
                {
                    printf("cmd: %s\n", cmd);
                    process(cmd);
                    memset(cmd, 0, sizeof(cmd));
                    memset(r_buf, 0, sizeof(r_buf));
                    index = 0;
                }
            }
            else
            {
                // We got finish data. now process cmd and clear the cmd.
                printf("cmd: %s\n", cmd);
                process(cmd);
                memset(cmd, 0, sizeof(cmd));
                memset(r_buf, 0, sizeof(r_buf));
                index = 0;
            }
        }
    }
    close(bluetooth_fd);
    return 0;
}