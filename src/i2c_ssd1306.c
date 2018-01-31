#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>

#include <stdio.h>
#include <string.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "i2c_ssd1306.h"

// 8x8 monochrome bitmap fonts from font8x8_basic.h by dhepper/font8x8
// https://github.com/dhepper/font8x8
#include "font8x8_basic.h"

static const char *TAG = "SSD1306";

// Pixel Buffer
static uint8_t buffer[DISPLAY_PIXSEL];


// manipulate graphics
//
// This graphics libraries are adapted from OLEDDisplay.cpp by squix78/esp8266-oled-ssd1306.
// https://github.com/squix78/esp8266-oled-ssd1306
//
void 
buffer_clear() 
{
  memset(buffer, 0x00, DISPLAY_PIXSEL);
}

void 
set_pixel(int16_t x, int16_t y, uint16_t color) 
{
  if ((x >= 0) && (x < DISPLAY_WIDTH) && (y >= 0) && (y < DISPLAY_HEIGHT)) {
    switch (color) {
      case WHITE:   buffer[x + (y / 8) * DISPLAY_WIDTH] |=  (1 << (y & 7)); break;
      case BLACK:   buffer[x + (y / 8) * DISPLAY_WIDTH] &= ~(1 << (y & 7)); break;
      case INVERT:  buffer[x + (y / 8) * DISPLAY_WIDTH] ^=  (1 << (y & 7)); break;
    }
  } 
}

int16_t 
get_pixel(int16_t x, int16_t y) 
{
  if ((x >= 0) && (x < DISPLAY_WIDTH) && (y >= 0) && (y < DISPLAY_HEIGHT)) {
    return (buffer[x + (y / 8) * DISPLAY_WIDTH] >> (y % 8)) & 0x1;
  }
  else {
    return 0;
  }
}

void 
draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color) 
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (steep) {
    swap_int16_t(x0, y0);
    swap_int16_t(x1, y1);
  }
  if (x0 > x1) {
    swap_int16_t(x0, x1);
    swap_int16_t(y0, y1);
  }

  int16_t dx = x1 - x0;
  int16_t dy = abs(y1 - y0);
  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  }
  else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      set_pixel(y0, x0, color);
    }
    else {
      set_pixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void 
draw_vertical_line(int16_t x, int16_t y, int16_t h, int16_t color) 
{
  if (y < 0) {
    h += y;
    y = 0;
  }

  if ((y + h) > DISPLAY_HEIGHT) {
    h = DISPLAY_HEIGHT - y; 
  }

  if (h <= 0) return;
  
  for (int16_t h1 = 0; h1 < h; h1++) {
    set_pixel(x, y + h1, color);
  }
}

void 
draw_horizontal_line(int16_t x, int16_t y, int16_t w, int16_t color) 
{
  if (x < 0) {
    w += x;
    x = 0;
  }

  if ((x + w) > DISPLAY_WIDTH) {
    w = DISPLAY_WIDTH - x; 
  }

  if (w <= 0) return;
  
  for (int16_t w1 = 0; w1 < w; w1++) {
    set_pixel(x + w1, y, color);
  }
}

void 
draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t color) 
{
  draw_horizontal_line(x, y, w, color);
  draw_horizontal_line(x, y + h - 1, w, color);
  draw_vertical_line(x, y, h, color);
  draw_vertical_line(x + w - 1, y, h, color);
}

void 
draw_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t color) 
{
  for (int16_t w1 = 0; w1 < w; w1++) {
    draw_vertical_line(x + w1, y, h, color);
  }
}

