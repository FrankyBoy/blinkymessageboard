#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define PIN D2
#define BRIGHTNESS 40

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
  maxOffset = msg.length() * 6;
  
  Serial.print("currentmessage=");
  Serial.println(msg);
  Serial.print("currentmessage.length=");
  Serial.println(msg.length());
}

void setup() {
  Serial.begin(9600);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(255);
  matrix.setTextColor(matrix.Color(255, 0, 0));

  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
  matrix.print(".");
  matrix.show();

  WiFi.begin("36C3-things", "congress2019");

  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    matrix.print(".");
    matrix.show();
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

  matrix.fillScreen(0);
  matrix.print(currentMessage.c_str());
  matrix.setCursor(currentOffset, 0);
  currentOffset -= 1;
  if(currentOffset < -maxOffset) { // negative scroll width for message to be out of screen entirely
    currentOffset = matrix.width();
    matrix.setTextColor(matrix.ColorHSV(
      (uint16)random(0, 65535),
      (uint8)random(100, 255),
      BRIGHTNESS));
  }

  matrix.show();
  delay(90);
}
