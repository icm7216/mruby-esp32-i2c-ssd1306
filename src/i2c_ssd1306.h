#ifndef I2CSSD1306H_
#define I2CSSD1306H_

#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64
#define DISPLAY_PIXSEL (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)
#define FONT_WIDTH      8
#define FONT_HEIGHT     8 

#define BLACK   0
#define WHITE   1
#define INVERT  2

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


// manipulate the graphics
#define swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void buffer_clear();
void set_pixel(int16_t x, int16_t y, uint16_t color) ;
int16_t get_pixel(int16_t x, int16_t y);
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color);
void draw_vertical_line(int16_t x, int16_t y, int16_t h, int16_t color);
void draw_horizontal_line(int16_t x, int16_t y, int16_t w, int16_t color);
void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t color);
void draw_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t color);
void draw_circle(int16_t x0, int16_t y0, int16_t r, int16_t color);
void draw_fill_circle(int16_t x0, int16_t y0, int16_t r, int16_t color);

// Display a character string
void draw_char(int16_t x, int16_t y, uint8_t c, int16_t color, int16_t fontsize);
void display_text(int16_t x, int16_t y, uint8_t *text, int16_t length, int16_t color, int16_t fontsize);

// mruby binding of manipulate the graphics
static mrb_value ssd1306_clear(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_set_pixel(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_get_pixel(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_draw_line(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_draw_vertical_line(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_draw_horizontal_line(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_draw_rect(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_draw_fill_rect(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_draw_circle(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_draw_fill_circle(mrb_state *mrb, mrb_value self);

// mruby binding of Display a character string
static mrb_value ssd1306_display(mrb_state *mrb, mrb_value self);
static mrb_value ssd1306_text(mrb_state *mrb, mrb_value self);


#endif /* I2CSSD1306H_ */