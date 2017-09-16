#include <ESP8266WiFi.h>

const char WiFiAPPSK[] = "bigredbacks";

const int LED_PIN = 5; 
boolean turnedOn;
WiFiServer server(80);
void setup() 
{
  turnedOn = false;
  initHardware();
  setupWiFi();
  server.begin();
}
void loop() 
{
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  String req = client.readStringUntil('\r');
  Serial.print("req: ");
  Serial.println(req);
  client.flush();
  if (req.indexOf("hello") != -1){
    Serial.println("toggle on");
    turnedOn = !turnedOn;
    digitalWrite(LED_PIN,(turnedOn)?HIGH:LOW);
    
  }
  client.flush();
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s+="<!DOCTYPE html> <html> <body> <h2>The XMLHttpRequest Object</h2> <button type=\"button\" onclick=\"loadDoc()\">Request data</button> <p id=\"demo\"></p> <script> function loadDoc() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"demo\").innerHTML = \"Success!\" } }; xhttp.open(\"GET\", \"hello\", true); xhttp.send(); } </script> </body> </html>";
  client.print(s);
  delay(1);

}

void setupWiFi()
{
  WiFi.mode(WIFI_AP);
  String AP_NameString = "J3s IOT";
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