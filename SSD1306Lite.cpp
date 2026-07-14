#include "SSD1306Lite.h"
#include <math.h>

/* 5x7 font, ASCII 32–127 */
static const uint8_t font5x7[96][5] = {
{0,0,0,0,0},{0,0,95,0,0},{0,7,0,7,0},{20,127,20,127,20},
{36,42,127,42,18},{35,19,8,100,98},{54,73,86,32,80},{0,0,7,0,0},
{0,28,34,65,0},{0,65,34,28,0},{20,8,62,8,20},{8,8,62,8,8},
{0,160,96,0,0},{8,8,8,8,8},{0,96,96,0,0},{32,16,8,4,2},
{62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},
{24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},
{54,73,73,73,54},{6,73,73,41,30},{0,54,54,0,0},{0,182,102,0,0},
{8,20,34,65,0},{20,20,20,20,20},{0,65,34,20,8},{2,1,89,9,6},
{62,65,93,85,30},{126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},
{127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},
{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},
{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},
{127,9,9,9,6},{62,65,97,33,94},{127,9,25,41,70},{38,73,73,73,50},
{1,1,127,1,1},{63,64,64,64,63},{31,32,64,32,31},{127,32,24,32,127},
{99,20,8,20,99},{3,4,120,4,3},{97,81,73,69,67},{0,127,65,65,0},
{2,4,8,16,32},{0,65,65,127,0},{4,2,1,2,4},{64,64,64,64,64},
{0,1,2,4,0},{32,84,84,84,120},{127,72,68,68,56},{56,68,68,68,32},
{56,68,68,72,127},{56,84,84,84,24},{8,126,9,1,2},{24,164,164,164,124},
{127,8,4,4,120},{0,68,125,64,0},{64,128,128,122,0},{127,16,40,68,0},
{0,65,127,64,0},{124,4,24,4,120},{124,8,4,4,120},{56,68,68,68,56},
{252,36,36,36,24},{24,36,36,40,252},{124,8,4,4,8},{72,84,84,84,32},
{4,63,68,64,32},{60,64,64,32,124},{28,32,64,32,28},{60,64,48,64,60},
{68,40,16,40,68},{28,160,160,160,124},{68,100,84,76,68},{8,54,65,65,0},
{0,0,119,0,0},{0,65,65,54,8},{2,1,2,4,2},{124,68,68,68,124}
};

/* ================= INIT ================= */

SSD1306Lite::SSD1306Lite(uint8_t addr) {
    address = addr;
}

void SSD1306Lite::command(uint8_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(0x00);
    Wire.write(cmd);
    Wire.endTransmission();
}

void SSD1306Lite::begin(int sda, int scl) {
    Wire.begin(sda, scl);
    delay(100);

    command(0xAE);
    command(0x20); command(0x00);
    command(0xB0);
    command(0xC8);
    command(0x00);
    command(0x10);
    command(0x40);
    command(0x81); command(0x7F);
    command(0xA1);
    command(0xA6);
    command(0xA8); command(0x3F);
    command(0xA4);
    command(0xD3); command(0x00);
    command(0xD5); command(0xF0);
    command(0xD9); command(0x22);
    command(0xDA); command(0x12);
    command(0xDB); command(0x20);
    command(0x8D); command(0x14);
    command(0xAF);
}

/* ================= BUFFER ================= */

void SSD1306Lite::clear() {
    memset(buffer, 0, sizeof(buffer));
    dirty = true;
}

void SSD1306Lite::fill() {
    memset(buffer, 0xFF, sizeof(buffer));
    dirty = true;
}

/* ================= UPDATE ================= */

void SSD1306Lite::update() {

    if (!dirty)
        return;

    for (uint8_t page = 0; page < 8; page++) {

        command(0xB0 + page);
        command(0x00);
        command(0x10);

        for (int col = 0; col < OLED_WIDTH; col += 16) {

            Wire.beginTransmission(address);
            Wire.write(0x40);

            for (int i = 0; i < 16; i++) {
                Wire.write(buffer[page * OLED_WIDTH + col + i]);
            }

            Wire.endTransmission();
        }
    }

    dirty = false;
}

/* ================= PIXEL ================= */

void SSD1306Lite::setPixel(int x, int y, DrawMode mode) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;

    int index = x + (y / 8) * OLED_WIDTH;
    uint8_t mask = 1 << (y % 8);

    if (mode == PIXEL_ON)
        buffer[index] |= mask;
    else if (mode == PIXEL_OFF)
        buffer[index] &= ~mask;
    else
        buffer[index] ^= mask;

    dirty = true;
}

/* ================= LINE ================= */

