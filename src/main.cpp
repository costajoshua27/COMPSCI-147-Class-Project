#include <Arduino.h>
#include <SPI.h>
#include <Servo.h>
#include <WiFi.h>
#include <HttpClient.h>

#include "TFT_eSPI.h"
#include "Adafruit_LC709203F.h"
#include "time.h"

const int SERVO_PIN = 25;
const int GREEN_PIN = 12;
const int YELLOW_PIN = 13;
const int BUZZER_PIN = 33;
const int DISPLAY_WIDTH = 135;
const int DISPLAY_HEIGHT = 240;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

char ssid[] = "";    // your network SSID (name) 
char pass[] = ""; // your network password (use for WPA, or use as key for WEP)

// Name of the server we want to connect to
const char kHostname[] = "";
const int kPort = 5000;

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

bool chargerOn = false;

int iterations = 0;
float totalBattery = 0;

String id;

Servo myservo;
Adafruit_LC709203F batteryGauge;
TFT_eSPI display = TFT_eSPI();

void setupBatteryGauge() {
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, HIGH);

  batteryGauge.begin();
  batteryGauge.setThermistorB(3950);
  batteryGauge.setPackSize(LC709203F_APA_1000MAH);
  batteryGauge.setAlarmVoltage(3.7);

  delay(1000);

  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(YELLOW_PIN, LOW);
  ledcAttachPin(BUZZER_PIN, 0);
}

void setupWifi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void makeRequest(HttpClient http, float battery) {
  int err = 0;

  // Get the time
  epochTime = getTime();

  // Build the url path
  String kPath = "/?id=" + id + "&charge=" + String(battery, 2) + "&timestamp=" + String(epochTime);
  char kPathChar[100];
  kPath.toCharArray(kPathChar, 100);

  // Make the request
  err = http.get(kHostname, kPort, kPathChar);
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                // Print out this character
                Serial.print(c);
               
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();
}

String randString() {
  String str = "";
  int generated = 0;
  while (generated < 6) {
    byte rand = random(0, 26);
    char letter = rand + 'a';
    str += letter;
    ++generated;
  }
  return str;
}

void setup() {
  // Setup serial
  Serial.begin(9600);

  // Setup display
  display.init();
  display.fillScreen(TFT_BLACK);
  display.setRotation(1);
  display.setTextSize(3);
  display.setTextColor(TFT_WHITE, TFT_BLACK);

  // Setup LED pins
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);

  // Setup Voltage pin to charge battery charger
  // pinMode(VOLTAGE_PIN, OUTPUT);
  myservo.attach(SERVO_PIN);

  // Setup Adafruit LC709203F Battery Gauge
  setupBatteryGauge();

  // Setup WiFi
  setupWifi();

  // Config time
  configTime(0, 0, ntpServer);

  // Get a charge ID
  id = "";
  id += randString();

  Serial.println(id);
}

void loop() {
  WiFiClient c;
  HttpClient http(c);

  chargerOn = !chargerOn;
  if (chargerOn) {
    myservo.write(45);

    display.fillScreen(TFT_BLACK);
    display.drawString("CHARGER ON", 10, 10);
    display.drawString("ID: " + id, 10, 50);
    delay(2000);
  }
  else {
    myservo.write(135);
    setupBatteryGauge();

    if (iterations == 5) {
      float avg = totalBattery / iterations;

      makeRequest(http, avg);

      totalBattery = 0;
      iterations = 0;

      if (avg > 95) {
        // turn on buzzer
        ledcWrite(0, 100);
      }
      else {
        // turn buzzer off
        ledcWrite(0, 0);
      }
    }

    delay(1000);

    float voltage = batteryGauge.cellVoltage();
    float percentage = batteryGauge.cellPercent();

    totalBattery += percentage;

    display.fillScreen(TFT_BLACK);
    Serial.print("Batt Voltage: "); Serial.println(voltage, 3);
    Serial.print("Batt Percent: "); Serial.println(percentage, 1);

    String voltageStr = "V: ";
    voltageStr += String(voltage);

    String percentageStr = "P: ";
    percentageStr += String(percentage);

    display.drawString(voltageStr, 10, 10);
    display.drawString(percentageStr, 10, 50);
    display.drawString("ID: " + id, 10, 70);

    iterations += 1;
    delay(1000);
  }
}