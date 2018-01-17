# mruby-esp32-i2c-ssd1306

OLED SSD1306 (I2C) library for mruby-esp32.

This library is a for the SSD1306 based 128x64 pixel OLED display running on the mruby-esp32.

# Installation

Add the following line below to your `esp32_build_config.rb`:

```ruby
  conf.gem :github => 'icm7216/mruby-esp32-i2c-ssd1306'
```

# Example

Example of controlling the OLED display with SSD1306 controller.

## Demo movie

![movie](ESP32-SSD1306_sample.gif)

In advance, you will need to add several mrbgems to `esp32_build_config.rb`
```ruby
  conf.gem :core => "mruby-math"
  conf.gem :core => "mruby-proc-ext"
```
## Code
```ruby
include ESP32

i2c = I2C.new(I2C::PORT0, scl: 22, sda: 21).init(I2C::MASTER)
oled = OLED::SSD1306.new(i2c)
oled.init
oled.clear
System.delay(100)
msg = ["Hello! mruby", "mruby-ESP32", "mruby", "ESP32"]

oled.fontsize = 1
oled.text(20, 0, msg[0])
oled.text(20, 12, msg[1])
oled.fontsize = 2
oled.text(20, 25, msg[0])
oled.text(20, 45, msg[1])
oled.display
System.delay(1000)
oled.clear

oled.rect(0, 0, 127, 63)
oled.vline(63, 10, 43)
oled.hline(32, 31, 60)
oled.circle(63, 31, 30)
oled.fill_circle(15, 31, 10)
oled.fill_rect(105, 21, 20, 20)
oled.display
System.delay(1000)
oled.color = OLED::INVERT
oled.fontsize = 3
oled.text(25, 5, msg[2])
oled.text(25, 36, msg[3])
oled.display
System.delay(2000)
oled.clear

oled.color = OLED::WHITE
x = 0
7.times do |n|
  fontsize = n + 1
  w = 8 * ((fontsize.div 2) + (fontsize & 0x01))
  x = x + 8 * (fontsize.div 2)
  oled.fontsize = fontsize
  oled.text(x, 0, fontsize.to_s)
end
oled.display
System.delay(1000)
oled.clear

# drawing Spirograph
rc = 15                           # Radius of the constant circle
rm = 9                            # Radius of the motion circle
rd = 8                            # Radius of the Drawing point
x_offset = 64
y_offset = 32

# trochoid functions
fx = ->(t){(rc + rm) * Math.cos(t) - rd * Math.cos(t * (rc + rm) / rm) + x_offset}
fy = ->(t){(rc + rm) * Math.sin(t) - rd * Math.sin(t * (rc + rm) / rm) + y_offset}

# greatest common divisor
def my_gcd(a, b)
  a, b = b, a % b while b > 0
  a
end

laps = rm / my_gcd(rc,rm)         # Number of laps
to = 2.0 * Math::PI * laps        # rotation angle
div = 0.3                         # angle
theta_end = (to + div).div div    # Drawing end angle

theta_end.times do |i|
  x0 = fx[div * i]
  y0 = fy[div * i]
  x1 = fx[div * (i + 1)]
  y1 = fy[div * (i + 1)]
  oled.line(x0, y0, x1, y1)
  oled.display
end

i2c.deinit
```


# using library

**Many thanks!**

*   I2C library for mruby-esp32 by [mruby-esp32/mruby-esp32-i2c
](https://github.com/mruby-esp32/mruby-esp32-i2c)
*   8x8 monochrome bitmap fonts from font8x8_basic.h by [dhepper/font8x8](https://github.com/dhepper/font8x8)
*   graphics libraries are adapted from OLEDDisplay.cpp by [squix78/esp8266-oled-ssd1306](https://github.com/squix78/esp8266-oled-ssd1306)


# License

MIT
