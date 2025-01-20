#include <Arduino.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// Pin Definitions
#define MG_PIN A3 // Analog pin for CO2 sensor
#define BOOL_PIN 2 // Pin for additional sensor (unused here)
#define RELAY_PIN 4 // Relay control pin
#define DHT_PIN 2 // DHT sensor pin

// Constants
#define DC_GAIN 8.5 // Amplifier gain
#define READ_SAMPLE_INTERVAL 50 // Sampling interval in ms
#define READ_SAMPLE_TIMES 5 // Number of samples to average
#define ZERO_POINT_VOLTAGE 0.220 // Voltage at 400 PPM CO2
#define REACTION_VOLTAGE 0.020 // Voltage drop at 1000 PPM CO2
#define DHT_TYPE DHT11 // DHT sensor type

// CO2 Calibration
float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, REACTION_VOLTAGE / (2.602 - 3)};
float slope = 2000.0;
float offset = 0.0;

// Variables
float temp_c = 0.0; // Temperature in Celsius
float humidity = 0.0; // Humidity percentage
boolean pump_status = false; // Pump status

// Objects
DHT dht(DHT_PIN, DHT_TYPE); // Initialize DHT sensor
SoftwareSerial ESP8266_Serial(6, 7); // ESP8266 communication (RX, TX)

// Function Prototypes
float MGReadCO2(int MG_PIN, float slope, float offset);
float MGRead(int MG_PIN);
int MGGetPercentage(float volts, float *pcurve);
void sendToESP8266(float temp_c, float humidity, int percentage, boolean pump_status);

// Setup function
void setup() {
  Serial.begin(9600); // Debug communication
  ESP8266_Serial.begin(9600); // ESP8266 communication
  dht.begin(); // Initialize DHT sensor

  pinMode(RELAY_PIN, OUTPUT); // Set relay pin as output

  Serial.println("Starting communication with ESP8266...");
}

// Main loop
void loop() {
  // Read CO2 sensor
  float co2value = MGReadCO2(MG_PIN, slope, offset);
  float percentage = (co2value / 10000) * 100;

  if (percentage == -1) {
    Serial.println("CO2: <400 PPM");
  } else {
    Serial.print("CO2: ");
    Serial.print(co2value);
    Serial.print(" PPM");
    Serial.print(" >> ");
    Serial.print(percentage);
    Serial.println(" %");
  }

  // Read DHT sensor
  temp_c = dht.readTemperature();
  humidity = dht.readHumidity();

  if (!isnan(temp_c) && !isnan(humidity)) {
    Serial.print("Temperature: ");
    Serial.print(temp_c);
    Serial.print(" °C >> Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Control relay based on humidity
    if (humidity <= 70) {
      digitalWrite(RELAY_PIN, HIGH);
      pump_status = true;
    } else {
      digitalWrite(RELAY_PIN, LOW);
      pump_status = false;
    }
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Send data to ESP8266
  sendToESP8266(temp_c, humidity, percentage, pump_status);

  delay(10000); // Wait for 10 seconds
}

// Function Definitions
float MGReadCO2(int MG_PIN, float slope, float offset) {
  int sensorValue = analogRead(MG_PIN);
  float voltage = sensorValue * (5.0 / 1023.0); // Convert to voltage
  return (slope * voltage) + offset; // Calculate CO2 in PPM
}

float MGRead(int MG_PIN) {
  return analogRead(MG_PIN) * (3.3 / 1023.0); // Convert to 3.3V voltage
}

int MGGetPercentage(float volts, float *pcurve) {
  if (volts < pcurve[1]) {
    return -1; // Below valid range
  }
  return int((volts - pcurve[1]) / (pcurve[2] - pcurve[1]) * 100);
}

void sendToESP8266(float temp_c, float humidity, int percentage, boolean pump_status) {
  String message = "Temperature: " + String(temp_c) + " °C, Humidity: " + String(humidity) + " %, CO2: " + String(percentage) + " %, Pump: " + String(pump_status);
  Serial.println(message);
  ESP8266_Serial.println(message);
}
