#include "TFTLCD.h"

#ifdef USE_ADAFRUIT_SHIELD_PINOUT
// special defines for the dataport
 #define DATAPORT1 PORTD
 #define DATAPIN1 PIND
 #define DATADDR1 DDRD

 #define DATAPORT2 PORTB
 #define DATAPIN2 PINB
 #define DATADDR2 DDRB

 #define DATA1_MASK 0xD0
 #define DATA2_MASK 0x2F

// for mega & shield usage, we just hardcoded it (its messy)

#else
 // for the breakout board tutorial, two ports are used :/
 #define DATAPORT1 PORTD
 #define DATAPIN1  PIND
 #define DATADDR1  DDRD

 #define DATAPORT2 PORTB
 #define DATAPIN2  PINB
 #define DATADDR2  DDRB

 #define DATA1_MASK 0xFC  // top 6 bits
 #define DATA2_MASK 0x03  // bottom 2 bits


 #define MEGA_DATAPORT PORTA
 #define MEGA_DATAPIN  PINA
 #define MEGA_DATADDR  DDRA
#endif


#include "glcdfont.c"
#include <avr/pgmspace.h>
#include "pins_arduino.h"
#include "wiring_private.h"

// Mike McCauley:
// Manage variations of GRAM arrangement.
// The inexpensive 2.4" touchscreens from Ebay made by http://www.mcufriend.com
// have unusual addressing so added this:
// Some instances of this device have reversed X or Y coordinates
// If your LCD display chip has inverted X addresses define this:
//#define INVERT_X
// If your LCD display chip has inverted Y addresses define this:
//#define INVERT_Y

#ifdef INVERT_X
#define X(x) (TFTWIDTH - x - 1)
#define I_X(x) (x)
#else
#define X(x) (x)
#define I_X(x) (TFTWIDTH - x - 1)
#endif
#ifdef INVERT_Y
#define Y(y) (TFTHEIGHT - y - 1)
#define I_Y(y) (y)
#else
#define Y(y) (y)
#define I_Y(y) (TFTHEIGHT - y - 1)
#endif


void TFTLCD::goHome(void) {
  goTo(0,0);
}

uint16_t TFTLCD::width(void) {
  return _width;
}
uint16_t TFTLCD::height(void) {
  return _height;
}



void TFTLCD::setCursor(uint16_t x, uint16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void TFTLCD::setTextSize(uint8_t s) {
  textsize = s;
}

void TFTLCD::setTextColor(uint16_t c) {
  textcolor = c;
}

size_t TFTLCD::write(uint8_t c) {
  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textsize);
    cursor_x += textsize*6;
  }
  return 0;
}

void TFTLCD::drawString(uint16_t x, uint16_t y, char *c, 
			uint16_t color, uint8_t size) {
  while (c[0] != 0) {
    drawChar(x, y, c[0], color, size);
    x += size*6;
    c++;
  }
}
// draw a character
void TFTLCD::drawChar(uint16_t x, uint16_t y, char c, 
		      uint16_t color, uint8_t size) {
  for (uint8_t i =0; i<5; i++ ) {
    uint8_t line = pgm_read_byte(font+(c*5)+i);
    for (uint8_t j = 0; j<8; j++) {
      if (line & 0x1) {
	if (size == 1) // default size
	  drawPixel(x+i, y+j, color);
	else {  // big size
	  fillRect(x+i*size, y+j*size, size, size, color);
	} 
      }
      line >>= 1;
    }
  }
}

// draw a triangle!
void TFTLCD::drawTriangle(uint16_t x0, uint16_t y0,
			  uint16_t x1, uint16_t y1,
			  uint16_t x2, uint16_t y2, uint16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color); 
}

