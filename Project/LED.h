#define PERIPHERAL_BASE 0x3F000000UL
#define GPIO_BASE (PERIPHERAL_BASE + 0x200000)

#define RED 26
#define GREEN 19
#define BLUE 13

extern void set_gpio_output(void *, int);
extern void set_led_red(void *);
extern void set_led_blue(void *);
extern void set_led_green(void *);

void set_gpio_output_value(void *, int, int);
void set_gpio_input(void *, int);
void set_gpio_pullup(void *, int);
void get_gpio_input_value(void *, int, int *);