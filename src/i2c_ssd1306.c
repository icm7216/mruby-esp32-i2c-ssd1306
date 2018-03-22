#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>
#include <mruby/class.h>
#include <mruby/data.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "tiny_grafx.h"

// SSD1306 control byte
#define SSD1306I2C_CONTROLBYTE_CMDSINGLE       0x80
#define SSD1306I2C_CONTROLBYTE_CMDSTREAM       0x00
#define SSD1306I2C_CONTROLBYTE_DATASTREAM      0x40

/* ------------------------------------------------
  As a reference, leave the configuration command information.
// Fundamental Commands 
#define SSD1306I2C_SET_CONTRAST                0x81    // 0x7F = default
#define SSD1306I2C_RESUME_RAM_CONTENT_DISPLAY  0xA4
#define SSD1306I2C_DISPLAY_OFF                 0xAE
#define SSD1306I2C_DISPLAY_ON                  0xAF
// Scrolling Commands
#define SSD1306I2C_STOP_SCROLLING              0x2E
// Addressing Setting Commands
#define SSD1306I2C_SET_MEMORY_ADDR_MODE        0x20    // 0x00 = Horizontal Mode
#define SSD1306I2C_SET_COLUMN_ADDR             0x21    // 0x00 = start, 0x7f = end
#define SSD1306I2C_SET_PAGE_ADDR               0x22    // 0x00 = start, 0x07 = end
// Hardware Configuration
#define SSD1306I2C_SET_DISPLAY_START_LINE      0x40    // start line is 0d
#define SSD1306I2C_REMAP                       0xA1    // SEG0 is mapped to column address 127
#define SSD1306I2C_MUX_RATIO                   0xA8    // 0x3F = 64d -1d
#define SSD1306I2C_SCAN_DIRECTION              0xC8    // SCAN_DIRECTION, reverse up-bottom
#define SSD1306I2C_SET_DISPLAY_OFFSET          0xD3    // 0x00 = no offset
#define SSD1306I2C_SET_COM_PINS                0xDA    // 0x12 = Alternative configuration, Disable L/R remap
#define SSD1306I2C_SET_OSC_FREQUENCY           0xd5    // 0x00
// Charge Pump Command
#define SSD1306I2C_SET_CHARGE_PUMP             0x8D    // 0x14 = enable charge pump
--------------------------------------------------- */

// SSD1306 display config
#define SSD1306_DISPLAY_WIDTH   128
#define SSD1306_DISPLAY_HEIGHT  64
#define SSD1306_DISPLAY_PIXEL   1024
#define SSD1306_FONT_WIDTH      8
#define SSD1306_FONT_HEIGHT     8

static const char *TAG = "SSD1306";


// ----- Common graphics methods ----------
// mruby binding of manipulate the graphics
// ----------------------------------------
static mrb_value
lcd_clear(mrb_state *mrb, mrb_value self)
{
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);

  buffer_clear(*tg);
  return self;
}

static mrb_value
lcd_set_pixel(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "ii", &x, &y);
	
  set_pixel(*tg, x, y, color);
  return mrb_nil_value();
}

static mrb_value
lcd_get_pixel(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y;
  int16_t pixel;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  mrb_get_args(mrb, "ii", &x, &y);
  
  pixel = get_pixel(*tg, x, y);
  return mrb_fixnum_value(pixel);
}

static mrb_value
lcd_draw_line(mrb_state *mrb, mrb_value self)
{
  mrb_int x0, y0, x1, y1;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iiii", &x0, &y0, &x1, &y1);
  if ((color < BLACK) || (color > INVERT)) {
    color = WHITE;
  }

  draw_line(*tg, x0, y0, x1, y1, color);
  return mrb_nil_value();
}

static mrb_value
lcd_draw_vertical_line(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, h;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &h);
	
  draw_vertical_line(*tg, x, y, h, color);
  return mrb_nil_value();
}

static mrb_value
lcd_draw_horizontal_line(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, w;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &w);
	
  draw_horizontal_line(*tg, x, y, w, color);
	return mrb_nil_value();
}

static mrb_value
lcd_draw_rect(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, w, h;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iiii", &x, &y, &w, &h);
	
  draw_rect(*tg, x, y, w, h, color);
	return mrb_nil_value();
}

static mrb_value
lcd_draw_fill_rect(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, w, h;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iiii", &x, &y, &w, &h);
	
  draw_fill_rect(*tg, x, y, w, h, color);
	return mrb_nil_value();
}

static mrb_value
lcd_draw_circle(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, r;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &r);
	
  draw_circle(*tg, x, y, r, color);
	return mrb_nil_value();
}

static mrb_value
lcd_draw_fill_circle(mrb_state *mrb, mrb_value self)
{
  mrb_int x, y, r;
  int16_t color;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &r);
	
  draw_fill_circle(*tg, x, y, r, color);
	return mrb_nil_value();
}

