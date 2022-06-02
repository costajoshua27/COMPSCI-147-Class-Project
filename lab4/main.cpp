#include <Arduino.h>

#include "SparkFunLSM6DSO.h"
#include "Wire.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <TFT_eSPI.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool ledOn = false;
int steps = 0;
int flag = 0;
float threshold = 1;
// For getting avg sample
float xval[100] = {0};
float yval[100] = {0};
float zval[100] = {0};
// For the samples we get from walking
int samplesCount = 0;
float totvect[100] = {0};
float totave[100] = {0};
float xaccl[100] = {0};
float yaccl[100] = {0};
float zaccl[100] = {0};
float xavg, yavg, zavg;
unsigned long lastTime;

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      // Read message and build it
      String totalMessage = "";
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
        totalMessage.concat(value[i]);
      }
      Serial.println();
      Serial.println("*********");

      // Handle on and off commands
        if (totalMessage.equals("on")) {
          ledOn = true;
        } else if (totalMessage.equals("off")) {
          ledOn = false;
        }
    }
  }
};

LSM6DSO myIMU;
TFT_eSPI display = TFT_eSPI();

void calibrate() {
  Serial.println("Calibrating...");
  float sumX = 0;
  float sumY = 0;
  float sumZ = 0;

  for (int i = 0; i < 100; i++) {
    xval[i] = myIMU.readFloatAccelX();
    sumX = xval[i] + sumX;

    yval[i] = myIMU.readFloatAccelY();
    sumY = yval[i] + sumY;

    zval[i] = myIMU.readFloatAccelZ();
    sumZ = zval[i] + sumZ;

  }

  xavg = sumX / 100.0;
  yavg = sumY / 100.0;
  zavg = sumZ / 100.0;

  // for (int i = 0; i < 100; i++) {
  //   xval[i] = myIMU.readFloatAccelX();
  //   sum = xval[i] + sum;
  // }

  // delay(100);
  // xavg = sum / 100.0;
  // Serial.println(xavg);

  // for (int j = 0; j < 100; j++)
  // {
  //   yval[j] = myIMU.readFloatAccelY();
  //   sum1 = yval[j] + sum1;
  // }

  // yavg = sum1 / 100.0;
  // Serial.println(yavg);
  // delay(100);

  // for (int q = 0; q < 100; q++)
  // {
  //   zval[q] = myIMU.readFloatAccelZ();
  //   sum2 = zval[q] + sum2;
  // }

  // zavg = sum2 / 100.0;
  delay(100);
  // Serial.println(zavg);
  Serial.println("Calibrated!");
}

void setup() {
  // Setup serial
  Serial.begin(9600);
  delay(500);

  // Setup display
  display.init();
  display.fillScreen(TFT_BLACK);
  display.setRotation(1);
  display.setTextColor(TFT_WHITE, TFT_BLACK);

  // Setup LED
  pinMode(32, OUTPUT);

  // Setup IMU
  Wire.begin();
  delay(10);
  if (myIMU.begin()) {
    Serial.println("IMU ready.");
  }
  else {
    Serial.println("Could not connect to IMU.");
  }

  if (myIMU.initialize(BASIC_SETTINGS)) {
    Serial.println("Loaded IMU settings.");
  }

  // Setup BLE
  BLEDevice::init("MyESP32");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE
                                        );
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  pCharacteristic->setValue("Hello World");
  pService->start();
  
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
   pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x0); 
    pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  // Setup pedometer
  calibrate();

  // Set starting time of pedometer
  lastTime = millis();
}

void loop() {
  // Every 100 seconds, reset the arrays
  int currTime = millis();
  if (samplesCount == 100 || currTime - lastTime >= 20000) {
    memset(totvect, 0, sizeof(totvect));
    memset(totave, 0, sizeof(totave)); 
    memset(xaccl, 0, sizeof(xaccl)); 
    memset(yaccl, 0, sizeof(yaccl)); 
    memset(zaccl, 0, sizeof(zaccl));

    samplesCount = 0;

    // update last time we updated a step
    lastTime = currTime;
  }

  // Get gyroscope readings
  xaccl[samplesCount] = myIMU.readFloatAccelX();
  yaccl[samplesCount] = myIMU.readFloatAccelY();
  zaccl[samplesCount] = myIMU.readFloatAccelZ();

  // Get distance value from XYZ coord
  float squaredX = pow((xaccl[samplesCount] - xavg),2);
  float squaredY = pow((yaccl[samplesCount] - yavg),2); 
  float squaredZ = pow((zaccl[samplesCount] - zavg),2);
  totvect[samplesCount] = sqrt(squaredX + squaredY + squaredZ);
  totave[samplesCount] = (totvect[samplesCount] + totvect[samplesCount - 1]) / 2;

  Serial.print(totave[samplesCount]);
  Serial.print(" ");
  Serial.println(threshold);
  if (totave[samplesCount] > threshold && flag == 0) {
    steps += 1;
    flag = 1;

    char str[16];
    itoa(steps, str, 10);
    pCharacteristic->setValue(str);
  }

  else if (totave[samplesCount] > threshold && flag == 1) {

  }

  if (totave[samplesCount] < threshold && flag == 1) {
    flag = 0;
  }

  if (steps < 0) {
    steps = 0;
  }

  Serial.println('\n');
  Serial.print("steps: ");
  Serial.println(steps);

  uint16_t x = display.width() / 2;
  uint16_t y = display.height() / 2;

  // Set datum to Middle Right
  display.setTextDatum(MR_DATUM);

  // Set the padding to the maximum width that the digits could occupy in font 4
  // This ensures small numbers obliterate large ones on the screen
  display.setTextPadding( display.textWidth("8888", 7) );

  // Draw a floating point number with 2 decimal places with right datum in font 4
  display.drawNumber(steps, x, y, 7);

  digitalWrite(32, ledOn ? HIGH : LOW);

  samplesCount++;

  delay(500);
}