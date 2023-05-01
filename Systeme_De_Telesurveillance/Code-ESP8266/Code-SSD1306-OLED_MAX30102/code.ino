	    #include <Wire.h>
		#include <Adafruit_SSD1306.h>
		#include "MAX30105.h"
		#include <MAX30105.h>
		
		#define SCREEN_WIDTH 128
		#define SCREEN_HEIGHT 32
		Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
		
		MAX30105 particleSensor;
		
		void setup() {
			display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
			display.display();
			delay(2000);
			display.clearDisplay();
			
			Wire.begin(4, 5);
			particleSensor.begin(Wire, I2C_SPEED_FAST);
			particleSensor.setup();
			particleSensor.setPulseAmplitudeRed(0x0A);
			particleSensor.setPulseAmplitudeGreen(0);
		}
		
		void loop() {
			int32_t ir, red;
			float temperature, bpm, spo2;
			
			temperature = particleSensor.readTemperature();
			ir = particleSensor.getIR();
			red = particleSensor.getRed();
			bpm = particleSensor.getHeartRate();
			spo2 = particleSensor.getSpO2();
			
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