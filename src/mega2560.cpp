#include <Arduino.h>
#include <DHT.h>

// Define which analog input channel you are going to use
#define MG_PIN (A3)
#define BOOL_PIN (2)
#define DC_GAIN (8.5) // Define the DC gain of amplifier

/***********************Software Related Macros************************************/
#define READ_SAMPLE_INTERVAL (50) // Sample interval in milliseconds
#define READ_SAMPLE_TIMES (5)     // Number of samples to average

/**********************Application Related Macros**********************************/
// These two values differ from sensor to sensor. User should determine this value.
#define ZERO_POINT_VOLTAGE (0.220) // Voltage at 400 PPM CO2
#define REACTION_VOLTAGE (0.020)   // Voltage drop at 1000 PPM CO2

// CO2 Read
float slope(2000.0);
float offset(0.0);

// DHT
#define DHT_PIN (2)
#define DHT_TYPE (DHT11)

DHT dht(DHT_PIN, DHT_TYPE); // Initialize the DHT sensor
float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))};

// Declare variables
float temp_c;
float humidity;
int relay1 = 4; // Relay pin
boolean tempState = false;

float get_temperature = 0.0f;
float get_humidity = 0.0f;
int get_tempLimit = -1;

// Declare function prototypes
float MGReadCO2(int MG_PIN, float slope, float offset); // Function to read CO2 concentration in PPM
float MGRead(int MG_PIN);                               // Function to read raw voltage
int MGGetPercentage(float volts, float *pcurve);        // Function to calculate CO2 percentage
void sendToESP8266(float temp, float hum, int co2);

void setup()
{
  // Start Serial communication with the computer (for debugging)
  Serial.begin(9600);

  // Start Serial3 communication with the ESP8266
  Serial3.begin(9600); // Set the baud rate to communicate with ESP8266
  dht.begin();
  Serial.println("Starting communication with ESP8266...");
}

void loop()
{
  // Serial.print("ESP8266 Board MAC Address:  ");

  // Read CO2 sensor
  float co2value = MGReadCO2(MG_PIN, slope, offset);
  float percentage = (co2value / 10000) * 100;

  Serial.print("CO2: ");
  if (percentage == -1)
  {
    Serial.println("<400 PPM");
  }
  else
  {
    Serial.print(co2value);
    Serial.print(" PPM");
    Serial.print("CO2: ");
    Serial.print(percentage);
    Serial.println(" %");
  }

  // Read DHT sensor
  temp_c = dht.readTemperature();
  humidity = dht.readHumidity();

  if (!isnan(temp_c) && !isnan(humidity))
  {
    Serial.print("Temperature: ");
    Serial.print(temp_c);
    Serial.print(" °C >> Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Send CO2, Temperature, and Humidity to ESP8266
    sendToESP8266(temp_c, humidity, percentage);

    // Control relay based on humidity
    digitalWrite(relay1, humidity <= 70 ? LOW : HIGH);
  }
  else
  {
    Serial.println("Failed to read from DHT sensor!");
  }

  delay(1000);
}

// Define the function to read CO2 concentration
float MGReadCO2(int MG_PIN, float slope, float offset)
{
  int sensorValue = analogRead(MG_PIN);         // Read the raw sensor value
  float voltage = sensorValue * (5.0 / 1023.0); // Convert to voltage (assuming 5V reference)

  // Convert voltage to ppm
  float ppm = (slope * voltage) + offset;

  // Convert ppm to percentage
  // float percentage = ppm / 10000.0;

  // Return both values as a struct
  // data = {ppm, percentage};
  return ppm;
}

// Read voltage from the analog sensor pin
float MGRead(int mg_pin)
{
  return analogRead(mg_pin) * (3.3 / 1023.0); // For 3.3V ESP32
}

// Calculate CO2 concentration percentage based on the voltage and calibration curve
int MGGetPercentage(float volts, float *pcurve)
{
  if (volts < pcurve[1])
  {
    return -1; // Below valid range
  }
  return int((volts - pcurve[1]) / (pcurve[2] - pcurve[1]) * 100);
}

// Send Temperature, Humidity, and CO2 data to the ESP8266
void sendToESP8266(float temp_c, float humidity, int percentage)
{
  // Create a formatted message to send
  String message = "Temperature: " + String(temp_c) + " °C, Humidity: " + String(humidity) + " %, CO2: " + String(percentage) + " %";

  // Send the message over Serial3 (to ESP8266)
  Serial.println(message);
  Serial3.println(message);
}
