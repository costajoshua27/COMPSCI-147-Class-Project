#include <Arduino.h>

int buttonOn;
int timeSincePressed;
const int GREEN_PIN = 25;
const int YELLOW_PIN = 33;
const int RED_PIN = 32;

void setup() {
  // Set lights as output
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  Serial.begin(9600);

  buttonOn = 0;
  
}

void loop() {

  if (buttonOn % 2 == 0) {
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(YELLOW_PIN, HIGH);
    digitalWrite(RED_PIN, HIGH);
  }
  else {
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
  }
  Serial.print("Hello");
  buttonOn++;
  sleep(1);
}