void SSD1306Lite::drawLine(int x0,int y0,int x1,int y1,int thickness,DrawMode mode){
    int dx=abs(x1-x0),sx=x0<x1?1:-1;
    int dy=-abs(y1-y0),sy=y0<y1?1:-1;
    int err=dx+dy;

    while(true){
        for(int t=-thickness/2;t<=thickness/2;t++)
            setPixel(x0,y0+t,mode);

        if(x0==x1 && y0==y1) break;

        int e2=2*err;
        if(e2>=dy){err+=dy;x0+=sx;}
        if(e2<=dx){err+=dx;y0+=sy;}
    }
}

/* ================= RECT ================= */

void SSD1306Lite::drawRect(int x,int y,int w,int h,bool filled,int thickness,DrawMode mode){
    if(filled){
        for(int i=x;i<x+w;i++)
            for(int j=y;j<y+h;j++)
                setPixel(i,j,mode);
    } else {
        for(int t=0;t<thickness;t++){
            drawLine(x,y+t,x+w,y+t,1,mode);
            drawLine(x,y+h-t,x+w,y+h-t,1,mode);
            drawLine(x+t,y,x+t,y+h,1,mode);
            drawLine(x+w-t,y,x+w-t,y+h,1,mode);
        }
    }
}

/* ================= CIRCLE ================= */

void SSD1306Lite::drawCircle(int xc,int yc,int r,bool filled,int thickness,DrawMode mode){
    if (filled) {
        for (int y = -r; y <= r; y++) {
            for (int x = -r; x <= r; x++) {
                if (x*x + y*y <= r*r) {
                    setPixel(xc + x, yc + y, mode);
                }
            }
        }
        return;
    }

    // Hollow with thickness
    for (int t = 0; t < thickness; t++) {
        int rr = r - t;
        int x = 0, y = rr;
        int d = 3 - 2 * rr;

        while (y >= x) {
            setPixel(xc + x, yc + y, mode);
            setPixel(xc - x, yc + y, mode);
            setPixel(xc + x, yc - y, mode);
            setPixel(xc - x, yc - y, mode);
            setPixel(xc + y, yc + x, mode);
            setPixel(xc - y, yc + x, mode);
            setPixel(xc + y, yc - x, mode);
            setPixel(xc - y, yc - x, mode);

            x++;
            if (d > 0) {
                y--;
                d += 4 * (x - y) + 10;
            } else {
                d += 4 * x + 6;
            }
        }
    }
}

/* ================= ARC ================= */

void SSD1306Lite::drawArc(int xc,int yc,int r,int startDeg,int endDeg,int thickness,DrawMode mode){
    if(startDeg > endDeg){
        int tmp = startDeg;
        startDeg = endDeg;
        endDeg = tmp;
    }

    for(int t = 0; t < thickness; t++){
        int rr = r - t;

        int x = 0;
        int y = rr;
        int d = 3 - 2 * rr;

        while(y >= x){

            // check 8 symmetric points
            int px[8] = {
                xc + x, xc - x, xc + x, xc - x,
                xc + y, xc - y, xc + y, xc - y
            };

            int py[8] = {
                yc + y, yc + y, yc - y, yc - y,
                yc + x, yc + x, yc - x, yc - x
            };

            for(int i = 0; i < 8; i++){
                int dx = px[i] - xc;
                int dy = py[i] - yc;

                float angle = atan2(dy, dx) * 180.0 / PI;
                if(angle < 0) angle += 360;

                if(angle >= startDeg && angle <= endDeg){
                    setPixel(px[i], py[i], mode);
                }
            }

            x++;
            if(d > 0){
                y--;
                d += 4 * (x - y) + 10;
            } else {
                d += 4 * x + 6;
            }
        }
    }
}

void SSD1306Lite::drawArcFast(int xc,int yc,int r,int startDeg,int endDeg,int thickness,DrawMode mode){

    if(startDeg > endDeg){
        int tmp = startDeg;
        startDeg = endDeg;
        endDeg = tmp;
    }

    // convert degrees → octant mask (8 bits)
    uint8_t mask = 0;

    for(int d = startDeg; d <= endDeg; d += 45){
        mask |= (1 << ((d / 45) % 8));
    }

    for(int t = 0; t < thickness; t++){
        int rr = r - t;

        int x = 0;
        int y = rr;
        int d = 3 - 2 * rr;

        while(y >= x){

            // 8 symmetric points + octant index
            if(mask & (1<<0)) setPixel(xc + x, yc + y, mode);
            if(mask & (1<<1)) setPixel(xc + y, yc + x, mode);
            if(mask & (1<<2)) setPixel(xc + y, yc - x, mode);
            if(mask & (1<<3)) setPixel(xc + x, yc - y, mode);
            if(mask & (1<<4)) setPixel(xc - x, yc - y, mode);
            if(mask & (1<<5)) setPixel(xc - y, yc - x, mode);
            if(mask & (1<<6)) setPixel(xc - y, yc + x, mode);
            if(mask & (1<<7)) setPixel(xc - x, yc + y, mode);

            x++;
            if(d > 0){
                y--;
                d += 4 * (x - y) + 10;
            } else {
                d += 4 * x + 6;
            }
        }
    }
}

