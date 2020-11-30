#include <stdint.h>

#define LOGO_WIDTH 50
#define LOGO_HEIGHT 6
#define LOGO_MOVE 4 // 4 pixels per frame
#define NUM_FRAMES (S_WIDTH / LOGO_MOVE)
#define LOGO_Y_LOC 1
#define S_WIDTH 128
#define S_HEIGHT 64
#define S_PAGES (S_HEIGHT / 8)
#define FONT_WIDTH 6
#define FONT_HEIGHT 1 // 1 page

extern void ssd1306_Init(int i2c_fd);
extern void write_str(int i2c_fd, char *str, int x, int y);

void ssd1306_command(int i2c_fd, uint8_t cmd);
void ssd1306_data(int i2c_fd, const uint8_t *data, size_t size);
void update_full(int i2c_fd, uint8_t *data);
void update_area(int i2c_fd, const uint8_t *data, int x, int y, int x_len, int y_len);
void write_char(int i2c_fd, char c, int x, int y);