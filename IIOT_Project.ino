#define BLYNK_MQTT_BROKER "blynk.cloud"
#define BLYNK_MQTT_PORT 8883
#define WIFI_SSID "FastNuces"
#define WIFI_PASS "fast1234"

#define BLYNK_TEMPLATE_ID "TMPL60Z1Igs7c"
#define BLYNK_TEMPLATE_NAME "IoT Weather Station"
#define BLYNK_AUTH_TOKEN "1nRwRlYSKyeEVzVC6XvHtwut96-p1Diz"

#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DHT.h"
#include "NetworkHelpers.h"

#define DHTPIN D5
#define rainpin A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

int rain;
String rainstatus;
float t, h;
String temp, hum;

void setup() {
  Serial.begin(115200);
  // Wait for serial monitor, up to 3 seconds
  while (!Serial && (millis() < 3000)) {
    delay(10);
  }
  delay(100);
  pinMode(rainpin, INPUT);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.print("  IoT WEATHER");
  lcd.setCursor(0, 1);
  lcd.print("    STATION");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Keep WiFi and MQTT connection
  if (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(1, 0);
    lcd.print("CONNECTING TO");
    lcd.setCursor(3, 1);
    lcd.print("INTERNET");
    connectWiFi();
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("CONNECTED TO");
    lcd.setCursor(3, 1);
    lcd.print("INTERNET");
    delay(2000);
  } else if (!mqtt.connected()) {
    connectMQTT();
  } else {
    mqtt.loop();
  }

  delay(10);
  takedata();
  pub();
}

void mqtt_connected() { Serial.println("MQTT CONNECTED"); }

void mqtt_handler(const String& topic, const String& value) {
  Serial.print("Got ");
  Serial.print(topic);
  Serial.print(", value: ");
  Serial.println(value);
}

void takedata() {
  lcd.clear();
  h = dht.readHumidity();
  t = dht.readTemperature();

  temp = String(t);
  hum = String(h);
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  rain = analogRead(rainpin);
  Serial.println(rain);
  if (rain < 300) {
    rainstatus = "VERY HEAVY RAIN";
  }
  if (rain > 300 && rain <= 450) {
    rainstatus = "HEAVY RAIN";
  }
  if (rain > 450 && rain <= 600) {
    rainstatus = "LIGHT RAIN";
  }
  if (rain > 600) {
    rainstatus = "NO RAIN";
  }
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.print("%");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RAIN  STATUS");
  lcd.setCursor(0, 1);
  lcd.print(rainstatus);
  delay(1000);
}

void pub() {
  mqtt.publish("ds/Rain", rainstatus.c_str());
  mqtt.publish("ds/Temperature", temp.c_str());
  mqtt.publish("ds/Humidity", hum.c_str());
}