/* ================= TRIANGLE ================= */

void SSD1306Lite::drawTriangle(int x1,int y1,int x2,int y2,int x3,int y3,bool filled,int thickness,DrawMode mode){

    if (!filled) {
        for (int t = 0; t < thickness; t++) {
            drawLine(x1,y1+t,x2,y2+t,1,mode);
            drawLine(x2,y2+t,x3,y3+t,1,mode);
            drawLine(x3,y3+t,x1,y1+t,1,mode);
        }
        return;
    }

    int minY = min(y1, min(y2,y3));
    int maxY = max(y1, max(y2,y3));

    for(int y = minY; y <= maxY; y++) {
        int nodes = 0;
        int nodeX[3];

        int x[3] = {x1,x2,x3};
        int yy[3] = {y1,y2,y3};

        for(int i=0,j=2;i<3;j=i++) {
            if ((yy[i]<y && yy[j]>=y)||(yy[j]<y && yy[i]>=y)) {
                nodeX[nodes++] = x[i] + (y-yy[i])*(x[j]-x[i])/(yy[j]-yy[i]);
            }
        }

        if(nodes==2) {
            if(nodeX[0]>nodeX[1]) {
                int temp=nodeX[0];
                nodeX[0]=nodeX[1];
                nodeX[1]=temp;
            }

            for(int i=nodeX[0]; i<=nodeX[1]; i++)
                setPixel(i,y,mode);
        }
    }
}

/* ================= BITMAP ================= */

void SSD1306Lite::drawBitmap(int x,int y,const uint8_t* bmp,int w,int h,int scale,DrawMode mode){
    for(int j = 0; j < h; j++){
        for(int i = 0; i < w; i++){

            // since your bitmap is 1 byte per pixel (0 or 1)
            if(bmp[j * w + i]){

                for(int sy = 0; sy < scale; sy++){
                    for(int sx = 0; sx < scale; sx++){
                        setPixel(
                            x + i * scale + sx,
                            y + j * scale + sy,
                            mode
                        );
                    }
                }

            }
        }
    }
}

/* ================= TEXT (MINIMAL) ================= */

void SSD1306Lite::drawChar(int x,int y,char c,int scale,DrawMode mode){
    if(c < 32 || c > 127) return;

    const uint8_t* ch = font5x7[c - 32];

    for(int col = 0; col < 5; col++){
        uint8_t line = ch[col];

        for(int row = 0; row < 7; row++){
            if(line & (1 << row)){
                for(int sx = 0; sx < scale; sx++){
                    for(int sy = 0; sy < scale; sy++){
                        setPixel(
                            x + col * scale + sx,
                            y + row * scale + sy,
                            mode
                        );
                    }
                }
            }
        }
    }
}

void SSD1306Lite::drawText(int x,int y,const char* str,int scale,DrawMode mode){
    int cursorX = x;

    while(*str){
        drawChar(cursorX, y, *str, scale, mode);
        cursorX += (6 * scale); // 5px glyph + 1px spacing
        str++;
    }
}

bool SSD1306Lite::isDirty() const {
    return dirty;
}

void SSD1306Lite::markDirty() {
    dirty = true;
}

void SSD1306Lite::drawTextBox(int x,int y,int w,const char* str,int scale,DrawMode mode){
    int cursorX = x;
    int cursorY = y;

    int charWidth = 6 * scale;
    int charHeight = 8 * scale;

    const char* wordStart = str;

    while (*str) {

        // Handle newline
        if (*str == '\n') {
            cursorX = x;
            cursorY += charHeight;
            str++;
            wordStart = str;
            continue;
        }

        // Find word length
        int wordLen = 0;
        while (str[wordLen] != ' ' && str[wordLen] != '\0' && str[wordLen] != '\n') {
            wordLen++;
        }

        int wordPixelWidth = wordLen * charWidth;

        // Wrap if word doesn't fit
        if (cursorX + wordPixelWidth > x + w) {
            cursorX = x;
            cursorY += charHeight;
        }

        // Draw word
        for (int i = 0; i < wordLen; i++) {
            drawChar(cursorX, cursorY, str[i], scale, mode);
            cursorX += charWidth;
        }

        str += wordLen;

        // Handle space
        if (*str == ' ') {
            cursorX += charWidth;
            str++;
        }
    }
}