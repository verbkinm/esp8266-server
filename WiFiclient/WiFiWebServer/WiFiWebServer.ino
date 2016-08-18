/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <ESP.h>
#include <string.h>

#define PORT 5555
#define PING "ping"

const char* ssid = "LYCEUM";
const char* password = "svetlana";
char servername[]="192.168.100.13";

byte mac[6];                     // the MAC address of your Wifi shield
String macaddr;
String ip;

WiFiClient client;

int     incomingByte = 0;
String  val = "low";

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local ip: ");
  Serial.println(WiFi.localIP());

  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);

  macaddr = "MAC:" + String(mac[5],HEX) + ":" + String(mac[4],HEX) + ":" +String(mac[3],HEX) + ":" +String(mac[2],HEX) + ":" +String(mac[1],HEX) + ":" +String(mac[0],HEX);
  macaddr.toUpperCase();
}

void loop() {
  Serial.println("begin loop");
  if(client.connect(servername,PORT)){
    Serial.print("connected to server");Serial.print(" ");Serial.print(servername);Serial.print(":");
    Serial.println(PORT);
    client.print(macaddr);
    client.print(val);
    delay(500);
  }
  while(client.connected()){
    client.print(PING);
    String rdStr = Serial.readString(); 
    
// Отправка команд серверу
    if(strlen(rdStr.c_str()) > 0){
      String sndStr = "cmd_";
      sndStr += rdStr;
      Serial.println(sndStr);
      client.print(sndStr);
    }
    
    incomingByte  = client.read();
    if(incomingByte != -1){
        Serial.print("Server sent: ");
        Serial.println(incomingByte);
        if(incomingByte == 48 ){
            digitalWrite(2, 0);
            Serial.println("GPIO is now low");
            client.print("low");
            val = "low";
            client.flush();
        }
        else if(incomingByte == 49 ){
            digitalWrite(2, 1);
            Serial.println("GPIO is now high");
            client.print("high");
            val = "high";
            client.flush();
        }
        else if(incomingByte == 50 ){
            ESP.restart();
        }
        else{
            Serial.println("invalid request");
            client.print("invalid request");  
        }
    }
    if (!client.connected()) {
        Serial.println("disconnected");
        client.stop();
        return;
    }
    delay(10);
  }
  Serial.println("end loop");
  delay(1000);
}

