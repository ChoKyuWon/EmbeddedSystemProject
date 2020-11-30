struct _protocol{
    unsigned int type : 1;
    unsigned int cmd : 2;
    unsigned int lcd : 1;
    unsigned int len : 4;
};

union protocol{
    struct _protocol proto;
    char data;
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
void lcd_write(int notion, char* data);