#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define PIN D2
#define BRIGHTNESS 100

String currentMessage;
int maxOffset = 0;
int currentOffset = 0;

WiFiServer server(7331);
WiFiClient client;
WiFiUDP Udp;
char incomingPacket[255];

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_SEQUENCE,
  NEO_GRB            + NEO_KHZ800);


void setMessage(String msg){
  currentMessage = msg;
  currentOffset = matrix.width();
  maxOffset = msg.length() * 6; // 6 is font width
  
  Serial.print("currentmessage=");
  Serial.println(msg);
  Serial.print("currentmessage.length=");
  Serial.println(msg.length());
}

void waitingDots(){
  static int wdCounter = 0;

  if(wdCounter++ < 5){
    matrix.drawPixel(wdCounter * 2 - 2, 7, matrix.Color(255, 0, 0));
  }else{
    matrix.clear();
    wdCounter = 0;
  }
  matrix.show();
}

void setup() {
  Serial.begin(9600);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(BRIGHTNESS);
  matrix.setTextColor(matrix.Color(255, 0, 0));

  matrix.clear();
  matrix.setCursor(0, 0);

  WiFi.begin("36C3-things", "congress2019");
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) { 
    waitingDots();
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  setMessage(WiFi.localIP().toString());
  Serial.print("Connected to WiFi! IP: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
  Udp.begin(1337);
}
 
void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write("Thank you :)");
    Udp.endPacket();
  
    setMessage(String(incomingPacket));
  }

  matrix.clear();
  matrix.print(currentMessage.c_str());
  matrix.setCursor(currentOffset, 0);
  currentOffset -= 1;
  // calculate new color for next run
  if(currentOffset < -maxOffset) { // negative scroll width for message to be out of screen entirely
    currentOffset = matrix.width();

    uint16 colorPool = 512;
    uint8 r, g, b;
    r = random(255);
    g = random(255);
    b = max(0, min(255, colorPool - r - g));
    matrix.setTextColor(matrix.Color(r, g, b));
  }

  matrix.show();
  delay(90);
}