void TFTLCD::fillTriangle ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t color)
{
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  int32_t dx1, dx2, dx3; // Interpolation deltas
  int32_t sx1, sx2, sy; // Scanline co-ordinates

  sx2=(int32_t)x0 * (int32_t)1000; // Use fixed point math for x axis values
  sx1 = sx2;
  sy=y0;

  // Calculate interpolation deltas
  if (y1-y0 > 0) dx1=((x1-x0)*1000)/(y1-y0);
    else dx1=0;
  if (y2-y0 > 0) dx2=((x2-x0)*1000)/(y2-y0);
    else dx2=0;
  if (y2-y1 > 0) dx3=((x2-x1)*1000)/(y2-y1);
    else dx3=0;

  // Render scanlines (horizontal lines are the fastest rendering method)
  if (dx1 > dx2)
  {
    for(; sy<=y1; sy++, sx1+=dx2, sx2+=dx1)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx2 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx2, sx2+=dx3)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
  else
  {
    for(; sy<=y1; sy++, sx1+=dx1, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx1 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx3, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
}

uint16_t TFTLCD::Color565(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;

  return c;
}

// draw a rectangle
void TFTLCD::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
		      uint16_t color) {
  // smarter version
  drawHorizontalLine(x, y, w, color);
  drawHorizontalLine(x, y+h-1, w, color);
  drawVerticalLine(x, y, h, color);
  drawVerticalLine(x+w-1, y, h, color);
}

// draw a rounded rectangle
void TFTLCD::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color) {
  // smarter version
  drawHorizontalLine(x+r, y, w-2*r, color);
  drawHorizontalLine(x+r, y+h-1, w-2*r, color);
  drawVerticalLine(x, y+r, h-2*r, color);
  drawVerticalLine(x+w-1, y+r, h-2*r, color);
  // draw four corners
  drawCircleHelper(x+r, y+r, r, 1, color);
  drawCircleHelper(x+w-r-1, y+r, r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r, y+h-r-1, r, 8, color);
}


// fill a rounded rectangle
void TFTLCD::fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r, y+r, r, 2, h-2*r-1, color);
}

// fill a circle
void TFTLCD::fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
  writeRegister(TFTLCD_WRITE_DIR, 0xC8);
  drawVerticalLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}


// used to do circles and roundrects!
void TFTLCD::fillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername, uint16_t delta,
			uint16_t color) {

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    if (cornername & 0x1) {
      drawVerticalLine(x0+x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawVerticalLine(x0-x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}


// draw a circle outline

void TFTLCD::drawCircle(uint16_t x0, uint16_t y0, uint16_t r, 
			uint16_t color) {
  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  drawCircleHelper(x0, y0, r, 0xF, color);
}

void TFTLCD::drawCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername,
			uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;


  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    } 
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

// fill a rectangle
void TFTLCD::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
		      uint16_t fillcolor) {
  // smarter version
  while (h--)
    drawHorizontalLine(x, y++, w, fillcolor);
}


void TFTLCD::drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color)
{
  if (x >= _width) return;

  drawFastLine(x,y,length,color,1);
}

void TFTLCD::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color)
{
  if (y >= _height) return;
  drawFastLine(x,y,length,color,0);
}

void TFTLCD::drawFastLine(uint16_t x, uint16_t y, uint16_t length, 
			  uint16_t color, uint8_t rotflag)
{
  if (rotflag) {
    setArea(x, y, 1, length);
  } else {
    setArea(x, y, length, 1);
  }
  writeCommand(0x22);

  *portOutputRegister(csport) &= ~cspin;
  //digitalWrite(_cs, LOW);
  *portOutputRegister(cdport) |= cdpin;
  //digitalWrite(_cd, HIGH);
  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  *portOutputRegister(wrport) |= wrpin;
  //digitalWrite(_wr, HIGH);

  setWriteDir();
  while (length--) {
    writeData_unsafe(color); 
  }

  // set back to default
  *portOutputRegister(csport) |= cspin;
  //digitalWrite(_cs, HIGH);
}



// bresenham's algorithm - thx wikpedia
void TFTLCD::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
		      uint16_t color) {
  // if you're in rotation 1 or 3, we need to swap the X and Y's

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  //dy = abs(y1 - y0);
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void TFTLCD::fillScreen(uint16_t color) {
  setMaxArea();
  uint32_t screen_pixels = TFTLCD_BUFF_SIZE; //240x320

  writeCommand(0x22);

  *portOutputRegister(csport) &= ~cspin;
  //digitalWrite(_cs, LOW);
  *portOutputRegister(cdport) |= cdpin;
  //digitalWrite(_cd, HIGH);
  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  *portOutputRegister(wrport) |= wrpin;
  //digitalWrite(_wr, HIGH);

  setWriteDir();
  while (screen_pixels--) {
    writeData_unsafe(color); 
  }

  *portOutputRegister(csport) |= cspin;
  //digitalWrite(_cs, HIGH);
}

void TFTLCD::fastSetAddr(uint16_t x, uint16_t y) {
  writeRegister(0x02, x >> 8);
  writeRegister(0x03, x);

  writeRegister(0x06, y >> 8);
  writeRegister(0x07, y);
}

void TFTLCD::setArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

  writeRegister(0x02, x >> 8);
  writeRegister(0x03, x);

  writeRegister(0x04, x+w-1 >> 8);
  writeRegister(0x05, x+w-1);

  writeRegister(0x06, y >> 8);
  writeRegister(0x07, y);

  writeRegister(0x08, y+h-1 >> 8);
  writeRegister(0x09, y+h-1);

  // writeCommand(0x22);
}

