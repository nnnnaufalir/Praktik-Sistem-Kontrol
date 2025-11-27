// --- KONFIGURASI PIN & PWM ---
const int enAPin = 13;
const int in1Pin = 26;
const int in2Pin = 25;
const int encoderPinA = 32;
const int encoderPinB = 33;

const int pwmFreq = 5000;
const int pwmResolution = 10; // 0 - 1023

// --- PARAMETER MOTOR & KONTROL ---
long setpoint = 1000;
float Kp = 10;

// [BARU] Deadzone Compensation
// Anda menyebutkan 273 motor berdengung. Kita set minimal gerak di 280 agar pasti muter.
const int minPWM = 280; 

// --- VARIABEL SISTEM ---
volatile long currentPosition = 0;
unsigned long lastPlotTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  
  // Setup PWM (ESP32 v3.0+)
  ledcAttach(enAPin, pwmFreq, pwmResolution);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), readEncoder, RISING);

  Serial.println("Setpoint,Posisi,Error,PWM_Output");
}

void loop() {
  if (Serial.available() > 0) {
    long input = Serial.parseInt();
    if (input != 0) setpoint = input;
  }

  // --- LOOP KONTROL ---
  unsigned long now = millis();
  
  long error = setpoint - currentPosition;

  // Rumus P Murni
  int controlSignal = error * Kp;

  // Batasi Max PWM (1023)
  if (controlSignal > 1023) controlSignal = 1023;
  if (controlSignal < -1023) controlSignal = -1023;
  
  // Kirim ke Driver
  driveMotor(controlSignal);

  // Plotting
  if (now - lastPlotTime >= 50) {
    lastPlotTime = now;
    Serial.print("Setpoint:"); Serial.print(setpoint); Serial.print(",");
    Serial.print("Posisi:"); Serial.print(currentPosition); Serial.print(",");
    Serial.print("Error:"); Serial.print(error); Serial.print(",");
    Serial.print("PWM_Output:"); Serial.println(controlSignal);
  }
}

void readEncoder() {
  if (digitalRead(encoderPinB) > 0) currentPosition++;
  else currentPosition--;
}

// --- FUNGSI DRIVE MOTOR DENGAN DEADZONE FIX ---
void driveMotor(int pwmVal) {
  int absPWM = abs(pwmVal);

  // [LOGIKA BARU] Deadzone Compensation
  // Jika PWM tidak nol TAPI kurang dari batas minimal gerak -> Paksa naik ke minPWM
  if (absPWM > 0 && absPWM < minPWM) {
    absPWM = minPWM; 
  }
  
  // Jika error sangat kecil (misal cuma beda 1-2 pulse), matikan saja biar gak dengung (Opsional)
  // if (absPWM > 0 && absPWM < 20) absPWM = 0; 

  if (pwmVal > 0) {
    // Putar Kanan (CW)
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    ledcWrite(enAPin, absPWM); 
  } 
  else if (pwmVal < 0) {
    // Putar Kiri (CCW)
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
    ledcWrite(enAPin, absPWM); 
  } 
  else {
    // Stop
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    ledcWrite(enAPin, 0);
  }
}