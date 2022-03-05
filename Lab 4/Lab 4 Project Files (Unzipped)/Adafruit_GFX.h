#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H

#define WIDTH 128
#define HEIGHT 128  // SET THIS TO 96 FOR 1.27"!

#define swap(a, b) {int t = a; a = b; b = t; }

void drawPixel(int x, int y, unsigned int color);

void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
void drawRect(int x, int y, int w, int h, unsigned int color);
void invertDisplay(char i);

void drawCircle(int x0, int y0, int r, unsigned int color);
void drawCircleHelper(int x0, int y0, int r, unsigned char cornername,
                      unsigned int color);
void fillCircle(int x0, int y0, int r, unsigned int color);
void fillCircleHelper(int x0, int y0, int r, unsigned char cornername,
                      int delta, unsigned int color);
void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                  unsigned int color);
void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                  unsigned int color);
void drawRoundRect(int x0, int y0, int w, int h, int radius,
                   unsigned int color);
void fillRoundRect(int x0, int y0, int w, int h, int radius,
                   unsigned int color);
void drawBitmap(int x, int y, const unsigned char *bitmap, int w, int h,
                unsigned int color);
void drawXBitmap(int x, int y, const unsigned char *bitmap, int w, int h,
                 unsigned int color);
void drawChar(int x, int y, unsigned char c, unsigned int color,
              unsigned int bg, unsigned char size);
void setCursor(int x, int y);
void setTextColor(unsigned int c, unsigned int bg);
void setTextSize(unsigned char s);
void setTextWrap(char w);

#if ARDUINO >= 100
virtual size_t write(unsigned char);
#else
void write(unsigned char);
#endif

int height(void);
int width(void);

unsigned char getRotation(void);
#endif // _ADAFRUIT_GFX_H