void TFTLCD::setMaxArea() {
  setArea(0, 0, _width, _height);
}

void TFTLCD::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  // check rotation, move pixel around if necessary
    switch (rotation) {
	case 0:
	    x = X(x);
	    y = Y(y);
	    break;
	    
	case 1:
	    swap(x, y);
	    x = I_X(x);
	    y = Y(y);
	    break;
	case 2:
	    x = I_X(x);
	    y = I_Y(y);
	    break;
	case 3:
	    swap(x, y);
	    x = X(x);
	    y = I_Y(y);
	    break;
    }
    
  if ((x >= TFTWIDTH) || (y >= TFTHEIGHT)) return;
  writeCommand(0x02c); //write_memory_start
  setArea(x,y, 1, 1);
  writeCommand(0x22);
  writeData(color);
}

static const uint16_t _regValues[] PROGMEM = {
  // gamma settings
  /* 0x46,0x91, */
  /* 0x47,0x11, */
  /* 0x48,0x00, */
  /* 0x49,0x66, */

  /* 0x4a,0x37, */
  /* 0x4b,0x04, */
  /* 0x4c,0x11, */
  /* 0x4d,0x77, */

  /* 0x4e,0x00, */
  /* 0x4f,0x1f, */
  /* 0x50,0x0f, */
  /* 0x51,0x00, */

  //240x320 window setting
  0x02,0x00, // Column address start2
  0x03,0x00, // Column address start1
  0x04,0x00, // Column address end2
  0x05,0xef, // Column address end1
  0x06,0x00, // Row address start2
  0x07,0x00, // Row address start1
  0x08,0x01, // Row address end2
  0x09,0x3f, // Row address end1

    // Display Setting
  0x01,0x06, // IDMON=0, INVON=1, NORON=1, PTLON=0
  0x16,0xC8, // MY=1, MX=1, MV=0, ML=0, BGR=1 (p.131)
  0x17,0x05, // IFPF=101 - 16Bit/Pixel (p.132)
  0x18,0x49, /* I/PI_RADJ=0100 (idle mode - 60Mz)
                N/P_RADJ=1001 (normal mode - 80Hz) (p. 133) */ // CHECK IT!!

  0x23,0x95, // N_DC=1001 0101
  0x24,0x95, // PI_DC=1001 0101
  0x25,0xFF, // I_DC=1111 1111

  // I don't know what it this ;)
  /* 0x27,0x02, // N_BP=0000 0010 */
  /* 0x28,0x02, // N_FP=0000 0010 */
  /* 0x29,0x02, // PI_BP=0000 0010 */
  /* 0x2a,0x02, // PI_FP=0000 0010 */
  /* 0x2C,0x02, // I_BP=0000 0010 */
  /* 0x2d,0x02, // I_FP=0000 0010 */

  /* 0x31,0x01, */
  /* 0x3a,0x01, // N_RTN=0000, N_NW=001    0001 */
  /* 0x3b,0x00, // P_RTN=0000, P_NW=001 */
  /* 0x3c,0xf0, // I_RTN=1111, I_NW=000 */
  /* 0x3d,0x00, // DIV=00 */
  /* 0xFF,1, */
  /* 0x35,0x38, // EQS=38h */
  /* 0x36,0x78, // EQP=78h */
  /* 0x3E,0x38, // SON=38h */
  /* 0x40,0x0F, // GDON=0Fh */
  /* 0x41,0xF0, // GDOFF */

  //VCOM SETTING
  /* 0x44,0x4D, // VCM=101 0000  4D */  // \
  /* 0x45,0x11, // VDV=1 0001   0011 */ // / THIS IS GAMMA SETTINGS ??
  //  0xFF,1,

  /* =================================
     = Power on setting flow (p.108) =
     ================================= */

  // Power control regisers
  0x1A,0x02, // BT=0010 (p.134)
  0x1B,0x1A, // VRH=0011 (p.135)
  // VMF, VML and VMH ??

  // I don't known that it this...
  /* 0x1D,0x07, // VC1=111   0007 */
  /* 0x1E,0x00, // VC3=000 */

  0xFF, 5,
  0x19, 0x01, // OSC_EN=1 (p.133)
  0xFF, 5,


  0x1C, 0x03, // AP=011 // quality vs current consumption (p.136)
  0xFF, 5,
  // (p.138)
  0x1F, 0X88, // STB=0   -> GASEN=1, VCOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
  0xFF, 5,
  0x1F, 0X80, // DK=0    -> GASEN=1, VCOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
  0xFF, 5,
  0x1F, 0X90, // PON=1   -> GASEN=1, VCOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
  0xFF, 5,
  0x1F, 0XD4, // VCOMG=1 -> GASEN=1, VCOMG=10, PON=1, DK=0, XDK=1, DVDH_TRI=0, STB=0
  0xFF, 10,

  // Display on flow (p.105)
  /* 0x90,0x7F, // SAP=0111 1111 */ // ??????
  0x28,0x38, // GON=1, DTE=1, D=10
  0xFF,50,
  0x28,0x3C, // GON=1, DTE=1, D=11

  // I have no fucking idea WHY, but without this registers, display not alive :/
  0x26,0x3C, //GON=1, DTE=1, D=11
  0x21,0x00
};

