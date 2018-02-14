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
#include "tiny_grafx.h"

static const char *TAG = "SSD1306";


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
  uint8_t addr, *buffer;
  i2c_cmd_handle_t cmd;
  esp_err_t err;

  port = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@port"));
  addr = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@addr")));

  // get frame buffer
  buffer = mrb_malloc(mrb, DISPLAY_PIXSEL);
  memset(buffer, 0, DISPLAY_PIXSEL);
  buffer_read(buffer, DISPLAY_PIXSEL);

  // send data to OLED
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
    if (err != ESP_OK) {
    ESP_LOGI(TAG, "ssd1306_display error: %d", err);
  }
  mrb_free(mrb, buffer);

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
