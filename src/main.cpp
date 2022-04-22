#include <Arduino.h>

#define RED_PIN 32
#define YELLOW_PIN 33
#define GREEN_PIN 25
#define BUTTON_PIN 15
#define BUZZER_PIN 2

// Hard coded enumerator
#define RED_STATE 0
#define RED_YELLOW_STATE 1
#define YELLOW_STATE 2
#define GREEN_STATE 3

#define RED_MILLIS 10000
#define RED_YELLOW_MILLIS 2000
#define YELLOW_MILLIS 2000
#define GREEN_MILLIS 5000

bool is_button_pressed;   // Flag to notify if button has bene pressed
int tl_state;           // Traffic light state.
unsigned long tl_timer; // Traffic light timer.
unsigned long prev_time; // Time of program execution during start of previous loop
unsigned long curr_time;
unsigned long buzzer_start_time; // Time when buzzer starts buzzing
bool is_buzzer_on; // Flag indicating if the buzzer is on
void setup()
{
    // Configure LED pins as outputs. 
    pinMode(RED_PIN, OUTPUT); 
    pinMode(YELLOW_PIN, OUTPUT); 
    pinMode(GREEN_PIN, OUTPUT);
    pinMode (BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);

    // Initial state for states and timers.
    curr_time = millis();
    prev_time = curr_time;
    tl_state = RED_STATE;

    is_buzzer_on = true;
    buzzer_start_time = curr_time;

    tl_timer = prev_time + RED_MILLIS;

    Serial.begin(9600);
}

void loop()
{
  // Update time
  curr_time = millis();
  unsigned long time_diff = curr_time - prev_time;
  prev_time = curr_time;

  tl_timer = time_diff > tl_timer ? 0 : tl_timer - time_diff;
  // Serial.print("HELLO");
  Serial.println(tl_timer);
  // TL state machine
  switch (tl_state)
  {
    case RED_STATE:
      // Turn red light on.
      digitalWrite(RED_PIN, HIGH);
      if (tl_timer == 0/*Timer expired*/)
      { 
        // Turn red light off.
        digitalWrite(RED_PIN, LOW);
        // Set timer for red-yellow state.
        tl_state = RED_YELLOW_STATE;
        tl_timer = RED_YELLOW_MILLIS;

      }
      break;
    case RED_YELLOW_STATE:
        // Code for red-yellow state.
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(YELLOW_PIN, HIGH);

        if (tl_timer == 0) {
          // Turn off red and yellow light
          digitalWrite(RED_PIN, LOW);
          digitalWrite(YELLOW_PIN, LOW);
          // Set timer for green state.
          tl_state = GREEN_STATE;
          is_button_pressed = false;
          tl_timer = 0;
        }
      break; 
    case YELLOW_STATE:
        // Code for yellow state.
        digitalWrite(YELLOW_PIN, HIGH);

        if (tl_timer == 0) {
          digitalWrite(YELLOW_PIN, LOW);
          tl_state = RED_STATE;
          tl_timer = RED_MILLIS;
        }
      break;
    case GREEN_STATE:
      // Turn green light on.
      digitalWrite(GREEN_PIN, HIGH);
      
      if (tl_timer == 0 && is_button_pressed/*Timer expired AND touch-button was pressed*/)
      {
        // Turn green light off.
        digitalWrite(GREEN_PIN, LOW);
        // Set timer for yellow 
        tl_state = YELLOW_STATE;
        tl_timer = YELLOW_MILLIS;
      }
      break;
  }
  // Detect touch - button pressed.
  int button_state = digitalRead(BUTTON_PIN);
  // Serial.print(button_state);
  if (button_state && !is_button_pressed) {
    is_button_pressed = true;
    tl_timer = GREEN_MILLIS;
  }
  
  // Buzzer state machine.
  switch (tl_state)
  {
    case RED_STATE:
      if (buzzer_start_time == 0 || (!is_buzzer_on && millis() - buzzer_start_time > 500)) {
        buzzer_start_time = millis();
        // Turn on buzzer
        is_buzzer_on = true;
        // tone(buzzer, 1000); // Send 1KHz sound signal... ADJUST BUZZING
        digitalWrite(BUZZER_PIN, HIGH);
      }
      if (is_buzzer_on && millis() - buzzer_start_time > 250) {
        is_buzzer_on = false;
        // Turn off buzzer
        digitalWrite(BUZZER_PIN, LOW);
      }
      break;
    case GREEN_STATE:
      if (buzzer_start_time == 0 || (!is_buzzer_on && millis() - buzzer_start_time > 1500)) {
        buzzer_start_time = millis();
        // turn on buzzer
        is_buzzer_on = true;
        // tone(buzzer, 1000); // Send 1KHz sound signal... ADJUST BUZZING
        digitalWrite(BUZZER_PIN, HIGH);
      }

      if (millis() - buzzer_start_time > 500 && millis() - buzzer_start_time < 1500) {
        is_buzzer_on = false;
        // Turn off buzzer
        digitalWrite(BUZZER_PIN, LOW);
      }
      break;
    default:
      // Set is_buzzer_on time to 0 for when state changes
      buzzer_start_time = 0;
      // Turn off buzzer
      digitalWrite(BUZZER_PIN, LOW);
  }

}
