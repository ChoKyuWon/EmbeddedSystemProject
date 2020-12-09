#include <stdint.h>

struct _protocol
{
    unsigned int lcd : 1;
    unsigned int cmd : 2;
    unsigned int type : 1;
    unsigned int len : 4;
};

union protocol
{
    struct _protocol proto;
    char data;
};

struct sensor_data
{
    uint16_t temp;
    uint16_t hum;
};

#define CMD 0
#define ACK 1

#define GETTMP 0
#define GETHUM 1
#define SETLED 2
#define SETLCD 3

int process(char *cmd);
int gettmp();
int gethum();
void watchdog();
void print_lcd_status(uint16_t temp, uint16_t hum);
void print_lcd_string(char *data);