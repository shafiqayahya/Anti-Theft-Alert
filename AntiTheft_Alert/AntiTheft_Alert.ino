#define BLYNK_TEMPLATE_ID "TMPL63l5ZilUi"
#define BLYNK_TEMPLATE_NAME "SECURE TABUNG FOR MOSQUE FUNDS"
#define BLYNK_AUTH_TOKEN "kjcouxuMFbwreEP8CUokmHEgOPtUlxbt"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi credentials
char ssid[] = "Mar_2G";         
char pass[] = "0136683747";

// Pin configuration
#define vibSensor   D6    // GPIO12
#define trigPin     D5    // GPIO14
#define echoPin     D2    // GPIO4
#define ledPin      D4    // GPIO2
#define buzzerPin   D7    // GPIO13

unsigned long lastVibrationTime = 0; // untuk cooldown
const unsigned long vibrationCooldown = 5000; // 5 saat

bool detectVibration() {
  int count = 0;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(vibSensor) == LOW) {
      count++;
    }
    delay(10);
  }
  return count >= 2;
}

void connectToWiFi() {
  Serial.print("Menyambung ke WiFi: ");
  Serial.println(ssid);
  WiFi.disconnect();
  delay(500);
  WiFi.begin(ssid, pass);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✔ WiFi Disambung!");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Gagal sambung WiFi. Semak SSID/katalaluan.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(vibSensor, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Serial.println("ESP8266 Alert System (Active Buzzer Mode)");

  connectToWiFi();

  Blynk.config(BLYNK_AUTH_TOKEN);
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.connect(5000);
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if (!Blynk.connected()) {
    Blynk.connect(5000);
  }

  if (!Blynk.connected()) {
    Serial.println("⚠ Blynk tidak disambung.");
    return;
  }

  Blynk.run();

  bool vibStatus = detectVibration();

  long duration;
  float distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  distance = (duration == 0) ? 999 : duration * 0.034 / 2;

  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.print(" cm | Getaran: ");
  Serial.println(vibStatus ? "YA" : "TIDAK");

  // Hantar ke Blynk
  Blynk.virtualWrite(V0, distance);
  Blynk.virtualWrite(V1, vibStatus ? 1 : 0);

  // LED based on jarak (jarak sah sahaja)
  if (distance != 999 && distance <= 10.0) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  // Getaran dikesan
  if (vibStatus && millis() - lastVibrationTime > vibrationCooldown) {
    lastVibrationTime = millis();

    // Beri amaran buzzer dan LED
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      digitalWrite(buzzerPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
      digitalWrite(buzzerPin, LOW);
      delay(200);
    }

    // Hantar notifikasi ke Blynk (pastikan logEvent sudah di setup dalam app)
    Blynk.logEvent("getaran_dikesan", "Getaran dikesan pada tabung masjid!");
  } else {
    digitalWrite(buzzerPin, HIGH);
  }

  // Status LED & Buzzer ke Blynk
  Blynk.virtualWrite(V2, digitalRead(ledPin));
  Blynk.virtualWrite(V3, digitalRead(buzzerPin));

  delay(500);
}

