#include <stdint.h>

void ssd1306_Init(int i2c_fd);
void ssd1306_command(int i2c_fd, uint8_t cmd);
void ssd1306_data(int i2c_fd, const uint8_t *data, size_t size);
void update_full(int i2c_fd, uint8_t *data);
void update_area(int i2c_fd, const uint8_t *data, int x, int y, int x_len, int y_len);
void write_char(int i2c_fd, char c, int x, int y);
void write_str(int i2c_fd, char *str, int x, int y);