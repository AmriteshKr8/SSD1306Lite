#ifndef SSD1306_LITE_H
#define SSD1306_LITE_H

#include <Arduino.h>
#include <Wire.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_DEFAULT_ADDRESS 0x3C

enum DrawMode
{
    PIXEL_ON,
    PIXEL_OFF,
    PIXEL_XOR
};

class SSD1306Lite
{
public:
    SSD1306Lite(uint8_t addr = OLED_DEFAULT_ADDRESS);
    void begin(int sda, int scl);
    void update();
    void clear();
    void fill();
    void setPixel(int x, int y, DrawMode mode);
    void drawLine(int x0, int y0, int x1, int y1, int thickness, DrawMode mode);
    void drawRect(int x, int y, int w, int h, bool filled, int thickness, DrawMode mode);
    void drawCircle(int xc, int yc, int r, bool filled, int thickness, DrawMode mode);
    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, bool filled, int thickness, DrawMode mode);
    void drawArc(int xc, int yc, int r, int startDeg, int endDeg, int thickness, DrawMode mode);
    void drawArcFast(int xc, int yc, int r, int startDeg, int endDeg, int thickness, DrawMode mode);
    void drawBitmap(int x, int y, const uint8_t *bmp, int w, int h, int scale, DrawMode mode);
    void drawChar(int x, int y, char c, int scale, DrawMode mode);
    void drawText(int x, int y, const char *str, int scale, DrawMode mode);
    void drawTextCentered(int y, const char *str, int scale, DrawMode mode);
    int drawTextBox(int x, int y, int w, int h, const char *str, int firstLine, int scale, DrawMode mode, bool scrollBar = true);

private:
    uint8_t address;
    uint8_t buffer[OLED_WIDTH * OLED_HEIGHT / 8];
    bool dirty = true;
    void command(uint8_t cmd);
    void drawCharClipped(int x, int y, char c, int scale, int clipX, int clipY, int clipW, int clipH, DrawMode mode);

public:
    bool isDirty() const;
    void markDirty();
};

#endif