#include <OneWire.h>
#include <DallasTemperature.h>

// --- KONFIGURASI PIN ---
const int oneWireBus = 5;
const int buttonPin = 32;
const int ledPin = 12;
const int relayPin = 25;

// --- PARAMETER KONTROLER ---
float setpoint = 40.0;     
float Kp = 10.0;           // Gain Proporsional

// --- OBJEK & VARIABEL ---
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

bool sistemAktif = false;
unsigned long lastPlotTime = 0;

void setup() {
  Serial.begin(115200);
  sensors.begin();

  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(relayPin, LOW); 
  digitalWrite(ledPin, LOW);

  // Header Serial Plotter (Opsional)
  Serial.println("Setpoint,Suhu_Air,Error_Suhu,Output_P");
}

void loop() {
  unsigned long now = millis();

  // --- 1. LOGIKA TOMBOL (Start/Stop) ---
  if (digitalRead(buttonPin) == LOW) {
    delay(50);
    if (digitalRead(buttonPin) == LOW) {
      sistemAktif = !sistemAktif;
      if (!sistemAktif) {
         digitalWrite(relayPin, LOW); 
         digitalWrite(ledPin, LOW);
      }
      while(digitalRead(buttonPin) == LOW);
    }
  }

  // --- 2. LOOP KONTROL UTAMA ---
  if (now - lastPlotTime >= 500) { // Update setiap 0.5 detik
    lastPlotTime = now;
    
    sensors.requestTemperatures();
    float suhuAktual = sensors.getTempCByIndex(0);
    
    // Filter error sensor
    if (suhuAktual == -127.00) suhuAktual = 0; 

    float error = 0;
    float outputP = 0;

    if (sistemAktif) {
      // --- RUMUS INTI ---
      // 1. Hitung Error (Selisih Setpoint - Suhu)
      error = setpoint - suhuAktual; 

      // 2. Hitung Output P (Aksi Kontrol)
      outputP = Kp * error; 

      // --- LOGIKA AKTUATOR SEDERHANA ---
      // Jika Output P Positif (> 0), artinya Suhu < Setpoint -> Heater NYALA
      // Jika Output P Negatif/Nol (<= 0), artinya Suhu >= Setpoint -> Heater MATI
      
      if (outputP > 0) {
        digitalWrite(relayPin, HIGH); // ON (Sesuaikan jika modul Active Low)
        digitalWrite(ledPin, HIGH);
      } else {
        digitalWrite(relayPin, LOW);  // OFF
        digitalWrite(ledPin, LOW);
      }
    } else {
      // Jika sistem standby, paksa semua nol
      error = 0;
      outputP = 0;
      digitalWrite(relayPin, LOW);
      digitalWrite(ledPin, LOW);
    }

    // --- 3. OUTPUT SERIAL PLOTTER ---
    // Format: Label:Nilai,Label:Nilai
    
    Serial.print("Setpoint:");
    Serial.print(setpoint);
    Serial.print(",");
    
    Serial.print("Suhu_Air:");
    Serial.print(suhuAktual);
    Serial.print(",");
    
    Serial.print("Error_Suhu:"); 
    Serial.print(error); // Ini yang diminta: Selisih Error
    Serial.print(",");
    
    // Kita limit tampilan grafik Output P agar tidak terlalu tinggi (Visual purpose only)
    float displayP = outputP;
    if (displayP > 50) displayP = 50; 
    if (displayP < 0) displayP = 0;
    
    Serial.print("Output_P:");
    Serial.println(displayP); 
  }
}