// mruby binding of Display a character string
static mrb_value
lcd_text(mrb_state *mrb, mrb_value self)
{
  mrb_int x, y;
  mrb_value data;
  int16_t color, fontsize;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  fontsize = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@fontsize")));
  mrb_get_args(mrb, "iiS", &x, &y, &data);
  
  display_text(*tg, x, y, RSTRING_PTR(data), RSTRING_LEN(data), color, fontsize);
  // ESP_LOGI(TAG, "color:%d, size:%d, text:%s", color, fontsize, RSTRING_PTR(data));
  return mrb_nil_value();
}
// ----- Common graphics methods -----



// ----- SSD1306 methods and functions -----

static mrb_value
ssd1306_display(mrb_state *mrb, mrb_value self)
{
  mrb_value port;
  uint8_t addr, *buffer;
  i2c_cmd_handle_t cmd;
  esp_err_t err;
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);

  port = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@port"));
  addr = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@addr")));

  // get frame buffer
  buffer = (uint8_t *)mrb_malloc(mrb, tg->display_pixel);
  if (buffer != NULL) {
    memset(buffer, 0x00, tg->display_pixel);
  }
  buffer_read(*tg, buffer, tg->display_pixel);
  
  // send data to OLED
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1 ) | I2C_MASTER_WRITE, true);  
  i2c_master_write_byte(cmd, SSD1306I2C_CONTROLBYTE_DATASTREAM, true);
  for (uint16_t i = 0; i < tg->display_pixel; i++) {
    i2c_master_write_byte(cmd, buffer[i], true);
  }
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(mrb_fixnum(port), cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
    if (err != ESP_OK) {
    ESP_LOGI(TAG, "ssd1306_display error: %d", err);
  }
  mrb_free(mrb, buffer);

  return mrb_fixnum_value(err);
}   

// Configuration the Tiny graphics libraries
static void
tinygrafx_init(tinygrafx_t *tg)
{
  tg->display_width = SSD1306_DISPLAY_WIDTH;
  tg->display_height = SSD1306_DISPLAY_HEIGHT;
  tg->display_pixel = SSD1306_DISPLAY_PIXEL;
  tg->font_width = SSD1306_FONT_WIDTH;
  tg->font_height = SSD1306_FONT_HEIGHT;

  // set frame buffer
  uint8_t *buffer;
  buffer = (uint8_t *)malloc(tg->display_pixel);
  if (buffer != NULL) {
    memset(buffer, 0, tg->display_pixel);
  }
  tg->display_buffer = buffer; 
}

// free mrb object for GC.
static void
meb_ssd1306_free(mrb_state *mrb, void *ptr)
{
  tinygrafx_t *tg = ptr;
  mrb_free(mrb, tg->display_buffer);
}

// mruby data_type
static const struct mrb_data_type mrb_spi_config_type = {
  "spi_config_type", meb_ssd1306_free
};

static mrb_value
ssd1306_tinygrafx_init(mrb_state *mrb, mrb_value self) 
{
  tinygrafx_t *tg = (tinygrafx_t *)DATA_PTR(self);
  if (tg) {
    mrb_free(mrb, tg);
  }
  
  tg = (tinygrafx_t *)mrb_malloc(mrb, sizeof(tinygrafx_t));
  DATA_TYPE(self) = &mrb_spi_config_type;
  DATA_PTR(self)  = tg;

  // Initialize the TINYGRAFX
  tinygrafx_init(tg);
  
  return mrb_nil_value();
}

// mrbgem init
void
mrb_mruby_esp32_i2c_ssd1306_gem_init(mrb_state* mrb)
{
  struct RClass *oled = mrb_define_module(mrb, "OLED");
  mrb_define_const(mrb, oled, "BLACK", mrb_fixnum_value(BLACK));
  mrb_define_const(mrb, oled, "WHITE", mrb_fixnum_value(WHITE));
  mrb_define_const(mrb, oled, "INVERT", mrb_fixnum_value(INVERT));

  struct RClass *ssd1306 = mrb_define_class_under(mrb, oled, "SSD1306", mrb->object_class);
  MRB_SET_INSTANCE_TT(ssd1306, MRB_TT_DATA);
  
  // Common graphics methods
  mrb_define_method(mrb, ssd1306, "clear", lcd_clear, MRB_ARGS_NONE());
  mrb_define_method(mrb, ssd1306, "set_pixel", lcd_set_pixel, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, ssd1306, "get_pixel", lcd_get_pixel, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, ssd1306, "line", lcd_draw_line, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, ssd1306, "vline", lcd_draw_vertical_line, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, ssd1306, "hline", lcd_draw_horizontal_line, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, ssd1306, "rect", lcd_draw_rect, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, ssd1306, "fill_rect", lcd_draw_fill_rect, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, ssd1306, "circle", lcd_draw_circle, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, ssd1306, "fill_circle", lcd_draw_fill_circle, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, ssd1306, "text", lcd_text, MRB_ARGS_REQ(3));

  // Send frame buffer to display
  mrb_define_method(mrb, ssd1306, "display", ssd1306_display, MRB_ARGS_NONE());
  
  // Initialize the TINYGRAFX
  mrb_define_method(mrb, ssd1306, "_init", ssd1306_tinygrafx_init, MRB_ARGS_NONE());
}

void
mrb_mruby_esp32_i2c_ssd1306_gem_final(mrb_state* mrb)
{
}
