#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6gdQcodak"                   // Blynk template ID
#define BLYNK_TEMPLATE_NAME "ppl2"                          // Blynk template name
#define BLYNK_AUTH_TOKEN "AwRQJZLjIt88wFDR2WtWIV2jFMRSqYv2" // Blynk auth token

#include <BlynkSimpleEsp8266.h> // Blynk header for ESP8266

// WiFi credentials
char ssid[] = "P-E-A-R";
char pass[] = "pnpnpnpn";

// Variables for sensor data
float get_temperature = 0.0f;
float get_humidity = 0.0f;
float get_co2 = 0.0f;
boolean get_pump_status = false;
int get_tempLimit = -1;
boolean tempState = false;

// multitask variable

unsigned long currentTime = 0;
unsigned long LastSendTime = 0;

// Pin definitions
const int RELAY_PIN = D1;

// SoftwareSerial for MEGA2560 communication
SoftwareSerial MEGA2560_Serial(D2, D3); // (RX, TX)

// NTP client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000);

// Web server setup
ESP8266WebServer server(80);

// Function prototypes
float extractValue(String data, String startDelimiter, String endDelimiter);
void sendToBlynk();
void resetMCU();
void getParsedData();
void handleRoot();
void handleNotFound();
void checkTimeForRollMotor();

// Setup function
void setup()
{
  Serial.begin(9600);
  MEGA2560_Serial.begin(9600);

  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("WiFi connected!");

  timeClient.begin();
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Web server started");
  pinMode(RELAY_PIN, OUTPUT);
}

// Main loop
void loop()
{
  Blynk.run();
  timeClient.update();
  server.handleClient();

  currentTime = millis();

  if (currentTime - LastSendTime >= 15000)
  {
    if (MEGA2560_Serial.available())
    {
      getParsedData();
      sendToBlynk();
    }
  }

  checkTimeForRollMotor();
}

// Blynk write handler
BLYNK_WRITE(InternalPinDBG)
{
  if (String(param.asStr()) == "reboot")
  {
    Serial.println("Rebooting...");
    resetMCU();
  }
}

// Extract a value from a delimited string
float extractValue(String data, String startDelimiter, String endDelimiter)
{
  int startIdx = data.indexOf(startDelimiter) + startDelimiter.length();
  int endIdx = data.indexOf(endDelimiter, startIdx);
  return data.substring(startIdx, endIdx).toFloat();
}

// Send data to Blynk
void sendToBlynk()
{
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime >= 1000)
  {
    lastSendTime = millis();
    Blynk.virtualWrite(V0, get_temperature);
    Blynk.virtualWrite(V1, get_humidity);
    Blynk.virtualWrite(V2, get_co2);

    if (get_tempLimit != -1 && get_temperature > get_tempLimit && !tempState)
    {
      Serial.println("Over Temperature Alert!");
      tempState = true;
    }
    else
    {
      tempState = false;
    }
  }
}

// Reset the MCU
void resetMCU()
{
#if defined(ESP8266) || defined(ESP32)
  ESP.restart();
#else
  for (;;)
  {
  }
#endif
}

// Parse incoming data from MEGA2560
void getParsedData()
{
  String receivedData = MEGA2560_Serial.readString();
  Serial.print("Received data: ");
  // Temperature: 23.5 °C, Humidity: 60 %, CO2: 26 %, Pump: True
  Serial.println(receivedData);

  get_temperature = extractValue(receivedData, "Temperature: ", " °C");
  get_humidity = extractValue(receivedData, "Humidity: ", " %");
  get_co2 = extractValue(receivedData, "CO2: ", " %");

  String pumpStatusStr = receivedData.substring(receivedData.indexOf("Pump: ") + 6); // Get the part after "Pump: "
  if (pumpStatusStr == "True")
  {
    get_pump_status = true; // Pump is ON
  }
  else if (pumpStatusStr == "False")
  {
    get_pump_status = false; // Pump is OFF
  }

  Serial.print("Temperature: ");
  Serial.println(get_temperature);
  Serial.print("Humidity: ");
  Serial.println(get_humidity);
  Serial.print("CO2: ");
  Serial.println(get_co2);
}

// Handle root web page
void handleRoot()
{
  String html = "<html><body>";
  html += "<h1>Sensor Data</h1>";
  html += "<p>Temperature: " + String(get_temperature) + " &deg;C</p>";
  html += "<p>Humidity: " + String(get_humidity) + " %</p>";
  html += "<p>CO2: " + String(get_co2) + " %</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Handle 404 errors
void handleNotFound()
{
  server.send(404, "text/plain", "404: Not Found");
}

// Check time and control relay
void checkTimeForRollMotor()
{
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  Serial.println(currentHour + currentMinute);

  if (((currentHour >= 6 && currentHour <= 8) && (currentHour != 8 || currentMinute == 0)) || ((currentHour >= 18 && currentHour <= 20) && (currentHour != 20 || currentMinute == 0)))
  {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Relay ON: 6:00 - 8:00 UTC+7");
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Relay OFF");
  }
}
