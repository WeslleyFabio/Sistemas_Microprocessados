
/** Put this in the src folder **/

#include "i2c-lcd.h"
  // change your handler here accordingly
I2C_HandleTypeDef *_hi2c;
uint16_t _lcd_address;
uint8_t _lcd_rows;
uint8_t _lcd_cols;

//#define SLAVE_ADDRESS_LCD 0x4E // change this according to ur setup



void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0 1100
	data_t[1] = data_u|0x08;  //en=0, rs=0 1000
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	HAL_I2C_Master_Transmit (_hi2c, _lcd_address,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);

	// colocar delay entre as transmiçoes seguindo o padrao arduino.
//													light    en				rs
	data_t[0] = data_u|0x0D;  //en=1, rs=0 1101     1	     1      0       1
	data_t[1] = data_u|0x09;  //en=0, rs=0 1001
	data_t[2] = data_l|0x0D;  //en=1, rs=0
	data_t[3] = data_l|0x09;  //en=0, rs=0
	HAL_I2C_Master_Transmit (_hi2c, _lcd_address,(uint8_t *) data_t, 4, 100);
}

void lcd_clear (void)
{
	lcd_send_cmd (0x01);


//	for (int i=0; i<70; i++)
//	{
//		lcd_send_data (' ');
//	}
}

void lcd_put_cur(int row, int col)
{
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

	if(row > _lcd_rows)
	{
		row = _lcd_rows;
	}

	if(col > _lcd_cols)
	{
		col = _lcd_cols;
	}

    lcd_send_cmd (0x80 | (col + row_offsets[row]));
}


void lcd_init (I2C_HandleTypeDef * i2c, uint16_t address, uint8_t rows, uint8_t cols)
{

	_lcd_address = address;
	_lcd_rows = rows;
	_hi2c = i2c;
	_lcd_cols = cols;

	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd (0x30);
	HAL_Delay(5);  // wait for >4.1ms
	lcd_send_cmd (0x30);
	HAL_Delay(1);  // wait for >100us
	lcd_send_cmd (0x30);
	HAL_Delay(10);
	lcd_send_cmd (0x20);  // 4bit mode
	HAL_Delay(10);

  // dislay initialisation
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	lcd_send_cmd (0x01);  // clear display
	HAL_Delay(2);
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}

void lcd_create_char(uint8_t id, uint8_t charMap[8])
{
	id &= 0x7; // we only have 8 locations 0-7
	lcd_send_cmd(LCD_SETCGRAMADDR | (id << 3));
	for (int i=0; i<8; i++) {
		lcd_send_data(charMap[i]);
	}
}
