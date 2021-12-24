
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "lcd_uart.pio.h"

// #define LCD_PIN 0
// #define LCD_BAUD_RATE 2400

//  
//  See https://mil.ufl.edu/3744/docs/lcdmanual/commands.html for commands
//  There are some extras which may be used, but one can implement them for themselves
// 
#define COMMAND_CLEAR               0x01
#define COMMAND_CURSOR_HOME         0x02
#define COMMAND_ENTRY_MODE_SET      0x04
#define COMMAND_DISPLAY             0x08
#define COMMAND_SET_DDRAM_ADDRESS   0x80

#define SLEEP_LONG sleep_us(1640);
#define SLEEP_SHORT sleep_us(40);


typedef struct {
    PIO pio;
    uint sm;
    int pin;
    float baud;
    bool initialising;
    alarm_id_t alarm_id;
} LCD;


int64_t lcd_init_alarm(alarm_id_t id, void *user_data) {
    ((LCD*) user_data)->initialising=false;
    return 0;
}

LCD lcd_init(PIO pio, uint sm, uint pin, float baud_rate) {
    uint offset = pio_add_program(pio, &N_UART_program);
    N_UART_program_init(pio, sm, offset, pin, baud_rate);
    sleep_ms(850);
    LCD out;
    out.pio = pio;
    out.sm = sm;
    out.pin = pin;
    out.baud = baud_rate;
    out.initialising = true;
    out.alarm_id = add_alarm_in_ms(850, lcd_init_alarm, &out, true);
    return out;
}

void sendCommand(LCD lcd, uint cmd) {
    pio_sm_put_blocking(lcd.pio, lcd.sm, 1U);
    pio_sm_put_blocking(lcd.pio, lcd.sm, ~cmd);
}

void lcd_clear_display_blocking(LCD lcd) {
    if (lcd.initialising) return;
    sendCommand(lcd, COMMAND_CLEAR);
    SLEEP_LONG;
}
void lcd_set_cursor_home_blocking(LCD lcd) {
    if (lcd.initialising) return;
    sendCommand(lcd, COMMAND_CURSOR_HOME);
    SLEEP_LONG;
}
void lcd_set_entry_mode_blocking(LCD lcd, bool input_direction, bool shift) {
    if (lcd.initialising) return;
    sendCommand(lcd, COMMAND_ENTRY_MODE_SET + (input_direction << 1) + shift);
    SLEEP_SHORT;
}
void lcd_set_display_on_off_blocking(LCD lcd, bool power) {
    if (lcd.initialising) return;
    sendCommand(lcd, COMMAND_DISPLAY + (power << 2));
    SLEEP_SHORT;
}
void lcd_set_cursor_style_blocking(LCD lcd, bool enabled, bool blinking) {
    if (lcd.initialising) return;
    sendCommand(lcd, COMMAND_DISPLAY + 0x04 + (enabled << 1) + blinking);
    SLEEP_SHORT;
}
void lcd_set_cursor_pos_blocking(LCD lcd, uint row, uint column) {
    if (lcd.initialising) return;
    sendCommand(lcd, COMMAND_SET_DDRAM_ADDRESS + (row << 4) + column);
    SLEEP_SHORT;
}

void lcd_send_message_blocking(LCD lcd, char *msg, uint len) {
    if (lcd.initialising) return;
    for (int i = 0; i < len; ++i) {
        pio_sm_put_blocking(lcd.pio, lcd.sm, ~msg[i]);
        SLEEP_SHORT;
    }
    SLEEP_SHORT;
}

// void lcd_set_function_set(bool dataLength, )



int main() {

    // stdio_init_all();
    // sleep_ms(4000);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 0);

    LCD lcd = lcd_init(pio0, 0, 0, 2400);


    lcd_clear_display_blocking(lcd);
    lcd_set_cursor_style_blocking(lcd, true, true);
    
    lcd_set_cursor_pos_blocking(lcd, 0,0);

    char *str = "Hello";
    lcd_send_message_blocking(lcd, str, 5);

    gpio_put(25,1);

    // while(1) {
    //     gpio_put(25, 1);
    //     sendCommand(1);
    //     sleep_ms(500);
    //     gpio_put(25, 0);
    //     sleep_ms(500);
    // }

}