#include <Arduino.h>
#include <ESP8266WiFi.h>

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6gdQcodak"                   // Blynk template id
#define BLYNK_TEMPLATE_NAME "ppl2"                          // Blynk template name
#define BLYNK_AUTH_TOKEN "AwRQJZLjIt88wFDR2WtWIV2jFMRSqYv2" // Blynk auth token

#include <BlynkSimpleEsp8266.h> // Board-specific header for ESP8266 Blynk

// WiFi Credentials
// Set password to "" for open networks.
char ssid[] = "vivoY33s";        // WiFi SSID "e.g. WIFINAME"
char pass[] = "8b6j2ejmhs5trj2"; // WiFi PASSWORD "e.g. 12345678"

float get_temperature = 0.0f;
float get_humidity = 0.0f;
float get_co2 = 0.0f;
int get_tempLimit = -1;
boolean tempState = false;

float extractValue(String data, String startDelimiter, String endDelimiter);
void sendToBlynk();
void resetMCU();

void setup()
{
  Serial.begin(9600);  // Initialize Serial Monitor for debugging
  Serial1.begin(9600); // Initialize Serial to receive data from ATmega2560

  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");

  Serial.println("ESP8266 is ready to receive data from ATmega2560.");
}

void loop()
{
  Blynk.run();
  if (Serial1.available())
  {
    String receivedData = Serial1.readString(); // Read the incoming string
    // String receivedData = "Temperature: 23.00 °C, Humidity: 40.00 %, CO2: 59.73 %"; // Read the incoming string

    // Print the received data
    Serial.print("Received data: ");
    Serial.println(receivedData);

    // Now parse the received string to extract temperature, humidity, and CO2 values

    // Example: "Temperature: 25.0 °C, Humidity: 60.0 %, CO2: 450 PPM"

    // Extract temperature
    get_temperature = extractValue(receivedData, "Temperature: ", " °C");
    // Extract humidity
    get_humidity = extractValue(receivedData, "Humidity: ", " %");
    // Extract CO2 concentration
    get_co2 = extractValue(receivedData, "CO2: ", " %");

    // Print the parsed values
    Serial.print("Parsed Temperature: ");
    Serial.println(get_temperature); // Temperature as float
    Serial.print("Parsed Humidity: ");
    Serial.println(get_humidity); // Humidity as float
    Serial.print("Parsed CO2: ");
    Serial.println(get_co2); // CO2 concentration as int

    sendToBlynk();
    delay(500);
  }
}

BLYNK_WRITE(InternalPinDBG)
{
  if (String(param.asStr()) == "reboot")
  {
    Serial.println("Rebooting...");

    // TODO: Perform any neccessary preparation here,
    // i.e. turn off peripherals, write state to EEPROM, etc.

    // NOTE: You may need to defer a reboot,
    // if device is in process of some critical operation.

    resetMCU();
  }
}

// Function to extract a value between two delimiters
float extractValue(String data, String startDelimiter, String endDelimiter)
{
  int startIdx = data.indexOf(startDelimiter) + startDelimiter.length();
  int endIdx = data.indexOf(endDelimiter, startIdx);

  String valueStr = data.substring(startIdx, endIdx); // Extract the value as a string
  return valueStr.toFloat();                          // Convert the string to a float and return
}

void sendToBlynk()
{
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime >= 1000)
  {
    Serial.print("sending data to blynk cloud");
    lastSendTime = millis();

    // if (get_temperature >= 100) get_temperature = 0;
    // if (get_humidity >= 70) get_humidity = 0;
    // if (get_co2 >= 70) get_co2 = 0;

    Blynk.virtualWrite(V0, get_temperature);
    Blynk.virtualWrite(V1, get_humidity);
    Blynk.virtualWrite(V2, get_co2);
    if (get_tempLimit != -1 && get_temperature > get_tempLimit)
    {
      if (!tempState)
      {
        Serial.println("Over Temperature Alert!");
        tempState = true;
      }
    }
    else
    {
      tempState = false;
    }
  }
}

void resetMCU()
{
#if defined(ARDUINO_ARCH_MEGAAVR)
  wdt_enable(WDT_PERIOD_8CLK_gc);
#elif defined(__AVR__)
  wdt_enable(WDTO_15MS);
#elif defined(__arm__)
  NVIC_SystemReset();
#elif defined(ESP8266) || defined(ESP32)
  ESP.restart();
#else
#error "MCU reset procedure not implemented"
#endif
  for (;;)
  {
  }
}