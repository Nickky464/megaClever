#include <Arduino.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// Pin Definitions
#define MG_PIN A3   // Analog pin for CO2 sensor
#define BOOL_PIN 2  // Pin for additional sensor (unused here)
#define RELAY_PIN 4 // Relay control pin
#define DHT_PIN 2   // DHT sensor pin

// Constants
#define DC_GAIN 8.5              // Amplifier gain
#define READ_SAMPLE_INTERVAL 50  // Sampling interval in ms
#define READ_SAMPLE_TIMES 5      // Number of samples to average
#define ZERO_POINT_VOLTAGE 0.220 // Voltage at 400 PPM CO2
#define REACTION_VOLTAGE 0.020   // Voltage drop at 1000 PPM CO2
#define DHT_TYPE DHT11           // DHT sensor type

// CO2 Calibration
float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, REACTION_VOLTAGE / (2.602 - 3)};
float slope = 2000.0;
float offset = 0.0;

// Variables
float temp_c = 0.0;          // Temperature in Celsius
float humidity = 0.0;        // Humidity percentage
boolean pump_status = false; // Pump status
float CO2Percentage = 0.0;
float co2value = 0.0;

unsigned long pumpLastTurnOnTime = 0;
unsigned long currentTime = 0;
unsigned long LastReadAndSend = 0;

// Objects
DHT dht(DHT_PIN, DHT_TYPE);          // Initialize DHT sensor
SoftwareSerial ESP8266_Serial(6, 7); // ESP8266 communication (RX, TX)

// Function Prototypes
float MGReadCO2(int MG_PIN, float slope, float offset);
int MGGetCO2Percentage(float volts, float *pcurve);

void sendToESP8266(float temp_c, float humidity, int CO2Percentage, boolean pump_status);
void SensorRead();

// Setup function
void setup()
{
  Serial.begin(9600);         // Debug communication
  ESP8266_Serial.begin(9600); // ESP8266 communication
  dht.begin();                // Initialize DHT sensor

  pinMode(RELAY_PIN, OUTPUT); // Set relay pin as output

  Serial.println("Starting communication with ESP8266...");
}

// Main loop
void loop()
{
  currentTime = millis();

  // multitask programing
  SensorRead();
  if (humidity <= 50)
  {
    digitalWrite(RELAY_PIN, HIGH);
    pump_status = true; // Set pump status
    pumpLastTurnOnTime = currentTime;
    Serial.println("Pump turned ON due to low humidity.");
  }
  else if (pump_status && (currentTime - pumpLastTurnOnTime >= 60000) || pump_status && humidity == 60) // 60000ms = 1min.
  {
    digitalWrite(RELAY_PIN, LOW);
    pump_status = false; // Set pump status
    Serial.println("Pump turned OFF after 1-minute delay.");
  }

  if (currentTime - LastReadAndSend >= 15000)
  {
    LastReadAndSend = currentTime;
    // Read sensor
    // Send data to ESP8266
    sendToESP8266(temp_c, humidity, CO2Percentage, pump_status);
  }
}

void SensorRead()
{
  float co2value = MGReadCO2(MG_PIN, slope, offset);
  CO2Percentage = (co2value / 10000) * 100;

  // Read DHT sensor
  temp_c = dht.readTemperature();
  humidity = dht.readHumidity();

  if (CO2Percentage == -1)
  {
    Serial.print("CO2<1%" + String(CO2Percentage));
  }

  if (!isnan(temp_c) && !isnan(humidity))
  {
    Serial.print("Temperature: ");
    Serial.print(temp_c);
    Serial.print(" °C >> Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print(CO2Percentage);

    // Control relay based on humidity
  }
  else
  {
    // temp_c = 9999; // For debug to fixing sensor have a problem.
    // humidity = 9999; // For debug to fixing sensor have a problem.
    Serial.println("Failed to read from DHT sensor!");
  }
}

// Function Definitions
float MGReadCO2(int MG_PIN, float slope, float offset)
{
  int sensorValue = analogRead(MG_PIN);
  float voltage = sensorValue * (5.0 / 1023.0); // Convert to voltage
  return (slope * voltage) + offset;            // Calculate CO2 in PPM
}

void sendToESP8266(float temp_c, float humidity, int CO2Percentage, boolean pump_status)
{
  String message = "Temperature: " + String(temp_c) + " °C, Humidity: " + String(humidity) + " %, CO2: " + String(CO2Percentage) + " %, Pump: " + String(pump_status);
  Serial.println(message);
  ESP8266_Serial.println(message);
}