void TFTLCD::initDisplay(void) {
  uint16_t a, d;

  reset();
  

    
  for (uint8_t i = 0; i < sizeof(_regValues) / 4; i++) {
    a = pgm_read_word(_regValues + i*2);
    d = pgm_read_word(_regValues + i*2 + 1);
     // a = pgm_read_word(&_regValues[i++]);
      //d = pgm_read_word(&_regValues[i++]);
    if (a == 0xFF) {
      delay(d);
    } else {
      writeRegister(a, d);
      //Serial.print("addr: "); Serial.print(a); 
      //Serial.print(" data: "); Serial.println(d, HEX);
    }
  }
  writeData(0x22);
}

uint8_t TFTLCD::getRotation(void) {
  return rotation;
}



/********************************* low level pin initialization */


TFTLCD::TFTLCD(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset) {
  _cs = cs;
  _cd = cd;
  _wr = wr;
  _rd = rd;
  _reset = reset;
  
  rotation = 0;
  _width = TFTWIDTH;
  _height = TFTHEIGHT;

  // disable the LCD
  digitalWrite(_cs, HIGH);
  pinMode(_cs, OUTPUT);  
  
  digitalWrite(_cd, HIGH);
  pinMode(_cd, OUTPUT);  
  
  digitalWrite(_wr, HIGH);
  pinMode(_wr, OUTPUT);  
  
  digitalWrite(_rd, HIGH);
  pinMode(_rd, OUTPUT);  

  digitalWrite(_reset, HIGH); 
  pinMode(_reset, OUTPUT); 

  csport = digitalPinToPort(_cs);
  cdport = digitalPinToPort(_cd);
  wrport = digitalPinToPort(_wr);
  rdport = digitalPinToPort(_rd);

  cspin = digitalPinToBitMask(_cs);
  cdpin = digitalPinToBitMask(_cd);
  wrpin = digitalPinToBitMask(_wr);
  rdpin = digitalPinToBitMask(_rd);

  cursor_y = cursor_x = 0;
  textsize = 1;
  textcolor = 0xFFFF;
}


/********************************** low level pin interface */

// reset flow (power on/off setting up flow -  p. 108)
void TFTLCD::reset(void) {
  digitalWrite(_reset, LOW);
  delay(2); 
  digitalWrite(_reset, HIGH);

  // resync
  writeData(0);
  writeData(0);
  writeData(0);  
  writeData(0);
}

inline void TFTLCD::setWriteDir(void) {
  DATADDR2 |= DATA2_MASK;
  DATADDR1 |= DATA1_MASK;
}

