#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 12
#define STRIPLENGTH 44

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
int amt;
float distance = 6.8; //change this to match with GPS
float userDistanceInput = 10; //change this to match with UI

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  amt = 8;
  pinMode(PIN, OUTPUT);
  Serial.begin(9600);
}

void pulse(){
  for(int i = 255; i > 0; i--){
    strip.setPixelColor(i, strip.Color(127, 0, 255));
    strip.setBrightness(i);
    strip.show();
    delay(10);
  }
  for(int i = 0; i < 255; i++){
    strip.setPixelColor(i, strip.Color(127, 0, 255));
    strip.setBrightness(i);
    strip.show();
    delay(10);
  }
}

void parade() {
  //while(waiting on user or gps init)
  for(int i = 0; i < strip.numPixels(); i++) {
    if (i % 2 == 0)
    strip.setPixelColor(i, strip.Color(0, 255, 0));
    else
    strip.setPixelColor(i, 0);
    strip.show();
    delay(50);
  }
    for(int i = 0; i < strip.numPixels(); i++) {
    if (i % 2 == 0)
    strip.setPixelColor(i, 0);
    else
    strip.setPixelColor(i*2, strip.Color(0, 255, 0));
    strip.show();
    delay(50);
    }
}

void flash(){
  int countdown = 3;
  while (countdown > 0){
    if (countdown == 1){
       ready(strip.Color(0, 255, 0));
    }
    else {
      ready(strip.Color(255, 0, 0));
    }
    strip.show();
    off();
    countdown--;
  }
}

void ready(uint32_t c){
  strip.clear();
  strip.show();
  for(int i = 0; i < strip.numPixels(); i++){
    strip.setPixelColor(i, c);
    strip.show();
  }
   delay(750);
 }

void off(){
  strip.clear();
  strip.show();
  delay(500);
}

void progress(){
  strip.clear();
  strip.show();
  int count = strip.numPixels() * (distance/userDistanceInput);
  float loading = (float) strip.numPixels() * (distance/userDistanceInput);
  
  for(int i = 0; i < count; i++){
    strip.setPixelColor(i, strip.Color(255, 255, 0));    
    strip.show();
    delay(20);
  }

  while(loading <= count + 1){
    strip.setPixelColor(count, strip.Color(255, 0, 0));
    strip.show();
    delay(500);
    strip.setPixelColor(count, 0);
    strip.show();
    delay(500);
  }
}

void whatPattern(char id){
  switch(id){
    case '0': 
      Serial.println("pattern: ready");
      ready(strip.Color(0, 255, 0));
      break;
    case '1':
      Serial.println("pattern: progress");
      progress();
      break;
   case '2':
      Serial.println("pattern: flash");
      flash();
      break;
   case '3':
      Serial.println("pattern: off");
      off();
      break;
  case '4':
      Serial.println("pattern: parade");
      parade();
      break;
  case '5': 
      Serial.println("pattern: pulse");
      pulse();
      break;
  }
}

void loop() {
  if(Serial.available()){
    char ch = Serial.read();
    whatPattern(ch);
  }
}






