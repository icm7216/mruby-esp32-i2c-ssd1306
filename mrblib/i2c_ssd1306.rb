module OLED
  class SSD1306
    attr_accessor :color
    attr_accessor :fontsize
    
    def initialize(i2c, addr=0x3c, color=1, fontsize=1, options={})
      @i2c = i2c
      @addr = addr
      @color = color
      @fontsize = fontsize
    end
    
    def init
      # see controller data sheet
      @i2c.send("\x00\xAE", @addr)          # display OFF
      @i2c.send("\x00\xA8\x3F", @addr)      # MUX ratio (0x3F = 64d -1d)
      @i2c.send("\x00\xD3\x00", @addr)      # set display offset (no offset)
      @i2c.send("\x00\x40", @addr)          # set display start line
      @i2c.send("\x00\xA1", @addr)          # re-map, SEG0 is mapped to column address 127
      @i2c.send("\x00\xC8", @addr)          # scan direction, reverse up-bottom
      @i2c.send("\x00\xDA\x12", @addr)      # set COM pins (Alternative configuration, Disable L/R remap)

      @i2c.send("\x00\x81\x7F", @addr)      # set contrast
      @i2c.send("\x00\x2E", @addr)          # stop scrolling
      @i2c.send("\x00\xA4", @addr)          # resume ram content display
      @i2c.send("\x00\xd5\x00", @addr)      # set osc frequency
      @i2c.send("\x00\x8D\x14", @addr)      # enable charge pump
      
      @i2c.send("\x00\x20\x00", @addr)      # ADDR_MODE, 0x00 = Horizontal Mode
      @i2c.send("\x00\x21\x00\x7f", @addr)  # COLUMN_ADDR, 0x00 = start, 0x7f = end
      @i2c.send("\x00\x22\x00\x07", @addr)  # PAGE_ADDR, 0x00 = start, 0x7f = end
      @i2c.send("\x00\xAF", @addr)          # display ON 
      System.delay(200)
      self
    end

    def ready?
      @i2c.ready?(@addr)
    end
  end
end