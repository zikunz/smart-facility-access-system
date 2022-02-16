/* These functions are based on the Arduino test program at
 *  https://github.com/adafruit/Adafruit-SSD1351-library/blob/master/examples/test/test.ino
 *
 *  You can use these high-level routines to implement your
 *  test program.
 */

// Configure SPI port and use these libraries to implement
// an OLED test program.
#include "test.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "string.h"
#include "stdio.h"

float p = 3.1415926;

//*****************************************************************************
//  function delays 3*ulCount cycles
void delay(unsigned long ulCount)
{
    int i;

    do
    {
        ulCount--;
        for (i = 0; i < 65535; i++)
            ;
    }
    while (ulCount);
}

//*****************************************************************************

void testFullChar()
{
    int x = 0;
    int y = 0;
    int i;

    for (i = 0; i < 1275; i++)
    {
        if (x == 0 && y == 0)
        {
            fillScreen(BLACK);
        }
        drawChar(x, y, font[i], RED, RED, 1);
        x = x + 8;
        if (x >= 128)
        {
            x = 0;
            y = y + 10;
        }
        if (y >= 128)
        {
            y = 0;
        }
    }
}

//*****************************************************************************

void testHelloworld()
{
    drawChar(0, 64, 'H', BLUE, BLUE, 2);
    drawChar(10, 64, 'e', BLUE, BLUE, 2);
    drawChar(20, 64, 'l', BLUE, BLUE, 2);
    drawChar(30, 64, 'l', BLUE, BLUE, 2);
    drawChar(40, 64, 'o', BLUE, BLUE, 2);
    drawChar(60, 64, 'w', BLUE, BLUE, 2);
    drawChar(70, 64, 'o', BLUE, BLUE, 2);
    drawChar(80, 64, 'r', BLUE, BLUE, 2);
    drawChar(90, 64, 'l', BLUE, BLUE, 2);
    drawChar(100, 64, 'd', BLUE, BLUE, 2);
    drawChar(110, 64, '!', BLUE, BLUE, 2);
}

//*****************************************************************************

void testHorizontal()
{
    fillRect(0, 0, 16, 128, BLUE);
    fillRect(16, 0, 16, 128, BLUE + 50);
    fillRect(32, 0, 16, 128, RED);
    fillRect(48, 0, 16, 128, CYAN);
    fillRect(64, 0, 16, 128, GREEN);
    fillRect(80, 0, 16, 128, YELLOW);
    fillRect(96, 0, 16, 128, WHITE);
    fillRect(112, 0, 16, 128, MAGENTA);
}

//*****************************************************************************

void testVertical()
{
    fillRect(0, 0, 128, 16, BLUE);
    fillRect(0, 16, 128, 16, BLUE + 50);
    fillRect(0, 32, 128, 16, RED);
    fillRect(0, 48, 128, 16, CYAN);
    fillRect(0, 64, 128, 16, GREEN);
    fillRect(0, 80, 128, 16, YELLOW);
    fillRect(0, 96, 128, 16, WHITE);
    fillRect(0, 112, 128, 16, MAGENTA);
}

//*****************************************************************************

void testfastlines(unsigned int color1, unsigned int color2)
{
    unsigned int x;
    unsigned int y;

    fillScreen(BLACK);
    for (y = 0; y < height() - 1; y += 8)
    {
        drawFastHLine(0, y, width() - 1, color1);
    }
    delay(100);
    for (x = 0; x < width() - 1; x += 8)
    {
        drawFastVLine(x, 0, height() - 1, color2);
    }
    delay(100);
}

//*****************************************************************************

void testdrawrects(unsigned int color)
{
    unsigned int x;

    fillScreen(BLACK);
    for (x = 0; x < height() - 1; x += 6)
    {
        drawRect((width() - 1) / 2 - x / 2, (height() - 1) / 2 - x / 2, x, x,
                 color);
        delay(10);
    }
}

//*****************************************************************************

void testfillrects(unsigned int color1, unsigned int color2)
{

    unsigned char x;

    fillScreen(BLACK);
    for (x = height() - 1; x > 6; x -= 6)
    {
        fillRect((width() - 1) / 2 - x / 2, (height() - 1) / 2 - x / 2, x, x,
                 color1);
        drawRect((width() - 1) / 2 - x / 2, (height() - 1) / 2 - x / 2, x, x,
                 color2);
        delay(10);
    }
}

//*****************************************************************************

void testfillcircles(unsigned char radius, unsigned int color)
{
    unsigned char x;
    unsigned char y;

    for (x = radius; x < width() - 1; x += radius * 2)
    {
        for (y = radius; y < height() - 1; y += radius * 2)
        {
            fillCircle(x, y, radius, color);
            delay(10);
        }
    }
}

//*****************************************************************************