inline void TFTLCD::setReadDir(void) {
  DATADDR2 &= ~DATA2_MASK;
  DATADDR1 &= ~DATA1_MASK;
}

inline void TFTLCD::write8(uint8_t d) {
  DATAPORT2 = (DATAPORT2 & DATA1_MASK) | 
    (d & DATA2_MASK);
  DATAPORT1 = (DATAPORT1 & DATA2_MASK) | 
    (d & DATA1_MASK); // top 6 bits
}

inline uint8_t TFTLCD::read8(void) {
 uint8_t d;
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328) || (__AVR_ATmega8__)

 d = DATAPIN1 & DATA1_MASK; 
 d |= DATAPIN2 & DATA2_MASK; 

#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__)  || defined(__AVR_ATmega1280__) 

#ifdef USE_ADAFRUIT_SHIELD_PINOUT

  // bit 6/7 (PH3 & 4)
  // first two bits 0 & 1 (PH5 & 6)
 d = (PINH & 0x60) >> 5;
 d |= (PINH & 0x18) << 3;

  // bits 2 & 3 & 5 (PB4 & PB5, PB7)
 d |= (PINB & 0xB0) >> 2;

  // bit 4  (PG5)
  if (PING & _BV(5))
    d |= _BV(4);

#else
 d = MEGA_DATAPIN;  
#endif

#else

  #error "No pins defined!"

#endif

 return d;
}

/********************************** low level readwrite interface */

// the C/D pin is high during write
void TFTLCD::writeData(uint16_t data) {
  volatile uint8_t *wrportreg = portOutputRegister(wrport);

  *portOutputRegister(csport) &= ~cspin;
  //digitalWrite(_cs, LOW);
  *portOutputRegister(cdport) |= cdpin;
  //digitalWrite(_cd, HIGH);
  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  setWriteDir();
  write8(data >> 8);
  
  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  write8(data);

  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  *portOutputRegister(csport) |= cspin;
  //digitalWrite(_cs, HIGH);
}


// this is a 'sped up' version, with no direction setting, or pin initialization
// not for external usage, but it does speed up stuff like a screen fill
inline void TFTLCD::writeData_unsafe(uint16_t data) {
  volatile uint8_t *wrportreg = portOutputRegister(wrport);

  write8(data >> 8);

  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  write8(data);

  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);
}

// the C/D pin is low during write
void TFTLCD::writeCommand(uint16_t cmd) {
  volatile uint8_t *wrportreg = portOutputRegister(wrport);

  *portOutputRegister(csport) &= ~cspin;
  //digitalWrite(_cs, LOW);
  *portOutputRegister(cdport) &= ~cdpin;
  //digitalWrite(_cd, LOW);
  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  setWriteDir();
  write8(cmd >> 8);

  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  write8(cmd);

  *wrportreg &= ~wrpin;
  //digitalWrite(_wr, LOW);
  *wrportreg |=  wrpin;
  //digitalWrite(_wr, HIGH);

  *portOutputRegister(csport) |= cspin;
}

uint16_t TFTLCD::readData() {
 uint16_t d = 0;
 
  *portOutputRegister(csport) &= ~cspin;
  //digitalWrite(_cs, LOW);
  *portOutputRegister(cdport) |= cdpin;
  //digitalWrite(_cd, HIGH);
  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  *portOutputRegister(wrport) |= wrpin;
  //digitalWrite(_wr, HIGH);
  
  setReadDir();

  *portOutputRegister(rdport) &= ~rdpin;
  //digitalWrite(_rd, LOW);

  delayMicroseconds(10);
  d = read8();
  d <<= 8;

  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  *portOutputRegister(rdport) &= ~rdpin;
  //digitalWrite(_rd, LOW);

  delayMicroseconds(10);
  d |= read8();

  *portOutputRegister(rdport) |= rdpin;
  //digitalWrite(_rd, HIGH);
  
  *portOutputRegister(csport) |= cspin;
  //digitalWrite(_cs, HIGH);
   
  return d;
}


/************************************* medium level data reading/writing */

uint16_t TFTLCD::readRegister(uint16_t addr) {
   writeCommand(addr);
   return readData();
}

void TFTLCD::writeRegister(uint16_t addr, uint16_t data) {
   writeCommand(addr);
   writeData(data);
}


