#include <FB_Const.h>
#include <FB_Error.h>
#include <FB_Network.h>
#include <FB_Utils.h>
#include <Firebase.h>
#include <FirebaseFS.h>
#include <Firebase_ESP_Client.h>
#include <MB_NTP.h>

#include <Arduino.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#include <Adafruit_Sensor.h>
#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#include <DHT_U.h>
#include <ESP8266WiFi.h>



#define DHTPIN D2
#define DHTTYPE DHT11   // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE); 

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Sedic_Home"
#define WIFI_PASSWORD "1999s2003"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "rM9RbWylnw5gnknjT7pjfxmfou5vqdHdFM5dfHex"

/* 3. Define the RTDB URL */
#define DATABASE_URL "fireplace-control-default-rtdb.europe-west1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "elninoever0@gmail.com"
#define USER_PASSWORD "fernandotorres"

#define LED D0            // Led in NodeMCU at pin GPIO16 (D0).
// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

uint32_t delayMS;


void setup(void)
{ 
  Serial.begin(115200);
  Serial.println("Humidity and temperature\n\n");
  
  pinMode(LED, OUTPUT);    // LED pin as output.

  // Initialize device.
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
// Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  //config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  //config.database_url = DATABASE_URL;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  

  /* Assign the callback function for the long running token generation task */
  //config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = "rM9RbWylnw5gnknjT7pjfxmfou5vqdHdFM5dfHex";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

#if defined(ESP8266)
  // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;

  /** Timeout options.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  Note:
  The function that starting the new TCP session i.e. first time server connection or previous session was closed, the function won't exit until the
  time of config.timeout.socketConnection.

  You can also set the TCP data sending retry with
  config.tcp_data_sending_retry = 1;

  */



  

  // connect to wifi. 
  /*
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.print("connecting"); 
  while (WiFi.status() != WL_CONNECTED) { 
    Serial.print("."); 
    delay(500); 
  } 
  Serial.println(); 
  Serial.print("connected: "); 
  Serial.println(WiFi.localIP()); 
   
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
*/
}

int n = 0;

void loop() {

/*if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
*/
    Serial.printf("Get bool... %s\n", Firebase.RTDB.getBool(&fbdo, FPSTR("Dioda/Ukljucena")) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());
#define vrijednost
 Firebase.RTDB.getBool(&fbdo, FPSTR("Dioda/Ukljucena"));
if(fbdo.to<bool>() == false){
  digitalWrite(LED, HIGH);// turn the LED off.(Note that LOW is the voltage level but actually 
                        //the LED is on; this is because it is acive low on the ESP8266.
//delay(1000);            // wait for 1 second.

}
else{
digitalWrite(LED, LOW); // turn the LED on.
//delay(1000); // wait for 1 second. 

}

  // Delay between measurements.
  delay(delayMS);

// Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    if(Firebase.ready()){
      Serial.printf("Set float... %s\n", Firebase.RTDB.setFloat(&fbdo, F("/Mjerenje/TemperaturaZraka"), event.temperature) ? "ok" : fbdo.errorReason().c_str());

    }

  }

// Delay between measurements.
  delay(delayMS);

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    if(Firebase.ready()){
      Serial.printf("Set float... %s\n", Firebase.RTDB.setFloat(&fbdo, F("/Mjerenje/VlaznostZraka"), event.relative_humidity) ? "ok" : fbdo.errorReason().c_str());

    }
  }

 // }


// set value 
/*
  float temp = dht.readTemperature();
  Serial.print("temperature = ");
  Serial.print(temp); 
  Serial.println("C  ");
  Firebase.setFloat("Mjerenje/TemperaturaZraka", temp); 
  // handle error 
  if (Firebase.failed()) { 
      Serial.print("Writing failed:"); 
      Serial.println(Firebase.error());   
      return; 
  }

  float vlaznost = dht.readHumidity();
      Serial.print("Current humidity = ");
      Serial.print(vlaznost);
      Serial.print("%  ");
      Firebase.setFloat("Mjerenje/VlaznostZraka", vlaznost); 
  // handle error 
  if (Firebase.failed()) { 
      Serial.print("Writing failed:"); 
      Serial.println(Firebase.error());   
      return; 
  }  
   
  delay(1000); */
}    
    