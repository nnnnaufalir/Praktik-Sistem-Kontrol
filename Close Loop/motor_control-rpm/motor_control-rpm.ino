// --- PIN DEFINITIONS ---
const int enAPin = 13; 
const int in1Pin = 26;
const int in2Pin = 25;
const int encoderPinA = 32;
const int encoderPinB = 33;

const int pwmFreq = 5000;
const int pwmResolution = 10; // 0 - 1023

// --- PARAMETER MOTOR ---
// Cari tahu PPR (Pulse Per Revolution) motor Anda.
// Misal PG45 rasio 1:100 biasanya sekitar 300-700 pulse per putaran poros output.
float PPR = 360.0; 

// --- PARAMETER KONTROL ---
float targetRPM = 120.0; // Target kecepatan (RPM)
float Kp = 35;         // Gain P untuk RPM biasanya lebih besar

int minPWM = 280;       // Deadzone Compensation (Hasil tes sebelumnya)

// --- VARIABEL SYSTEM ---
volatile long encoderCount = 0;
long lastEncoderCount = 0;
unsigned long lastTime = 0;
float currentRPM = 0.0;

void setup() {
  Serial.begin(115200);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  
  ledcAttach(enAPin, pwmFreq, pwmResolution);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), readEncoder, RISING);

  Serial.println("TargetRPM,CurrentRPM,Error,PWM_Output");
}

void loop() {
  // Update Target via Serial
  if (Serial.available() > 0) {
    float input = Serial.parseFloat();
    if (input > 0) targetRPM = input;
  }

  unsigned long now = millis();

  // --- 1. HITUNG RPM (Sampling setiap 100ms) ---
  // Jangan sampling terlalu cepat (misal 10ms) karena resolusi RPM jadi kasar
  if (now - lastTime >= 100) { 
    
    // Hitung berapa pulse terjadi dalam 100ms
    long deltaPulse = encoderCount - lastEncoderCount;
    lastEncoderCount = encoderCount;
    
    // Rumus: (Pulse / PPR) * (60000ms / SelangWaktu)
    // deltaPulse diambil nilai mutlak (abs) karena RPM selalu positif
    currentRPM = (abs(deltaPulse) / PPR) * (60000.0 / (now - lastTime));
    lastTime = now;

    // --- 2. KONTROLER P UNTUK RPM ---
    
    float error = targetRPM - currentRPM;
    
    // Hitung Sinyal Kontrol
    int drivePWM = 0;
    
    // Hanya aktifkan motor jika target > 0
    if (targetRPM > 0) {
      // Feedforward (minPWM) + Feedback (Error * Kp)
      // Artinya: "Minimal jalan dulu (280), baru tambah tenaga sesuai error"
      drivePWM = minPWM + (error * Kp);
      
      // Batasi Max
      if (drivePWM > 1023) drivePWM = 1023;
      // Jangan biarkan turun di bawah minPWM jika error masih positif
      if (drivePWM < minPWM) drivePWM = minPWM; 
    } else {
      drivePWM = 0;
    }

    // --- 3. EKSEKUSI MOTOR ---
    // Untuk RPM Control, kita asumsikan putaran 1 arah dulu (CW)
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    ledcWrite(enAPin, drivePWM);

    // --- 4. SERIAL PLOTTER ---
    Serial.print("TargetRPM:"); Serial.print(targetRPM); Serial.print(",");
    Serial.print("CurrentRPM:"); Serial.print(currentRPM); Serial.print(",");
    Serial.print("Error:"); Serial.print(error); Serial.print(",");
    Serial.print("PWM_Output:"); Serial.println(drivePWM);
  }
}

void readEncoder() {
  if (digitalRead(encoderPinB) > 0) encoderCount++;
  else encoderCount--;
}