void 
draw_circle(int16_t x0, int16_t y0, int16_t r, int16_t color) 
{
  int16_t x = 0;
  int16_t y = r;
	int16_t dp = 1 - r;

  set_pixel(x0, y0 + r, color);
  set_pixel(x0, y0 - r, color);
  set_pixel(x0 + r, y0, color);
  set_pixel(x0 - r, y0, color);

	do {
		if (dp < 0) {
			dp = dp + 2 * (++x) + 3;
    }
    else {
			dp = dp + 2 * (++x) - 2 * (--y) + 5;
    }

		set_pixel(x0 + x, y0 + y, color);     //For the 8 octants
		set_pixel(x0 - x, y0 + y, color);
		set_pixel(x0 + x, y0 - y, color);
		set_pixel(x0 - x, y0 - y, color);
		set_pixel(x0 + y, y0 + x, color);
		set_pixel(x0 - y, y0 + x, color);
		set_pixel(x0 + y, y0 - x, color);
		set_pixel(x0 - y, y0 - x, color);

	} while (x < y);
}

void 
draw_fill_circle(int16_t x0, int16_t y0, int16_t r, int16_t color) 
{
  int16_t x = 0;
  int16_t y = r;
	int16_t dp = 1 - r;
  
  draw_horizontal_line(x0 - r, y0, 2 * r, color);

	do {
		if (dp < 0) {
			dp = dp + 2 * (++x) + 3;
    }
    else {
			dp = dp + 2 * (++x) - 2 * (--y) + 5;
    }

    draw_horizontal_line(x0 - x, y0 - y, 2 * x, color);
    draw_horizontal_line(x0 - x, y0 + y, 2 * x, color);
    draw_horizontal_line(x0 - y, y0 - x, 2 * y, color);
    draw_horizontal_line(x0 - y, y0 + x, 2 * y, color);
	} while (x < y);
}

// Display a character string
void 
draw_char(int16_t x, int16_t y, uint8_t c, int16_t color, int16_t fontsize) 
{
  uint8_t row_pixel;
  uint16_t font_width;

  for (int16_t y1 = 0; y1 < FONT_HEIGHT; y1++) {  
    row_pixel = font8x8_basic[c][y1];

    for (int16_t x1 = 0; x1 < FONT_WIDTH; x1++) {
      if (row_pixel & 0x01) {
        if (fontsize == 1) {
          set_pixel(x + x1, y + y1, color);
        }
        else {
          font_width = (fontsize & 0x01) + (fontsize / 2);
          draw_fill_rect(x + x1 * font_width, y + y1 * fontsize, font_width, fontsize, color);
        }
      }
      row_pixel >>= 1;
    }
  }
  // ESP_LOGI(TAG, "draw char: 0x%X=%c", c, c);
}

void 
display_text(int16_t x, int16_t y, uint8_t *text, int16_t length, int16_t color, int16_t fontsize) 
{
  // ESP_LOGI(TAG, "display text: %s, length: %d, fontsize: %d", text, length, fontsize);
  uint16_t font_width;

  for (int16_t i = 0; i < length; i++) {
    if (text[i] == '\n') {
      x =0;
      y += FONT_WIDTH * fontsize;
    }
    else {
      draw_char(x, y, text[i], color, fontsize);
      if (fontsize == 1) {
        x += FONT_WIDTH * fontsize;
      }
      else {
        font_width = (fontsize & 0x01) + (fontsize / 2);
        x += FONT_WIDTH * font_width;
      }
    }
  }
}




// mruby binding of manipulate the graphics
static mrb_value
ssd1306_clear(mrb_state *mrb, mrb_value self)
{
  buffer_clear();
  return self;
}

static mrb_value
ssd1306_set_pixel(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y;  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "ii", &x, &y);

	set_pixel(x, y, color); 
  return mrb_nil_value();
}

static mrb_value
ssd1306_get_pixel(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y;  
  int16_t pixel;
  mrb_get_args(mrb, "ii", &x, &y);
	pixel = get_pixel(x, y);

  return mrb_fixnum_value(pixel);
}

