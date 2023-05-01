#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include <MAX30105.h>

#include <time.h>
#include "secrets.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
		
MAX30105 particleSensor;
int32_t ir, red;
float temperature, bpm, spo2;

unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5000;
 
#define AWS_IOT_PUBLISH_TOPIC   "esp8266/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp8266/sub"
 
WiFiClientSecure net;
 
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);
 
PubSubClient client(net);
 
time_t now;
time_t nowish = 1510592825;
 
 
void NTPConnect(void){
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish){
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}
 
 
void messageReceived(char *topic, byte *payload, unsigned int length){
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
 
 
void connectAWS(){
  delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));
 
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
 
  NTPConnect();
 
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
 
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);
 
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME)){
    Serial.print(".");
    delay(1000);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
 
void publishMessage(){
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["temperature"] = temperature;
  doc["Heart-Rate"] = bpm;
  doc["SpO2"] = spo2;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
 
void setup(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.display();
	delay(2000);
	display.clearDisplay();

  Wire.begin(4, 5);
	particleSensor.begin(Wire, I2C_SPEED_FAST);
	particleSensor.setup();
	particleSensor.setPulseAmplitudeRed(0x0A);
	particleSensor.setPulseAmplitudeGreen(0);

  Serial.begin(115200);
  connectAWS();


}
 
 
void loop(){
  
  readSensor();
  displaySSD1306();
  if (isnan(temperature) || isnan(bpm) ||isnan(bpm) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  Serial.println("Temperature: " + String(temperature) + " °C");
	Serial.println("Heart Rate: " + String(bpm) + " bpm");
	Serial.println("SpO2: " + String(spo2) + " %");

  delay(2000);
 
  now = time(nullptr);
 
  if (!client.connected()){
    connectAWS();
  }
  else{
    client.loop();
    if (millis() - lastMillis > 5000){
      lastMillis = millis();
      publishMessage();
    }
  }
}

void readSensor(){
  temperature = particleSensor.readTemperature();
	ir = particleSensor.getIR();
	red = particleSensor.getRed();
	bpm = particleSensor.getHeartRate();
	spo2 = particleSensor.getSpO2();
}

void displaySSD1306(){
  display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	display.println("Temperature: " + String(temperature) + " C");
	display.setCursor(0, 10);
	display.println("Heart Rate: " + String(bpm) + " bpm");
	display.setCursor(0, 20);
	display.println("SpO2: " + String(spo2) + " \%");
	display.display();
			
	delay(1000);
}