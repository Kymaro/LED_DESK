/*
 * Arduino interface for the use of WS2812 strip LEDs
 * Uses Adalight protocol and is compatible with Boblight, Prismatik etc...
 * "Magic Word" for synchronisation is 'Ada' followed by LED High, Low and Checksum
 * @author: Wifsimster <wifsimster@gmail.com> 
 * @library: FastLED v3.001
 * @date: 11/22/2015
 */
#include "FastLED.h"

//INFOS RUBAN AMBILIGHT
#define AMBI_LEDS 120
#define AMBI_PIN 5
#define AMBI_INT 2
byte ambi_on = LOW;

//INFOS RUBAN BLANC ICI
#define WHITE_LEDS 30
#define WHITE_PIN 10
#define WHITE_INT 3
byte white_on = LOW;

#define NUM_LEDS AMBI_LEDS+WHITE_LEDS

// Baudrate, higher rate allows faster refresh rate and more LEDs (defined in /etc/boblight.conf)
#define serialRate 115200

// Adalight sends a "Magic Word" (defined in /etc/boblight.conf) before sending the pixel data
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;

// Initialise LED-array
CRGB leds[AMBI_LEDS + WHITE_LEDS];

void setup() {
  // Use NEOPIXEL to keep true colors
  FastLED.addLeds<NEOPIXEL, AMBI_PIN>(leds, 0, AMBI_LEDS);
  FastLED.addLeds<NEOPIXEL, WHITE_PIN>(leds, AMBI_LEDS, WHITE_LEDS);
  
  pinMode(WHITE_INT, INPUT_PULLUP);
  pinMode(AMBI_INT, INPUT_PULLUP);

  whiteINT();
  ambiINT();

  attachInterrupt(digitalPinToInterrupt(WHITE_INT), whiteINT, CHANGE);
  attachInterrupt(digitalPinToInterrupt(AMBI_INT), ambiINT, CHANGE);
}

void loop() { 

  if(!ambi_on)
  {
    // Wait for first byte of Magic Word
    for(i = 0; i < sizeof prefix; ++i) {
    waitLoop: while (!Serial.available()) ;;
    // Check next byte in Magic Word
    if(prefix[i] == Serial.read()) continue;
    // otherwise, start over
    i = 0;
    goto waitLoop;
    }
  
    // Hi, Lo, Checksum  
    while (!Serial.available()) ;;
    hi=Serial.read();
    while (!Serial.available()) ;;
    lo=Serial.read();
    while (!Serial.available()) ;;
    chk=Serial.read();
  
    // If checksum does not match go back to wait
    if (chk != (hi ^ lo ^ 0x55)) {
      i=0;
      goto waitLoop;
    }
  
    memset(leds, 0, AMBI_LEDS * sizeof(struct CRGB));
    // Read the transmission data and set LED values
    for (uint8_t i = 0; i < AMBI_LEDS; i++) {
      byte r, g, b;    
      while(!Serial.available());
      r = Serial.read();
      while(!Serial.available());
      g = Serial.read();
      while(!Serial.available());
      b = Serial.read();
      leds[i].r = r;
      leds[i].g = g;
      leds[i].b = b;
    }
    // Shows new values
    FastLED.show();
  }else
  {
    delay(100);
  }
}

void whiteINT()
{
  white_on = digitalRead(WHITE_INT); // LOW = ON / HIGH = OFF
  if(!white_on)// LOW donc ON
  {
    for(int i = AMBI_LEDS; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB::White;
    }
  }else// HIGH donc OFF
  {
    for(int i = AMBI_LEDS; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB::Black;
    }  
  }
  FastLED.show();
}

void ambiINT()
{
  ambi_on = digitalRead(AMBI_INT);
  if(ambi_on) // HIGH donc OFF
  {
    for(int i = 0; i < AMBI_LEDS; i++)
    {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    Serial.end();
  }else // LOW donc ON
  {
    Serial.begin(serialRate);
    // Send "Magic Word" string to host
    Serial.print("Ada\n");
  }  
}