static mrb_value
ssd1306_draw_line(mrb_state *mrb, mrb_value self) 
{
  mrb_int x0, y0, x1, y1;
  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  
  mrb_get_args(mrb, "iiii", &x0, &y0, &x1, &y1);

  if ((color < BLACK) || (color > INVERT)) {  
    color = WHITE;
  }
  draw_line(x0, y0, x1, y1, color);
  
  return mrb_nil_value();
}

static mrb_value
ssd1306_draw_vertical_line(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, h;  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &h);

	draw_vertical_line(x, y, h, color);
	return mrb_nil_value();
}

static mrb_value
ssd1306_draw_horizontal_line(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, w;  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &w);

	draw_horizontal_line(x, y, w, color);
	return mrb_nil_value();
}

static mrb_value
ssd1306_draw_rect(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, w, h;  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iiii", &x, &y, &w, &h);

	draw_rect(x, y, w, h, color); 
	return mrb_nil_value();
}

static mrb_value
ssd1306_draw_fill_rect(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, w, h;  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iiii", &x, &y, &w, &h);

	draw_fill_rect(x, y, w, h, color); 
	return mrb_nil_value();
}

static mrb_value
ssd1306_draw_circle(mrb_state *mrb, mrb_value self)
{
	mrb_int x, y, r;  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &r);

	draw_circle(x, y, r, color); 
	return mrb_nil_value();
}

static mrb_value
ssd1306_draw_fill_circle(mrb_state *mrb, mrb_value self)
{
  mrb_int x, y, r;  
  int16_t color;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  mrb_get_args(mrb, "iii", &x, &y, &r);

	draw_fill_circle(x, y, r, color); 
	return mrb_nil_value();
}

// mruby binding of Display a character string
static mrb_value
ssd1306_display(mrb_state *mrb, mrb_value self)
{
  mrb_value port;
  uint8_t addr;
  i2c_cmd_handle_t cmd;
  esp_err_t err;

  port = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@port"));
  addr = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@addr")));

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1 ) | I2C_MASTER_WRITE, true);  
  i2c_master_write_byte(cmd, CONTROLBYTE_DATASTREAM, true);
  for (uint16_t i = 0; i < DISPLAY_PIXSEL; i++) {
    i2c_master_write_byte(cmd, buffer[i], true);
  }
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(mrb_fixnum(port), cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  return mrb_fixnum_value(err);
}   

static mrb_value
ssd1306_text(mrb_state *mrb, mrb_value self) 
{
  mrb_int x, y;
  mrb_value data;

  int16_t color, fontsize;
  color = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@color")));
  fontsize = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@fontsize")));

  mrb_get_args(mrb, "iiS", &x, &y, &data);
  display_text(x, y, RSTRING_PTR(data), RSTRING_LEN(data), color, fontsize);
  
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
  
  // manipulate graphics
  mrb_define_method(mrb, ssd1306, "clear", ssd1306_clear, MRB_ARGS_NONE());
  mrb_define_method(mrb, ssd1306, "set_pixel", ssd1306_set_pixel, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, ssd1306, "get_pixel", ssd1306_get_pixel, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, ssd1306, "line", ssd1306_draw_line, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, ssd1306, "vline", ssd1306_draw_vertical_line, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, ssd1306, "hline", ssd1306_draw_horizontal_line, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, ssd1306, "rect", ssd1306_draw_rect, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, ssd1306, "fill_rect", ssd1306_draw_fill_rect, MRB_ARGS_REQ(4));
  mrb_define_method(mrb, ssd1306, "circle", ssd1306_draw_circle, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, ssd1306, "fill_circle", ssd1306_draw_fill_circle, MRB_ARGS_REQ(3));

  // Display a character string
  mrb_define_method(mrb, ssd1306, "display", ssd1306_display, MRB_ARGS_NONE());
  mrb_define_method(mrb, ssd1306, "text", ssd1306_text, MRB_ARGS_REQ(3));
  
}

void
mrb_mruby_esp32_i2c_ssd1306_gem_final(mrb_state* mrb)
{
}
