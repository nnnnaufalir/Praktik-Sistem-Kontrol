const int buttonPin = 32;
const int ledPin = 12;
const int relayHeater = 25;

bool actionState = false;
#define ACTION_DURATION 5000

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(relayHeater, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);
  digitalWrite(relayHeater, LOW);
  Serial.println("Led & Heater MATI");
}

void loop() {
  if (digitalRead(buttonPin) == 0) {
    delay(10);
    if (digitalRead(buttonPin) == 1) {
      Serial.println("Button AKTIF");
      actionState = true;
    }
  }

  if (actionState == true) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(relayHeater, HIGH);
    Serial.println("Led & Heater HIDUP");
    delay(ACTION_DURATION);
    digitalWrite(ledPin, LOW);
    digitalWrite(relayHeater, LOW);
    Serial.println("Led & Heater MATI");
    actionState = false;
  }
}
