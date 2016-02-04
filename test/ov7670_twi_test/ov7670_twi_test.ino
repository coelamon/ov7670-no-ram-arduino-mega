/* OV7670
 * Arduino Mega 2560
 *
 * open serial monitor (9600 baud)
 * maybe you'll need to press the reset button on arduino
 *
 * connections:
 * OV7670 XCLK => (through level shift!) => pin 46 = PL3(OC5A), output
 * OV7670 SIOC => (through level shift!) => pin 21 SCL
 * OV7670 SIOD => (through level shift!) => pin 20 SDA
 * OV7670 3V3   => 3.3V
 * OV7670 RESET => 3.3V
 * OV7670 GND   => GND
 * OV7670 PWDN  => GND
 */

#include "ov7670.h"

struct reg_name_t{
  uint8_t reg_num;
  char* reg_name;
};

reg_name_t reg_list[] =
{
  {REG_GAIN, "GAIN"},
  {REG_BLUE, "BLUE"},
  {REG_RED, "RED"},

  {REG_PID, "PID"},
  {REG_VER, "VER"},
  
  {REG_SCALING_XSC, "SCALING_XSC"},
  {REG_SCALING_YSC, "SCALING_YSC"},
  {REG_SCALING_DCWCTR, "SCALING_DCWCTR"},
  {REG_SCALING_PCLK_DIV, "SCALING_PCLK_DIV"},
  {REG_HSTART, "HSTART"},
  {REG_HSTOP, "HSTOP"},
  {REG_HREF, "HREF"},
  {REG_VSTART, "VSTART"},
  {REG_VSTOP, "VSTOP"},
  {REG_VREF, "VREF"},
  
  {REG_CLKRC, "CLKRC"},

  {REG_RGB444, "RGB444"},
  
  {MTX1, "MTX1"},
  {MTX2, "MTX2"},
  {MTX3, "MTX3"},
  {MTX4, "MTX4"},
  {MTX5, "MTX5"},
  {MTX6, "MTX6"},
  {MTXS, "MTXS"},

  {REG_TSLB, "TSLB"},

  {REG_COM1, "COM1"},
  {REG_COM3, "COM3"},
  {REG_COM7, "COM7"},
  {REG_COM9, "COM9"},
  {REG_COM13, "COM13"},
  {REG_COM14, "COM14"},
  {REG_COM15, "COM15"},
  {REG_COM16, "COM16"},
  
  {REG_REG76, "REG76"},

  {0xB0, "RSVD 0xB0"},

  {0xFF, "END"}
};

void arduinoMegaInit()
{
  // Setup PWM clock
  // OV7670 XCLK => (through level shift!) => pin 46 = PL3(OC5A), output
  DDRL |= _BV(3);
  TCCR5A = _BV(COM5A0) | _BV(WGM51) | _BV(WGM50);
  TCCR5B = _BV(WGM53) | _BV(WGM52) | _BV(CS50);
  OCR5A = 0; // (F_CPU)/(2*(X+1))
  
  delay(1000);
  
  // OV7670 SIOC => (through level shift!) => SCL
  // OV7670 SIOD => (through level shift!) => SDA
  // set up twi for 100khz
  // F_SCL = F_CPU/(16 + 2* TWBR* 4^(TWSR&3))
  TWSR &= ~3; //disable prescaler for TWI
  TWBR = 72;
}

void setup()
{
  Serial.begin(9600);
  
  arduinoMegaInit();
  Serial.println("debug: arduino mega init done");
  
  wrReg(REG_COM7, 0x80); //Reset the camera.
  delay(100);
  Serial.println("debug: reset camera done");
  
  int i = 0;
  while (reg_list[i].reg_num != 0xFF)
  {
    uint8_t regVal = rdReg(reg_list[i].reg_num);
    Serial.print(reg_list[i].reg_name);
    Serial.print(": ");
    Serial.print(regVal, HEX);
    Serial.print(" hex (");
    Serial.print(regVal);
    Serial.println(" decimal)");
    i++;
  }
  
  Serial.println("debug: setup done");
}

void loop()
{
  if (Serial.available() > 0)
  {
    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }
}
