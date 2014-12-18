#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN    6    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 53

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

volatile int state = 0;
volatile int flag = 0;
volatile int time = 0;

void setup() {
  //set clock speed to max
  CLKPR = (1<<CLKPCE);
  CLKPR = 0;
  
  //set up interrupts for button clicks
  attachInterrupt(1, button1, HIGH);
  attachInterrupt(2, button2, HIGH);
  attachInterrupt(3, button3, HIGH);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  
  //clear flag before and after show plays
  if (flag == 1) {
    flag = 0;
  }
  
  //play triggered light show
  startShow(state);
  }

void startShow(int i) {
  switch(i){
    case 0: colorWipe2(strip.Color(0, 0, 0), 80);    // Default state = off
            break;
    case 1: colorWipe1(strip.Color(0, 0, 255), 160);  // Blue
            break;            
    case 2: state = 0;
            rainbowCycle(3);  // Rainbow
            break;
    case 3: colorWipe1(strip.Color(0, 255, 0), 80);  // Green
            break;
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

//self clearing and watches for flag from interrupt
void colorWipe1(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
    time++;
    if ((flag == 1)) {
      colorWipe(strip.Color(0, 0, 0), 1);
      break;
      }
      if (time > 54) {
        time = 0;
        state = 0;
        colorWipe2(strip.Color(0, 0, 0), 80);
        break;
        }        
      }
    }

//slow clearing after each show
void colorWipe2(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait); 
      
      if ((flag == 1)) {
        colorWipe(strip.Color(0, 0, 0), 1);
        break;
      }
  }
}

//rainbow show
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
    
    if ((flag == 1)) {
        colorWipe(strip.Color(0, 0, 0), 1);
        break;
    }
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

//button interrupts for each show type
void button1()
{
  state = 1;
  flag = 1;
  time = 0;
}

void button2()
{
  state = 2;
  flag = 1;
  time = 0;
}

void button3()
{
  state = 3;
  flag = 1;
  time = 0;
}
