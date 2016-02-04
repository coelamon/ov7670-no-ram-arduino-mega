/* OV7670
 * Arduino Mega 2560
 * 
 * open serial monitor (9600 baud),
 * send anything,
 * wait about 30 seconds
 * see results
 *
 * connections:
 * OV7670 XCLK => (through level shift!) => pin 46 = PL3(OC5A), output
 * OV7670 VSYNC => pin 26 = PA4, input
 * OV7670 HREF => pin 27 = PA5, input
 * OV7670 PCLK => pin 28 = PA6, input
 * OV7670 3V3   => 3.3V
 * OV7670 RESET => 3.3V
 * OV7670 GND   => GND
 * OV7670 PWDN  => GND
 */

bool startCount = false;

void setup() {
  // Setup the 8mhz PWM clock
  // OV7670 XCLK => (through level shift!) => pin 46 = PL3(OC5A), output
  DDRL |= _BV(3);
  TCCR5A = _BV(COM5A0) | _BV(WGM51) | _BV(WGM50);
  TCCR5B = _BV(WGM53) | _BV(WGM52) | _BV(CS50);
  OCR5A = 127; // (F_CPU)/(2*(X+1))
  // OV7670 VSYNC => pin 26 = PA4, input
  DDRA &= ~ _BV(4);
  // OV7670 HREF => pin 27 = PA5, input
  DDRA &= ~ _BV(5);
  // OV7670 PCLK => pin 28 = PA6, input
  DDRA &= ~ _BV(6);
  
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0)
  {
    while (Serial.available() > 0)
    {
      Serial.read();
    }
    startCount = true;
  }
  
  if (!startCount)
    return;
  
  Serial.println("counter start");
  
  long byteCount = 0;
  int rowCount = 0;
  
  int rowIndex = 0;
  int bytesPerRowCount[481];
  
  for (int i = 0; i < 481; i++)
    bytesPerRowCount[i] = 0;

  // VSYNC
  while (PINA & _BV(4));//wait for low
  while (!(PINA & _BV(4)));//wait for high
  
  while (PINA & _BV(4));//wait for low
  
  int vsyncPrev = PINA & _BV(4);
  int vsyncNow = vsyncPrev;
  
  int hrefPrev = PINA & _BV(5);
  int hrefNow = hrefPrev;
  
  while (1)
  {
    vsyncNow = PINA & _BV(4);
    if (vsyncPrev == 0 && vsyncNow > 0)
      break;
    vsyncPrev = vsyncNow;
    
    hrefNow = PINA & _BV(5);
    if (hrefPrev == 0 && hrefNow > 0)
    {
      rowCount++;
      rowIndex = rowCount - 1;
      if (rowIndex > 480)
        rowIndex = 480;
    }
    hrefPrev = hrefNow;
    
    if (hrefNow > 0) // HREF is high
    {
      // PCLK
      while (PINA & _BV(6)); //wait for low
      while (!(PINA & _BV(6))); //wait for high
      if (hrefNow > 0) // HREF is high
      {
        byteCount++;
        bytesPerRowCount[rowIndex]++;
      }
    }
  }
  
  Serial.print("row count: ");
  Serial.print(rowCount);
  Serial.println("");
  Serial.print("byte count: ");
  Serial.print(byteCount);
  Serial.println("");
  
  Serial.print("bytes per row count: ");
  for (int i = 0; i < 480; i++)
  {
    Serial.print(bytesPerRowCount[i]);
    Serial.print(" ");
  }
  Serial.println("");
  
  Serial.println("done!");
  
  startCount = false;
}

