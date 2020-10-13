#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>

#define PERIPHERAL_BASE 0x3F000000UL
#define GPIO_BASE (PERIPHERAL_BASE + 0x200000)

#define RED 26
#define GREEN 19
#define BLUE 13

void set_gpio_output_value(void *, int, int);
void set_gpio_output(void *, int);
void set_gpio_input(void *, int);
void set_gpio_pullup(void *, int);
void set_led_red(void *);
void set_led_blue(void *);
void set_led_green(void *);

void get_gpio_input_value(void *, int, int *);

int main()
{
    int fdmem = open("/dev/mem", O_RDWR);
    if (fdmem < 0)
    {
        printf("Error openning /dev/mem\n");
        return -1;
    }
    void *gpio_ctr = mmap(0, 4096, PROT_READ + PROT_WRITE, MAP_SHARED, fdmem, GPIO_BASE);
    if (gpio_ctr == MAP_FAILED)
    {
        printf("mmap failed\n");
        return -2;
    }

    set_gpio_output(gpio_ctr, RED);
    set_gpio_output(gpio_ctr, GREEN);
    set_gpio_output(gpio_ctr, BLUE);

    set_gpio_input(gpio_ctr, 4);
    set_gpio_pullup(gpio_ctr, 4);

    int gpio_4_value;
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    if (gpio_4_value)
    {
        set_led_blue(gpio_ctr);
    }
    else
    {
        set_led_green(gpio_ctr);
    }
    munmap(gpio_ctr, 4096);
    close(fdmem);
}

void set_gpio_output_value(void *gpio_ctr, int gpio_nr, int value)
{
    int reg_id = gpio_nr / 32;
    int pos = gpio_nr % 32;
    if (value)
    {
#define GPIO_SET_OFFSET 0x1c
        uint32_t *output_set = (u_int32_t *)(gpio_ctr + GPIO_SET_OFFSET + 0x4 * reg_id);
        *output_set = 0x1 << pos;
    }
    else
    {
#define GPIO_CLR_OFFSET 0x28
        uint32_t *output_clr = (uint32_t *)(gpio_ctr + GPIO_CLR_OFFSET + 0x4 * reg_id);
        *output_clr = 0x1 << pos;
    }
}

void set_gpio_output(void *gpio_ctr, int gpio_nr)
{
    int reg_id = gpio_nr / 10;
    int pos = gpio_nr % 10;

    uint32_t *fsel_reg = (uint32_t *)(gpio_ctr + 0x4 * reg_id);

    uint32_t fsel_val = *fsel_reg;

    uint32_t mask = 0x7 << (pos * 3);
    fsel_val &= ~mask;

    uint32_t gpio_output_select = 0x1 << (pos * 3);
    fsel_val |= gpio_output_select;
    *fsel_reg = fsel_val;
}

void set_gpio_input(void *gpio_ctr, int gpio_nr)
{
    int reg_id = gpio_nr / 10;
    int pos = gpio_nr & 10;

    uint32_t *fsel_reg = (uint32_t *)(gpio_ctr + 0x4 * reg_id);
    uint32_t fsel_val = *fsel_reg;
    uint32_t mask = 0x7 << (pos * 3);
    fsel_val &= ~mask;

    *fsel_reg = fsel_val;
}

void set_gpio_pullup(void *gpio_ctr, int gpio_nr)
{
    int reg_id = gpio_nr / 32;
    int pos = gpio_nr % 32;

#define GPIO_PUD_OFFSET 0x94
#define GPIO_PUDCLK_OFFSET 0x98
    uint32_t *pud_reg = (uint32_t *)(gpio_ctr + GPIO_PUD_OFFSET);
    uint32_t *pudclk_reg = (uint32_t *)(gpio_ctr + GPIO_PUDCLK_OFFSET + 0x4 * reg_id);

#define GPIO_PUD_PULLUP 0x2
    *pud_reg = GPIO_PUD_PULLUP;
    usleep(1);
    *pudclk_reg = (0x1 << pos);
    usleep(1);
    *pud_reg = 0;
    *pudclk_reg = 0;
}

void get_gpio_input_value(void *gpio_ctr, int gpio_nr, int *value)
{
    int reg_id = gpio_nr / 32;
    int pos = gpio_nr % 32;
#define GPIO_LEV_OFFSET 0x34
    uint32_t *level_reg = (uint32_t *)(gpio_ctr + GPIO_LEV_OFFSET + 0x4 * reg_id);
    uint32_t level = *level_reg & (0x1 << pos);

    *value = level ? 1 : 0;
}

void set_led_red(void *gpio_ctr)
{
    set_gpio_output_value(gpio_ctr, RED, 1);
    set_gpio_output_value(gpio_ctr, GREEN, 0);
    set_gpio_output_value(gpio_ctr, BLUE, 0);
}

void set_led_green(void *gpio_ctr)
{
    set_gpio_output_value(gpio_ctr, RED, 0);
    set_gpio_output_value(gpio_ctr, GREEN, 1);
    set_gpio_output_value(gpio_ctr, BLUE, 0);
}

void set_led_blue(void *gpio_ctr)
{
    set_gpio_output_value(gpio_ctr, RED, 0);
    set_gpio_output_value(gpio_ctr, GREEN, 0);
    set_gpio_output_value(gpio_ctr, BLUE, 1);
}