void testdrawcircles(unsigned char radius, unsigned int color)
{
    unsigned char x;
    unsigned char y;

    for (x = 0; x < width() - 1 + radius; x += radius * 2)
    {
        for (y = 0; y < height() - 1 + radius; y += radius * 2)
        {
            drawCircle(x, y, radius, color);
            delay(10);
        }
    }
}

//*****************************************************************************

void testtriangles()
{
    int color = 0xF800;
    int t;
    int w = width() / 2;
    int x = height() - 1;
    int y = 0;
    int z = width() - 1;

    fillScreen(BLACK);
    for (t = 0; t <= 15; t += 1)
    {
        drawTriangle(w, y, y, x, z, x, color);
        x -= 4;
        y += 4;
        z -= 4;
        color += 100;
        delay(10);
    }
}

//*****************************************************************************

void testroundrects()
{
    int color = 100;

    int i;
    int x = 0;
    int y = 0;
    int w = width();
    int h = height();

    fillScreen(BLACK);

    for (i = 0; i <= 24; i++)
    {
        drawRoundRect(x, y, w, h, 5, color);
        x += 2;
        y += 3;
        w -= 4;
        h -= 6;
        color += 1100;
    }
}

//*****************************************************************************
void testlines(unsigned int color)
{
    unsigned int x;
    unsigned int y;

    fillScreen(BLACK);
    for (x = 0; x < width() - 1; x += 6)
    {
        drawLine(0, 0, x, height() - 1, color);
    }
    delay(10);
    for (y = 0; y < height() - 1; y += 6)
    {
        drawLine(0, 0, width() - 1, y, color);
    }
    delay(100);

    fillScreen(BLACK);
    for (x = 0; x < width() - 1; x += 6)
    {
        drawLine(width() - 1, 0, x, height() - 1, color);
    }
    delay(100);
    for (y = 0; y < height() - 1; y += 6)
    {
        drawLine(width() - 1, 0, 0, y, color);
    }
    delay(100);

    fillScreen(BLACK);
    for (x = 0; x < width() - 1; x += 6)
    {
        drawLine(0, height() - 1, x, 0, color);
    }
    delay(100);
    for (y = 0; y < height() - 1; y += 6)
    {
        drawLine(0, height() - 1, width() - 1, y, color);
    }
    delay(100);

    fillScreen(BLACK);
    for (x = 0; x < width() - 1; x += 6)
    {
        drawLine(width() - 1, height() - 1, x, 0, color);
    }
    delay(100);
    for (y = 0; y < height() - 1; y += 6)
    {
        drawLine(width() - 1, height() - 1, 0, y, color);
    }
    delay(100);

}

//*****************************************************************************

void lcdTestPattern(void)
{
    unsigned int i, j;
    goTo(0, 0);

    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 128; j++)
        {
            if (i < 16)
            {
                writeData(RED >> 8);
                writeData((unsigned char) RED);
            }
            else if (i < 32)
            {
                writeData(YELLOW >> 8);
                writeData((unsigned char) YELLOW);
            }
            else if (i < 48)
            {
                writeData(GREEN >> 8);
                writeData((unsigned char) GREEN);
            }
            else if (i < 64)
            {
                writeData(CYAN >> 8);
                writeData((unsigned char) CYAN);
            }
            else if (i < 80)
            {
                writeData(BLUE >> 8);
                writeData((unsigned char) BLUE);
            }
            else if (i < 96)
            {
                writeData(MAGENTA >> 8);
                writeData((unsigned char) MAGENTA);
            }
            else if (i < 112)
            {
                writeData(BLACK >> 8);
                writeData((unsigned char) BLACK);
            }
            else
            {
                writeData(WHITE >> 8);
                writeData((unsigned char) WHITE);
            }
        }
    }
}
/**************************************************************************/
void lcdTestPattern2(void)
{
    unsigned int i, j;
    goTo(0, 0);

    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 128; j++)
        {
            if (j < 16)
            {
                writeData(RED >> 8);
                writeData((unsigned char) RED);
            }
            else if (j < 32)
            {
                writeData(YELLOW >> 8);
                writeData((unsigned char) YELLOW);
            }
            else if (j < 48)
            {
                writeData(GREEN >> 8);
                writeData((unsigned char) GREEN);
            }
            else if (j < 64)
            {
                writeData(CYAN >> 8);
                writeData((unsigned char) CYAN);
            }
            else if (j < 80)
            {
                writeData(BLUE >> 8);
                writeData((unsigned char) BLUE);
            }
            else if (j < 96)
            {
                writeData(MAGENTA >> 8);
                writeData((unsigned char) MAGENTA);
            }
            else if (j < 112)
            {
                writeData(BLACK >> 8);
                writeData((unsigned char) BLACK);
            }
            else
            {
                writeData(WHITE >> 8);
                writeData((unsigned char) WHITE);
            }
        }
    }
}

/**************************************************************************/

