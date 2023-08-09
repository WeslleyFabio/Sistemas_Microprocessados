#include "main.h"

#define LCD_SETCGRAMADDR 0x40

void lcd_init (I2C_HandleTypeDef * i2c, uint16_t address, uint8_t rows, uint8_t cols);

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_put_cur(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (void);

void lcd_create_char(uint8_t id, uint8_t charMap[8]);
