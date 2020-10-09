/*
 * SSD1306Commands.h
 *
 *  Created on: Oct 8, 2020
 *      Author: Tim Hewitt
 */

#ifndef INC_SSD1306COMMANDS_H_
#define INC_SSD1306COMMANDS_H_

// This is the I2C address (8 bit)
//  There are two possible addresses: with D/C# (pin 13) grounded, the address is 0x78,
//  with D/C# tied high it is 0x7A. Assume grounded by default.
#define SSD1306_SA0                0x78
#define SSD1306_SA1                0x7A
#define SSD1306_DEF_SA             SSD1306_SA0

// Display dimensions
#define ROWS                       64
#define COLUMNS                    128
#define PAGES                      (ROWS / 8)
#define MAX_PAGE                   (PAGES - 1)
#define MAX_ROW                    (ROWS - 1)
#define MAX_COLUMN                 (COLUMNS - 1)

// Character dimensions 8x8 font
#define CHARS                      (COLUMNS / FONT8x8_WIDTH)

// Command and Datamode
#define COMMAND_MODE               0x80 // continuation bit is set!
#define DATA_MODE                  0x40

// Commands and Parameter defines
#define SET_LOWER_COLUMN           0x00 // | with lower nibble  (Page mode only)
#define SET_HIGHER_COLUMN          0x10 // | with higher nibble (Page mode only)

#define HORIZONTAL_ADDRESSING_MODE 0x00
#define VERTICAL_ADDRESSING_MODE   0x01
#define PAGE_ADDRESSING_MODE       0x02
#define SET_MEMORY_ADDRESSING_MODE 0x20 // takes one byte as given above

#define SET_COLUMN_ADDRESS         0x21 // takes two bytes, start address and end address of display data RAM
#define SET_PAGE_ADDRESS           0x22 // takes two bytes, start address and end address of display data RAM

// Command maybe unsupported by SSD1308
#define FADE_INTERVAL_8_FRAMES     0x00
#define FADE_INTERVAL_16_FRAMES    0x01
#define FADE_INTERVAL_24_FRAMES    0x02
#define FADE_INTERVAL_32_FRAMES    0x03
#define FADE_INTERVAL_64_FRAMES    0x07
#define FADE_INTERVAL_128_FRAMES   0x0F
#define FADE_BLINK_DISABLE         0x00
#define FADE_OUT_ENABLE            0x20
#define BLINK_ENABLE               0x30
#define SET_FADE_BLINK             0x23 // takes one byte
                                        //  bit5-4 = 0, fade/blink mode
                                        //  bit3-0 = Time interval in frames

#define SET_DISPLAY_START_LINE     0x40 // | with a row number 0-63 to set start row. (Reset = 0)

#define SET_CONTRAST               0x81 // takes one byte, 0x00 - 0xFF

#define SET_SEGMENT_REMAP_0        0xA0 // column address 0 is mapped to SEG0 (Reset)
#define SET_SEGMENT_REMAP_127      0xA1 // column address 127 is mapped to SEG0

#define SET_DISPLAY_GDDRAM         0xA4 // restores display to contents of RAM
#define SET_ENTIRE_DISPLAY_ON      0xA5 // turns all pixels on, does not affect RAM

#define SET_NORMAL_DISPLAY         0xA6 // a databit of 1 indicates pixel 'ON'
#define SET_INVERSE_DISPLAY        0xA7 // a databit of 1 indicates pixel 'OFF'

#define SET_MULTIPLEX_RATIO        0xA8 // takes one byte, from 16xMUX to 64xMUX (MUX Ratio = byte+1; Default 64)

#define EXTERNAL_IREF              0x10
#define INTERNAL_IREF              0x00
#define SET_IREF_SELECTION         0xAD // sets internal or external Iref

#define SET_DISPLAY_POWER_OFF      0xAE
#define SET_DISPLAY_POWER_ON       0xAF

#define PAGE0                      0x00
#define PAGE1                      0x01
#define PAGE2                      0x02
#define PAGE3                      0x03
#define PAGE4                      0x04
#define PAGE5                      0x05
#define PAGE6                      0x06
#define PAGE7                      0x07
#define SET_PAGE_START_ADDRESS     0xB0 // | with a page number to get start address (Page mode only)

#define SET_COMMON_REMAP_0         0xC0 // row address  0 is mapped to COM0 (Reset)
#define SET_COMMON_REMAP_63        0xC8 // row address 63 is mapped to COM0

#define SET_DISPLAY_OFFSET         0xD3 // takes one byte from 0-63 for vertical shift, Reset = 0

#define SET_DISPLAY_CLOCK          0xD5 // takes one byte
                                        //  bit7-4 = Osc Freq DCLK (Reset = 1000b)
                                        //  bit3-0 = Divide ration (Reset = oooob, Ratio = 1)

#define SET_PRECHARGE_TIME         0xD9 // takes one byte
                                        //  bit7-4 = Phase2, upto 15 DCLKs (Reset = 0010b)
                                        //  bit3-0 = Phase1, upto 15 DCLKs (Reset = 0010b)

#define SET_CHARGE_PUMP			   0x8D

#define COMMON_BASE                0x02 //
#define COMMON_SEQUENTIAL          0x00 // Sequential common pins config
#define COMMON_ALTERNATIVE         0x10 // Odd/Even common pins config (Reset)
#define COMMON_LEFTRIGHT_NORMAL    0x00 // LeftRight Normal (Reset)
#define COMMON_LEFTRIGHT_FLIP      0x20 // LeftRight Flip
#define SET_COMMON_CONF            0xDA // takes one byte as given above


#define VCOMH_DESELECT_0_65_CODE   0x00
#define VCOMH_DESELECT_0_77_CODE   0x20
#define VCOMH_DESELECT_0_83_CODE   0x30
#define SET_VCOMH_DESELECT_LEVEL   0xDB // takes one byte as given above

#define NOP                        0xE3

#define SCROLL_INTERVAL_5_FRAMES   0x00
#define SCROLL_INTERVAL_64_FRAMES  0x01
#define SCROLL_INTERVAL_128_FRAMES 0x02
#define SCROLL_INTERVAL_256_FRAMES 0x03
#define SCROLL_INTERVAL_3_FRAMES   0x04
#define SCROLL_INTERVAL_4_FRAMES   0x05
#define SCROLL_INTERVAL_25_FRAMES  0x06
#define SCROLL_INTERVAL_2_FRAMES   0x07

#define SET_RIGHT_HOR_SCROLL       0x26 // takes 6 bytes: 0x00, PageStart, Scroll_Interval, PageEnd, 0x00, 0xFF
#define SET_LEFT_HOR_SCROLL        0x27 // takes 6 bytes: 0x00, PageStart, Scroll_Interval, PageEnd, 0x00, 0xFF

#define SET_VERT_RIGHT_HOR_SCROLL  0x29 // takes 5 bytes: 0x00, PageStart, Scroll_Interval, PageEnd, VertOffset
#define SET_VERT_LEFT_HOR_SCROLL   0x2A // takes 5 bytes: 0x00, PageStart, Scroll_Interval, PageEnd, VertOffset

#define SET_DEACTIVATE_SCROLL      0x2E
#define SET_ACTIVATE_SCROLL        0x2F

#define SET_VERTICAL_SCROLL_AREA   0xA3 // takes 2 bytes: Rows in Top Area (Reset=0), Rows in Scroll Area (Reset=64)

#endif /* INC_SSD1306COMMANDS_H_ */
