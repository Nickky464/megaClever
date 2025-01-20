#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT.h>

// Blynk Credentials
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6gdQcodak"
#define BLYNK_TEMPLATE_NAME "ppl2"
#define BLYNK_AUTH_TOKEN "AwRQJZLjIt88wFDR2WtWIV2jFMRSqYv2"
#include <BlynkSimpleEsp8266.h>

// WiFi Credentials
char ssid[] = "P-E-A-R";  // WiFi SSID "e.g. WIFINAME"
char pass[] = "pnpnpnpn"; // WiFi PASSWORD "e.g. 12345678"

// Sensor and Pin Definitions
#define DHT_PIN D4     // GPIO pin for DHT sensor
#define DHT_TYPE DHT11 // DHT11 sensor type
#define MG_PIN A0      // Analog pin for CO2 sensor
#define RELAY_PIN D1   // GPIO pin for relay control
#define DC_GAIN (8.5)  // Amplifier DC gain

// Calibration Values for CO2 Sensor
#define ZERO_POINT_VOLTAGE (0.220) // Voltage at 400 PPM CO2
#define REACTION_VOLTAGE (0.020)   // Voltage drop at 1000 PPM CO2

DHT dht(DHT_PIN, DHT_TYPE); // Initialize DHT sensor

// Global Variables
float get_temperature = 0.0f;
float get_humidity = 0.0f;
float get_co2 = 0.0f;
float slope = 2000.0;
float offset = 0.0;
float percentage;

void MGReadCO2(int MG_PIN, float slope, float offset, float &ppm); // Modify function prototype
void sendToBlynk();
void readSensors();

float MGReadCO2(int MG_PIN, float slope, float offset); // Function to read CO2 concentration in PPM
float MGRead(int MG_PIN);                               // Function to read raw voltage
int MGGetPercentage(float volts, float *pcurve);        // Function to calculate CO2 percentage
void sendToESP8266(float temp, float hum, int co2);

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize WiFi and Blynk
  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");

  // Initialize DHT sensor and relay pin
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
}

void loop()
{
  Blynk.run(); // Run Blynk

  // Read data from sensors
  readSensors();

  // Send data to Blynk
  sendToBlynk();

  // Control relay based on humidity
  // if (get_humidity <= 70)
  // {
  //   digitalWrite(RELAY_PIN, LOW); // Turn relay off
  // }
  // else
  // {
  //   digitalWrite(RELAY_PIN, HIGH); // Turn relay on
  // }

  delay(10000); // Delay for sensor reading
}

void readSensors()
{
  // Read CO2 sensor
  float co2value = MGReadCO2(MG_PIN, slope, offset);
  float percentage = (co2value / 10000) * 100;
  // MGReadCO2(MG_PIN, slope, offset, get_co2);

  // Read DHT11 sensor
  get_temperature = dht.readTemperature();
  get_humidity = dht.readHumidity();

  // Debugging: Print values to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(get_temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(get_humidity);
  Serial.print(" %, CO2: ");
  Serial.print(percentage);
  Serial.println(" %");
}

float MGReadCO2(int MG_PIN, float slope, float offset)
{
  int sensorValue = analogRead(MG_PIN);         // Read the raw sensor value
  float voltage = sensorValue * (3.3 / 1023.0); // Convert to voltage (assuming 5V reference)

  // Convert voltage to ppm
  float ppm = (slope * voltage) + offset;

  // Convert ppm to percentage
  // float percentage = ppm / 10000.0;

  // Return both values as a struct
  // data = {ppm, percentage};
  return ppm;
}

void sendToBlynk()
{
  Blynk.virtualWrite(V0, get_temperature); // Send temperature to Blynk
  Blynk.virtualWrite(V1, get_humidity);    // Send humidity to Blynk
  Blynk.virtualWrite(V2, percentage);      // Send CO2 ppm to Blynk
}
