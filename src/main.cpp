#include <Arduino.h>
#include <WiFi.h>
#include <HttpClient.h>
#include "DHT20.h"

DHT20 DHT(&Wire);

#define DATA_PIN 21
#define CLOCK_PIN 22

char ssid[] = "UCInet Mobile Access";    // your network SSID (name) 
char pass[] = ""; // your network password (use for WPA, or use as key for WEP)

// Name of the server we want to connect to
const char kHostname[] = "18.188.228.61";
const int kPort = 5000;

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

void setup() {
  // Initialize humidity and temperature sensor
  DHT.begin(DATA_PIN, CLOCK_PIN);

  // Start the serial connection
  Serial.begin(9600);

  // We start by connecting to a WiFi network
  delay(1000);
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

void loop() {
  int err = 0;
  
  WiFiClient c;
  HttpClient http(c);

  // READ SENSOR DATA
  Serial.println("Type,\tStatus,\tHumidity (%),\tTemperature (C)");
  Serial.print("DHT20, \t");
  int status = DHT.read();
  switch (status)
  {
  case DHT20_OK:
    Serial.print("OK,\t");
    break;
  case DHT20_ERROR_CHECKSUM:
    Serial.print("Checksum error,\t");
    break;
  case DHT20_ERROR_CONNECT:
    Serial.print("Connect error,\t");
    break;
  case DHT20_MISSING_BYTES:
    Serial.print("Missing bytes,\t");
    break;
  default:
    Serial.print("Unknown error,\t");
    break;
  }
  // DISPLAY DATA, sensor has only one decimal.
  float humidity = DHT.getHumidity();
  float temperature = DHT.getTemperature();

  Serial.print(humidity, 1);
  Serial.print(",\t");
  Serial.println(temperature, 1);
  Serial.println();

  // Build the url path
  String kPath = "/?temp=" + String(temperature, 2) + "&hum=" + String(humidity, 2);
  char kPathChar[50];
  kPath.toCharArray(kPathChar, 50);

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

  delay(2000);
}