
#define BLYNK_TEMPLATE_ID "TMPL4O-LP3Ert"
#define BLYNK_TEMPLATE_NAME "senzortemplate"
#define BLYNK_AUTH_TOKEN "7ep5xyxeAkkYEl3xZxKXSMEWqRGn_4e6"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wi-Fi credentials
char ssid[] = "P16_network";
char password[] = "manutd4ever";

// Blynk authentication token
char auth[] = "7ep5xyxeAkkYEl3xZxKXSMEWqRGn_4e6";

// HC-SR04 pin connections
const int trigPin = 5;
const int echoPin = 18;

const int buzzerPin = 26;
const int buttonPin = 27;

// Blynk virtual pin for the Gauge widget
const int distancePin = V1;

// Button variables
int buttonPressCount = 0;
bool buzzerActive = false;
int attempts = 0;
int passNumber = 5;

BLYNK_WRITE(V2)
{
  int num=param.asInt();
  

  if(num > 0) {
    passNumber = num; 
    Blynk.logEvent("pass_change", String("Password has been changed, new password is push button ") + num + String(" times")); 
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi connected");

  // Initialize Blynk
  Blynk.begin(auth, ssid, password);

  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  Blynk.run();
  measureDistance();
  handleButton();
  delay(1000);
}

void measureDistance() {
  long duration;
  int distance;

  // Send ultrasonic pulse
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo response
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance in centimeters
  distance = duration * 0.034 / 2;

  // Update Blynk Gauge widget
  Blynk.virtualWrite(distancePin, distance);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 10) {
    activateBuzzer(7000);
    
    
      Serial.print(attempts);
      if(attempts == 3){
        Blynk.logEvent("button_pushed", String("Password introduced incorrectly 3 times!"));
        attempts = 0;
      }
    
    delay(5000);
  }
}

void activateBuzzer(unsigned long duration) {
  unsigned long startTime = millis();  // Get the current time
  int pressCount = 0;
  static int tryPassword = 0;
  passNumber = random(3, 10);
  Blynk.logEvent("button_pushed", String("Buzzer activated, password is press button ") + passNumber + String(" times"));

  buzzerActive = true; // Set the buzzer as active
  attempts++;
  while (millis() - startTime < duration) {
    digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
    delayMicroseconds(1000);        // Delay for a short duration
    digitalWrite(buzzerPin, LOW);   // Turn off the buzzer
    delayMicroseconds(1000);        // Delay for the same duration

    bool isFiveTimes = handleButton();  // Check for button presses during buzzer activation
    if(isFiveTimes){
      Blynk.logEvent("button_pushed", String("Password introduced is correct"));
      attempts = 0;
     // Blynk.email("alexandru99d@yahoo.com", "Subject", "Password put corect");
      break;
    }
  }

  buzzerActive = false; // Set the buzzer as inactive
  buttonPressCount = 0; // Reset the button press count
}

bool handleButton() {
  static bool buttonState = HIGH;
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  

  // Read the current button state
  bool reading = digitalRead(buttonPin);

  // Check if the button state has changed
  if (reading != lastButtonState) {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }

  // Check if the button state has been stable for the debounce delay
  if (millis() - lastDebounceTime >= debounceDelay) {
    // Update the button state only if it has changed
    if (reading != buttonState) {
      buttonState = reading;

      // If the button is pressed (LOW) and the buzzer is active, increase the press count
      if (buttonState == LOW && buzzerActive) {
        buttonPressCount++;
        Serial.println(buttonPressCount);


        // If the button has been pressed passNumber or more times, stop the buzzer
        if (buttonPressCount >= passNumber) {
          Serial.println("hereeeee");
          digitalWrite(buzzerPin, LOW);
          buzzerActive = false;
          buttonPressCount = 0;
          return true;
        }
      }
    }
  }

  // Update the last button state
  lastButtonState = reading;
  return false;
}


