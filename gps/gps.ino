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
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //RMC and GGA data sent to serial monitor
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY); //RMC data only

  // send both commands below to change both the output rate (how often the position
  // is written to the serial line), and the position fix rate.
  
  // set 10 Hz update rate - for 9600 baud you'll have to set the output to RMC only (see above)
  // Note the position can only be updated at most 5 times a second so it will lag behind serial output.
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);
  
  // set 5hz update rate
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  //GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);

  // request updates on antenna status, comment out to keep quiet
  // GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  
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
}

float toRad(float degree){
  return (degree * 71) / 4068;
}

void calculateDistance(){
  float lat1rad = toRad(degLat);
  float lat2rad = toRad(GPS.latitudeDegrees);
  float deltaLat = toRad(degLat-GPS.latitudeDegrees);
  float deltaLong = toRad(degLong-GPS.longitudeDegrees);
  const float r = 6731e3; //radius of earth in meters

  Serial.print("previous latitude: "); Serial.print(lat); Serial.print(latDir); Serial.print(", "); 
  Serial.print(lon); Serial.println(lonDir); 
  Serial.print("current latitude: "); Serial.print(GPS.latitude); Serial.print(GPS.lat); Serial.print(", ");
  Serial.print(GPS.longitude); Serial.print(GPS.lon); Serial.println(", ");

  float a = sin(deltaLat/2) * sin(deltaLat/2) + cos(lat1rad) * cos(lat2rad) * sin(deltaLong/2) * sin(deltaLong/2);
  float c = 2 * atan2(sqrt(a), sqrt(1-a));

  Serial.print(a, 9); Serial.print("         "); Serial.println(c, 9);

  float distance = r * c;
  
  //float distance = acos(sin(lat1rad)*sin(lat2rad) + cos(lat1rad)*cos(lat2rad)*cos(deltaL)) * r;
  totalDistance += distance;
  Serial.print("the total distance is: "); Serial.println(totalDistance, 9);
}

void loop()                     // run over and over again
{
  
  // in case you are not using the interrupt above, you'll
  // need to 'hand query' the GPS, not suggested :(
  /*if (! usingInterrupt) {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
  }*/
  
  // if a sentence is received, we can check the checksum, parse it...
  if(GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
  if(!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
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
