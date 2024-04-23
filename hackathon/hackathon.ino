#include "DHT.h"
#include <ESP8266WiFi.h> // Make sure you have this line 
#include <WiFiClientSecure.h>
#define DHTPIN D2 // Pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11 sensor type
#define LDR_PIN A0
#define GREEN_LED D3 // Green LED connected to pin D3
#define RED_LED D4  // Red LED connected to pin D4

DHT dht(DHTPIN, DHTTYPE); // Create a DHT object

int readingsCount = 0; // Counter for the number of readings
float temperatureSum = 0.0; // Sum of temperature readings
float humiditySum = 0.0; // Sum of humidity readings

const char* ssid = "Unifil Computacao"; // Replace with your WiFi SSID
const char* password = ""; // Replace with your WiFi password
const char* host = "script.google.com";  
const char* GAS_ID = "AKfycbwORgSGCvWgrmEcuwJquIFbM9PAylcLQyT5OW-xe3gnUh0gGSk8QuhT5_J9vlmvK-h-Mg"; // Your Google Apps Script ID
void setup() {
  
  Serial.begin(9600); // Initialize serial communication
  Serial.println("DHTxx test!"); // Print a message

  dht.begin(); // Initialize the DHT sensor
  pinMode(GREEN_LED, OUTPUT); // Set Green LED pin as output
  pinMode(RED_LED, OUTPUT);  // Set Red LED pin as output
}

void loop() {
  // Read temperature and humidity from the DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int brightnessValue = analogRead(LDR_PIN);

  float brightnessPercentage = map(brightnessValue, 0, 1024, 0, 100);

  // Check if the readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT");
  } else {
    // Print the current temperature and humidity readings
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
    Serial.print("Brightness: ");
    Serial.print(brightnessPercentage);
    Serial.println("%");
    Serial.println(brightnessValue);
    String data = String("Humidity=") + humidity + "&Temperature=" + temperature + "&Brightness=" + brightnessPercentage;
    update_google_sheet(data); 

    // Calculate the average temperature and humidity after 10 readings
    if (temperature <= 26) {
        digitalWrite(GREEN_LED, HIGH); // Turn on Green LED
        digitalWrite(RED_LED, LOW);  // Turn off Red LED
      } else {
        digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
        digitalWrite(RED_LED, HIGH);   // Turn on Red LED
      }
  }
  
  delay(2000); // Delay between readings (in milliseconds)
}

void update_google_sheet(String data) {
  Serial.print("Connecting to ");
  Serial.println(host);

  // Connect to WiFi
  Serial.printf("Connecting to WiFi network %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500); 
  }
  Serial.println(" connected successfully");

  // Use WiFiClientSecure to establish HTTPS for Google Sheets
  WiFiClientSecure client;
  const int httpPort = 443; // HTTPS port 

  client.setInsecure(); // (NOTE: Consider using full certificate verification in production)

  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return; 
  }

  String url = "/macros/s/" + String(GAS_ID) + "/exec?" + data;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // Send HTTP GET request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(host) + "\r\n" +
               "Connection: close\r\n\r\n");

  // Troubleshooting: Read and print the response
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.println(line); 
  }
}
