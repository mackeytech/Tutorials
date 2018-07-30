/*  This tutorial is based off the Example testing sketch for 
 *   various DHT humidity/temperature sensors Written by ladyada
 *   and the included BasicHttpClient available with the installed
 *   NodeMCU board.
 *   
 *   Before starting this tutorial you should have completed the following
 *   steps:
 *   
 *   1. Installed Arduino IDE 
 *   2. Installed ESP8266 Boards
 *     https://github.com/esp8266/Arduino
 *   3. Installed applicable driver, most NodeMCU boards utilize
 *     CH340 or CP2102 USB drivers. All boards sold by Mackey Tech use
 *     the CP2102 driver. 
 *     https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers
 *   4. Installed the DHT sensor and the Adafruit Unified Sensor libraries by Adafruit
 *   
 *   ****All code edits in program will be marked with the applicable steps
 *   noted below.
 * 
 *   Step 1: Set data pin used to connect DHT sensor module to NodeMCU and
 *           reference a NodeMCU pinout to map correct pin.
 *   
 *   Step 2: Confirm and change the sensor being used as required.
 * 
 *   Step 3: Login in to https://thingspeak.com/ or create an account if you don't have one.
 *           Create a new channel for where you plan to store your sensor data. Make sure you
 *           set it up to have two fields, one for temp and one for humidity(This sketch 
 *           will submit temp to field 1 and the humidity to field 1). Inside your new
 *           channel select the API Keys tabs and find the update a channel feed link. This
 *           is the link we will send the data to. Copy and paste it in the code inside the
 *           quotes. Make sure you remove the 0 after the equal sign at the end. 
 *           ***Also, you will need to remove the 's' from the https so the url is just http
 *   
 *   Step 4: Update the wifi settings, replace the network name and password with the parameters
 *           for the network you would like to connect your device to.
 *   
 *   Step 5: Update how often you would like to send sensor data to thingspeak in milliseconds.
 *           30 seconds would be 30000 and 60 seconds would be 60000 for examples.
 *   
 *   
 *   -----------------------------------------------------------------------------------------------
 *   
 *   Once code had been update, you can upload to your board and watch the serial monitor to see
 *   the the the sensor data and details about each request to udate the thingspeak channel.
 *   
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include "DHT.h"

//--------------
// Step 1
//--------------
#define DHTPIN 4    // D2 // what digital pin we connected to

//--------------
// Step 2
//--------------
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)


ESP8266WiFiMulti WiFiMulti;

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

char bufferRequest[125]; // char array to hold web request URL
char temp[10];           // used to store sensor data as a char array
char humid[10];          // used to store sensor data as a char array


//--------------
// Step 3
//--------------
char getRequest[] = "http://api.thingspeak.com/update?api_key=XXXXXXXXXXXXXXX&field1=";

void setup() {
  Serial.begin(9600);
  Serial.println("DHTxx test!");

  Serial.println();
  Serial.println();
  Serial.println();

  for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
  }

  WiFi.mode(WIFI_STA);

//--------------
// Step 4
//--------------
  WiFiMulti.addAP("Network_Name", "Network_Password");

  dht.begin();
  bufferRequest[0] = '\0';
}

void loop() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  gcvt(f, 4, temp);
  gcvt(h, 4, humid);  

  strcat(bufferRequest, getRequest);
  strcat(bufferRequest, temp);

  strcat(bufferRequest, "&field2=");
  strcat(bufferRequest, humid);

  Serial.println(bufferRequest);

  

  if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        
        http.begin(bufferRequest); 
  
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
    bufferRequest[0] = '\0';

//--------------
// Step 5
//--------------
  delay(30000);  // Time to wait between taking and sending updated sensor data in milliseconds.
}
