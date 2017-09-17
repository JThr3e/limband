// this sketch will allow you to bypass the Atmega chip
// and connect the Ultimate GPS directly to the USB/Serial
// chip converter.
 
// Connect VIN to +5V
// Connect GND to Ground
// Connect the GPS TX (transmit; data out from GPS) pin to Digital 3
// Connect the GPS RX (receive; data into GPS) pin to Digital 2

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <Wire.h>
#include <I2C_Anything.h>

//setting up software serial, can change pins to match wiring
SoftwareSerial mySerial(3, 2);
Adafruit_GPS GPS(&mySerial);
#define GPSECHO false //true if GPS echos data to serial monitor

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

//global variables
unsigned long timer = millis();
float totalDistance = 0;
float changeInDistance = 0;
float instVelocity = 0;
uint8_t h, m, s, y, mo, d;
uint16_t ms;
int32_t latitude_fixed, longitude_fixed;
float degLat, degLong;
float lat, lon;
//float geoidheight, altitude;
//float speed = knots, angle, magvariation, HDOP;
char latDir, lonDir;
int counter = 0;

void setup() {
  Serial.begin(115200); //connect to read GPS fast enough
  GPS.begin(9600); //default baud rate for GPS
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); //RMC data only
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);

  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  
  useInterrupt(true);
  delay(1000);
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

void requestEvent(){
  byte v = (instVelocity/40)*255;
  byte d = (changeInDistance/40)*255;
  byte msg[] = {v, d};
  Wire.write(msg, 2);
}

void setVariables(){
  h = GPS.hour;
  m = GPS.minute;
  s = GPS.seconds;
  ms = GPS.milliseconds;
  y = GPS.year;
  mo = GPS.month;
  d = GPS.day;
  degLat = GPS.latitudeDegrees;
  degLong = GPS.longitudeDegrees;
  latDir = GPS.lat;
  lonDir = GPS.lon;
  lat = GPS.latitude;
  lon = GPS.longitude;
  instVelocity = convertKnots(GPS.speed);
}

float toRad(float degree){
  return (degree * 71) / 4068;
}

float convertKnots(float knots){
  return knots*1.852; //to kmph
}

void calculateDistance(){
  float lat1rad = toRad(degLat);
  float lat2rad = toRad(GPS.latitudeDegrees);
  float deltaLat = toRad(degLat-GPS.latitudeDegrees);
  float deltaLong = toRad(degLong-GPS.longitudeDegrees);
  const float r = 6371; //radius of earth in kilometers

  Serial.print("previous latitude: "); Serial.print(lat); Serial.print(latDir); Serial.print(", "); 
  Serial.print(lon); Serial.println(lonDir); 
  Serial.print("current latitude: "); Serial.print(GPS.latitude); Serial.print(GPS.lat); Serial.print(", ");
  Serial.print(GPS.longitude); Serial.print(GPS.lon); Serial.println(", ");

  float a = sin(deltaLat/2) * sin(deltaLat/2) + cos(lat1rad) * cos(lat2rad) * sin(deltaLong/2) * sin(deltaLong/2);
  float c = 2 * atan2(sqrt(a), sqrt(1-a));

  Serial.print(a, 9); Serial.print("         "); Serial.println(c, 9);

  changeInDistance = r * c;
  
  //float distance = acos(sin(lat1rad)*sin(lat2rad) + cos(lat1rad)*cos(lat2rad)*cos(deltaL)) * r;
  totalDistance += changeInDistance;
  Serial.print("the total distance is: "); Serial.println(totalDistance, 9);
}

void loop()                     // run over and over again
{
  // if a sentence is received, we can check the checksum, parse it...
  if(GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
  if(!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to falsex
     return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  if(counter == 0){
    setVariables();
    counter++;
  }

  // if millis() or timer wraps around, we'll just reset it
  if (millis() - timer > 1000){
    timer = millis();
    calculateDistance();
    setVariables();
  }
}
