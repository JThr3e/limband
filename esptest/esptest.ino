#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN 12

const char WiFiAPPSK[] = "bigredbacks";
const int LED_PIN = 4; 
boolean turnedOn;
int km2run;
int timer;
WiFiServer server(80);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(115, PIN, NEO_GRB + NEO_KHZ800);

void setup() 
{
  km2run = 0;
  timer = 0;
  turnedOn = false;
  initHardware();
  setupWiFi();
  server.begin();
  strip.begin();
  strip.show();
}


//every second get distance traveled from the GPS
void loop() 
{
  runSimu();
  weefeeShet();
  delay(1);
}








void runSimu(){
  if(millis() - timer > 1000){
    timer = millis();
    if(km2run > 0){
      km2run -= random(1,10);
      Serial.println(km2run);
    }
  }
}

void weefeeShet(){
  WiFiClient client = server.available();
  if (client) {
    Serial.println(client);
  String req = client.readStringUntil('\r');
  Serial.println(client.available());
  Serial.print("req: ");
  Serial.println(req);
  client.flush();
  if (req.indexOf("?=") != -1){
    int strIndex = req.indexOf("=")+1;
    int stpIndex = req.indexOf("HTTP/1.1")-1;
    String dis = req.substring(strIndex, stpIndex);
    Serial.println(dis.toInt());
    if(dis.toInt() != 0 && km2run <= 0){
      km2run = dis.toInt();
      client.print("You will run for "+dis+" miles!");
      Serial.println("foo");
      client.flush();
    }
    else{
      client.print("Error! Zero value? Already in workout? Not a number?");
      Serial.println("bar");
      client.flush();
    }
  }
  else if(req.indexOf("cancel") != -1){
      km2run = 0;
      client.print("Workout Canceled!");
      client.flush();
  }
  else{
    Serial.println("bazzy");
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s +="<!DOCTYPE html> <html> <head> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width\"> <title>LimBand | Home</title> <style> body{ font:15px/1.5 Helvetica, Arial, sans-serif; padding: 0; margin: 0; background-color: gray; } /* Global */ .container{ width: 80%; margin: auto; overflow: hidden; } ul{ margin: 0; padding: 0; } /* Header **/ header{ background: #35424a; color: #ffffff; padding-top: 30px; min-height: 70px; border-bottom: pink 3px solid; } header a{ color: #ffffff; text-decoration: none; text-transform: uppercase; font-size: 16px; } header li{ float:left; display:inline; padding: 0 20px 0 20px; } header #branding{ float:left; } header #branding h1{ margin:0; } header nav{ float:right; margin-top: 10px; } header .highlight, header .current a{ color: #50EBEC; font-weight: bold; } header a:hover{ color: #46C7C7; font-weight: bold; } /* Showcase */ #showcase{ min-height: 400px; /* background:url('./doge.jpg') no-repeat 0 0; */ text-align: center; color: #ffffff; } #showcase h1{ margin-top: 100px; font-size: 55px; margin-bottom: 10px; } /* About */ #about{ min-height: 200px; text-align: center; color: #ffffff; } /*input/output section */ #IO { color: #ffffff; text-align: center; font-size: 30px; } footer { color: #ffffff; text-align: center; padding: 15px; } </style> </head> <body> <Header> <div class= \"container\"> <div id= \"branding\"> <h1>Lim<span class=\"highlight\">Band</span></h1> </div> <nav> <ul> <li class= \"current\"><a href=\"index.html\">Home</a></li> </ul> </nav> </div> </Header> <section id=\"showcase\"> <div class=\"container\"> <h1>Meet the LimBand!</h1> </div> </section> <section id=\"about\"> <div class=\"container\"> <h1>Meet the team!</h1> </div> </section> <section id=\"IO\"> <div class =\"container\"> <div id = \"info\">Current Workout:</div> <form id=\"form1\"> Enter Target Distance: <input name=\"miles\" type=\"text\" placeholder=\"Miles\"> <br> <button type=\"button\" class=\"button\" onclick=\"load()\">Submit</button> </form> <button id = \"cancel\" onclick=\"cancelWrk()\">Cancel Workout</button> <div id=\"output\"></div> </div> </section> <script> function load() { var x = document.getElementById(\"form1\"); console.log(\"MARK\"); var miles = x.elements[\"miles\"].value; var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"output\").innerHTML = this.responseText; } }; xhttp.open(\"GET\", \"/miles?=\"+miles, true); xhttp.send(); } function cancelWrk() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"output\").innerHTML = this.responseText; } }; xhttp.open(\"GET\", \"/cancel\", true); xhttp.send(); } </script> <footer> <p>LimBand, Copyright &copy; 2017</p> </footer> </body> </html>";
    client.print(s);
    client.flush();
  }
 }
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP);
  String AP_NameString = "ECEs cAn dO eVeRyThINg";
  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);
  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

void initHardware()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}