void TFTLCD::goTo(uint16_t x, uint16_t y) {
  	calcGRAMPosition(&x, &y);
	// writeRegister(0x0020, x);     // GRAM Address Set (Horizontal Address) (R20h)
  	// writeRegister(0x0021, y);     // GRAM Address Set (Vertical Address) (R21h)
  	// writeCommand(0x0022);            // Write Data to GRAM (R22h)
        fastSetAddr(x,y);
        writeCommand(0x22);

}

void TFTLCD::setDefaultViewport()
{
	writeRegister(TFTLCD_HOR_START_AD, 0);    
	writeRegister(TFTLCD_HOR_END_AD, TFTWIDTH - 1);    
	writeRegister(TFTLCD_VER_START_AD, 0);     
	writeRegister(TFTLCD_VER_END_AD, TFTHEIGHT -1);     		
}

void TFTLCD::getViewport(uint16_t *bx, uint16_t *by, uint16_t *ex, uint16_t *ey)
{
	*bx = readRegister(TFTLCD_HOR_START_AD);    
	*ex = readRegister(TFTLCD_HOR_END_AD);    
	*by = readRegister(TFTLCD_VER_START_AD);     
	*ey = readRegister(TFTLCD_VER_END_AD);     
}

void TFTLCD::setViewport(uint16_t bx, uint16_t by, uint16_t ex, uint16_t ey)
{
	calcGRAMPosition(&bx, &by);
	calcGRAMPosition(&ex, &ey);
	
	// Fix coordinates to be in order
	if( ey < by )
		swap(ey, by);
	if( ex < bx )
		swap(ex, bx);
	
	writeRegister(TFTLCD_HOR_START_AD, bx);    
	writeRegister(TFTLCD_HOR_END_AD, ex);    
	writeRegister(TFTLCD_VER_START_AD, by);     
	writeRegister(TFTLCD_VER_END_AD, ey); 	
}

// Writes 16-bit data in bulk, using callback to get more
void TFTLCD::bulkWrite(uint16_t *data, uint16_t bufferSize, uint16_t (*getNextValues)(void *), void *userData)
{
	*portOutputRegister(csport) &= ~cspin;
	*portOutputRegister(cdport) |= cdpin;
	*portOutputRegister(rdport) |= rdpin;
	*portOutputRegister(wrport) |= wrpin;

	setWriteDir();
	while( bufferSize )
	{
		for(uint16_t i=0; i < bufferSize; i++)
		{
			writeData_unsafe(data[i]);
		}
		bufferSize = getNextValues(userData);
	}
	*portOutputRegister(csport) |= cspin;		
}

 
 
void TFTLCD::calcGRAMPosition(uint16_t *posx, uint16_t *posy)
{
  uint16_t x = *posx;
  uint16_t y = *posy;
  switch( rotation )
  {
      case 0:
	  x = X(x);
	  y = Y(y);
	  break;
      case 1:  // 90
	  swap(x, y);
	  x = I_X(x);
	  y = Y(y);
	  break;
      case 2:  // 180
	  x = I_X(x);
	  y = I_Y(y);
	  break;
      case 3: // 270
	  swap(x, y);
	  y = I_Y(y);
	  break;
  }
  *posx = x;
  *posy = y;
}



void TFTLCD::setRotation(uint8_t x) {

  x %= 4;  // cant be higher than 3
  rotation = x;
  switch (x) {
  case 0:
    writeRegister(TFTLCD_WRITE_DIR, 0x08); // MY=0; MX=0; MV=0 (ML=0, BGR=1)
    _width = TFTWIDTH;
    _height = TFTHEIGHT;
    break;
  case 1:
    writeRegister(TFTLCD_WRITE_DIR, 0x68); // MY=0; MX=0; MV=0 (ML=0, BGR=1)
    _width = TFTHEIGHT;
    _height = TFTWIDTH;
    break;
  case 2:
    writeRegister(TFTLCD_WRITE_DIR, 0xC8); // MY=0; MX=0; MV=0 (ML=0, BGR=1)
    _width = TFTWIDTH;
    _height = TFTHEIGHT;
    break;
  case 3:
    writeRegister(TFTLCD_WRITE_DIR, 0xA8); // MY=0; MX=0; MV=0 (ML=0, BGR=1)
    _width = TFTHEIGHT;
    _height = TFTWIDTH;
    break;
  }
  setDefaultViewport();
}
