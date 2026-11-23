
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// WiFi
const char* ssid = "MUSI 9";
const char* password = "NIKKIM123";

// MQTT Broker
const char* mqtt_server = "192.168.1.4";
const int mqtt_port = 1883;

// RFID
#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// Servo
#define SERVO_PIN 13
Servo myServo;

// MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Timer servo
unsigned long closeAt = 0;

// WiFi connection
void setup_wifi() {
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

// Callback ketika menerima perintah dari Python
void callback(char* topic, byte* payload, unsigned int length) {
  String command = "";

  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }

  Serial.print("Perintah diterima: ");
  Serial.println(command);

  if (command == "1") { //dibuang
    myServo.write(180);
    closeAt = millis() + 3000;
    Serial.println("Servo bergerak ke 180 derajat");
  }
  else if (command == "0") { //diterima (stay)
    myServo.write(90);
    closeAt = 0;
    Serial.println("Servo kembali ke 90 derajat");
  }
  else if(command == "-1"){ //ditelan
    myServo.write(0);
    closeAt = millis() + 3000;
    Serial.println("Servo bergerak ke 0 derajat");
  }
}

// MQTT connection
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");

    if (client.connect("ESP32_RFID_Servo")) {
      Serial.println("Connected!");

      client.subscribe("servo/control");
      Serial.println("Subscribed: servo/control");
    } else {
      Serial.print("Failed, state=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // RFID initialization
  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  // Servo initialization
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(90);

  Serial.println("RFID Ready!");
  Serial.println("Servo Ready!");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Automatically return servo after 3 seconds
  if (closeAt != 0 && (long)(millis() - closeAt) >= 0) {
    myServo.write(90);
    closeAt = 0;
    Serial.println("Servo otomatis kembali ke 90 derajat");
  }

  // Read RFID card
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uid += "0";
    }

    uid += String(rfid.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();

  Serial.print("UID Card: ");
  Serial.println(uid);

  if (client.publish("rfid/card", uid.c_str())) {
    Serial.println("UID berhasil dikirim ke MQTT!");
  } else {
    Serial.println("Gagal mengirim UID!");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000);
}
