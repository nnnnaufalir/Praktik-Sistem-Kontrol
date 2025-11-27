#include <OneWire.h>
#include <DallasTemperature.h>

// --- KONFIGURASI PIN ---
const int oneWireBus = 5;
const int buttonPin = 32;
const int ledPin = 12;
const int relayPin = 25;

// --- KONFIGURASI WAKTU ---
// Ubah angka ini untuk mengatur durasi (bisa desimal, misal 0.5 menit)
const float durasiMenit = 0.5;
const unsigned long heaterDuration = durasiMenit * 60 * 1000;
const unsigned long plotInterval = 500;  // Update grafik tiap 500ms

// --- OBJEK & VARIABEL ---
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

bool heaterState = false;
unsigned long startHeaterTime = 0;
unsigned long lastPlotTime = 0;

void setup() {
  Serial.begin(115200);
  sensors.begin();

  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Inisialisasi Output (Relay Active High sesuai modul)
  digitalWrite(ledPin, LOW);
  digitalWrite(relayPin, LOW);

  // Header opsional untuk Serial Plotter
  Serial.println("Suhu_Air,Status_Heater");
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. LOGIKA INPUT (PRESS-RELEASE) ---
  if (digitalRead(buttonPin) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(buttonPin) == LOW) {
      // Toggle State
      heaterState = !heaterState;

      if (heaterState) {
        startHeaterTime = currentMillis;
        // Feedback Serial (Teks akan muncul di monitor, grafik mungkin skip sebentar)
        Serial.print(">>> START: Pemanasan selama ");
        Serial.print(durasiMenit);
        Serial.println(" Menit.");
      } else {
        Serial.println(">>> STOP: Dimatikan Manual.");
      }

      // Tunggu tombol dilepas agar tidak trigger berulang
      while (digitalRead(buttonPin) == LOW)
        ;
    }
  }

  // --- 2. LOGIKA TIMER (OPEN LOOP) ---
  if (heaterState) {
    // Cek durasi habis
    if (currentMillis - startHeaterTime >= heaterDuration) {
      heaterState = false;
      Serial.println(">>> SELESAI: Waktu Habis.");
    }
  }

  // --- 3. EKSEKUSI OUTPUT ---
  if (heaterState) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(relayPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(relayPin, LOW);
  }

  // --- 4. MONITORING SERIAL PLOTTER ---
  if (currentMillis - lastPlotTime >= plotInterval) {
    lastPlotTime = currentMillis;

    sensors.requestTemperatures();
    float suhu = sensors.getTempCByIndex(0);

    // Filter error sensor
    if (suhu == -127.00) suhu = 0;

    // Format Plotter: "Label:Nilai,Label:Nilai"
    Serial.print("Suhu_Air:");
    Serial.print(suhu);
    Serial.print(",");  // Pemisah

    Serial.print("Status_Heater:");
    // Output 50 agar grafik heater terlihat tinggi di atas grafik suhu
    Serial.println(heaterState ? 50 : 0);
  }
}