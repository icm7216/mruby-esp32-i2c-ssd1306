#ifndef I2CSSD1306H_
#define I2CSSD1306H_

// Control byte
#define CONTROLBYTE_CMDSINGLE       0x80
#define CONTROLBYTE_CMDSTREAM       0x00
#define CONTROLBYTE_DATASTREAM      0x40

// Fundamental Commands
#define SET_CONTRAST                0x81    // 0x7F = default
#define RESUME_RAM_CONTENT_DISPLAY  0xA4
#define DISPLAY_OFF                 0xAE
#define DISPLAY_ON                  0xAF

// Scrolling Commands
#define STOP_SCROLLING              0x2E

// Addressing Setting Commands
#define SET_MEMORY_ADDR_MODE        0x20    // 0x00 = Horizontal Mode
#define SET_COLUMN_ADDR             0x21    // 0x00 = start, 0x7f = end
#define SET_PAGE_ADDR               0x22    // 0x00 = start, 0x07 = end

// Hardware Configuration
#define SET_DISPLAY_START_LINE      0x40    // start line is 0d
#define REMAP                       0xA1    // SEG0 is mapped to column address 127
#define MUX_RATIO                   0xA8    // 0x3F = 64d -1d
#define SCAN_DIRECTION              0xC8    // SCAN_DIRECTION, reverse up-bottom
#define SET_DISPLAY_OFFSET          0xD3    // 0x00 = no offset
#define SET_COM_PINS                0xDA    // 0x12 = Alternative configuration, Disable L/R remap
#define SET_OSC_FREQUENCY           0xd5    // 0x00

// Charge Pump Command
#define SET_CHARGE_PUMP             0x8D    // 0x14 = enable charge pump

#endif /* I2CSSD1306H_ */
