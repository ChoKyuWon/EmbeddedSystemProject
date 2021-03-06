#include "Minimum+1_font.h"
#include "image.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "LCD.h"

void ssd1306_command(int i2c_fd, uint8_t cmd) {
  uint8_t buffer[2];
  buffer[0] = (0 << 7) | (0 << 6);
  buffer[1] = cmd;

  if (write(i2c_fd, buffer, 2) != 2) {
    printf("i2c write failed at cmd transmission.\n");
  }
}

void ssd1306_data(int i2c_fd, const uint8_t *data, size_t size) {
  uint8_t *buffer = (uint8_t *)malloc(size + 1);

  buffer[0] = (0 << 7) | (1 << 6);
  memcpy(buffer + 1, data, size);
  if (write(i2c_fd, buffer, size + 1) != size + 1) {
    printf("i2c write failed at data transmission.\n");
  }
}

void ssd1306_Init(int i2c_fd) {
  ssd1306_command(i2c_fd, 0xA8);
  ssd1306_command(i2c_fd, 0x3f);
  ssd1306_command(i2c_fd, 0xD3); // Set Display Offset
  ssd1306_command(i2c_fd, 0x00);
  ssd1306_command(i2c_fd, 0x40); // Set Display Start Line
  ssd1306_command(i2c_fd, 0xA0); // Set Segment re-map
  // 0xA1 for vertical inversion
  ssd1306_command(i2c_fd, 0xC0); // Set COM Output Scan Direction
  // 0xC8 for horizontal inversion
  ssd1306_command(i2c_fd, 0xDA); // Set COM Pins hardware configuration
  ssd1306_command(i2c_fd, 0x12); // Manual says 0x2, but 0x12 is required
  ssd1306_command(i2c_fd, 0x81); // Set Contrast Control
  ssd1306_command(i2c_fd, 0x7F); // 0:min, 0xFF:max
  ssd1306_command(i2c_fd, 0xA4); // Disable Entire Display On
  ssd1306_command(i2c_fd, 0xA6); // Set Normal Display
  ssd1306_command(i2c_fd, 0xD5); // Set Osc Frequency
  ssd1306_command(i2c_fd, 0x80);
  ssd1306_command(i2c_fd, 0x8D); // Enable charge pump regulator
  ssd1306_command(i2c_fd, 0x14);
  ssd1306_command(i2c_fd, 0xAF); // Display ON
}
void update_full(int i2c_fd, uint8_t *data) {
  ssd1306_command(i2c_fd, 0x20); // addressing mode
  ssd1306_command(i2c_fd, 0x0);  // horizontal addressing mode
  ssd1306_command(i2c_fd, 0x21); // set column start/end address
  ssd1306_command(i2c_fd, 0);
  ssd1306_command(i2c_fd, S_WIDTH - 1);
  ssd1306_command(i2c_fd, 0x22); // set page start/end address
  ssd1306_command(i2c_fd, 0);
  ssd1306_command(i2c_fd, S_PAGES - 1);
  ssd1306_data(i2c_fd, data, S_WIDTH * S_PAGES);
}

void update_area(int i2c_fd, const uint8_t *data, int x, int y, int x_len,
                 int y_len) {
  ssd1306_command(i2c_fd, 0x20); // addressing mode
  ssd1306_command(i2c_fd, 0x0);  // horizontal addressing mode
  ssd1306_command(i2c_fd, 0x21); // set column start/end address
  ssd1306_command(i2c_fd, x);
  ssd1306_command(i2c_fd, x + x_len - 1);
  ssd1306_command(i2c_fd, 0x22); // set page start/end address
  ssd1306_command(i2c_fd, y);
  ssd1306_command(i2c_fd, y + y_len - 1);
  ssd1306_data(i2c_fd, data, x_len * y_len);
}

void write_char(int i2c_fd, char c, int x, int y) {
  if (c < ' ')
    c = ' ';
  update_area(i2c_fd, font[c - ' '], x, y, FONT_WIDTH, FONT_HEIGHT);
}
void write_str(int i2c_fd, char *str, int x, int y) {
  char c;
  while (c = *str++) {
    write_char(i2c_fd, c, x, y);
    x += FONT_WIDTH;
  }
}

int LCD_main() {
  int i2c_fd = open("/dev/i2c-1", O_RDWR);
  if (i2c_fd < 0) {
    printf("err opening device.\n");
    return -1;
  }
  ssd1306_Init(i2c_fd);
  uint8_t *data = (uint8_t *)malloc(S_WIDTH * S_PAGES * NUM_FRAMES);
  for (int i = 0; i < NUM_FRAMES; i++) { // i=frame index
    for (int x = 0; x < LOGO_WIDTH; x++) {
      for (int y = 0; y < LOGO_HEIGHT; y++) {
        int target_x = (i * LOGO_MOVE + x) % S_WIDTH;
        data[S_WIDTH * S_PAGES * i + S_WIDTH * (y + LOGO_Y_LOC) + target_x] =
            skku[LOGO_WIDTH * y + x];
      }
    }
  };
  while (1) {
    for (int i = 0; i < NUM_FRAMES; i++) {
      update_full(i2c_fd, &data[S_WIDTH * S_PAGES * i]);
    }
  }

  update_full(i2c_fd, data);
  free(data);
  close(i2c_fd);
  return 0;
}
