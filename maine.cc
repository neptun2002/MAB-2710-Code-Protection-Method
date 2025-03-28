/*
The MAB-2710 code protection method provides a basic level of authorization by linking the 
firmware execution to the unique hardware characteristics of the ESP8266 or ESP32 device.
 While it offers a degree of protection against simple cloning, it's important to understand its
 limitations, particularly the lack of strong cryptographic security. For applications requiring 
higher levels of security, more robust techniques such as flash encryption and secure boot 
(available on ESP32) should be considered in conjunction with or as an alternative to this 
method. The MAB-2710 method serves as a custom, device-specific locking mechanism 
designed by E. Mohammed Babelly, offering a balance of simplicity and a basic level of code
protection.
*/

//=========================================(Start Of Function)========================================
#if defined(ESP32)
  #include <EEPROM.h>
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <EEPROM.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#endif

// Define EEPROM size (adjust as needed)
#define EEPROM_SIZE 512

// EEPROM Addresses
#define SECURITY_CODE_ADDR 0
#define WIFI_SSID_ADDR 64
#define WIFI_PASS_ADDR 96
#define MQTT_SERVER_ADDR 128
#define MQTT_TOPIC_ADDR 192

// Lengths
#define MAX_SECURITY_CODE_LEN 128 // Adjust based on the length of the generated code
#define MAX_WIFI_SSID_LEN 32
#define MAX_WIFI_PASS_LEN 32
#define MAX_MQTT_SERVER_LEN 64
#define MAX_MQTT_TOPIC_LEN 64

// Global variables
char storedSSID[MAX_WIFI_SSID_LEN + 1] = "";
char storedPassword[MAX_WIFI_PASS_LEN + 1] = "";
char storedMqttServer[MAX_MQTT_SERVER_LEN + 1] = "";
char storedMqttTopic[MAX_MQTT_TOPIC_LEN + 1] = "";
char storedSecurityCode[MAX_SECURITY_CODE_LEN + 1] = "";

AsyncWebServer server(80); //Port = 80

// Function prototypes
void checkAuthorization();
void loadCredentials();
void saveCredentials();
void startAPMode();
void connectWiFi();

String getChipSecurityCode();
String getChipID();
String getMacAddress();
String generateConfigPage();
void writeSecurityCodeToEEPROM(const String& code);







//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================


void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting ESP...");

  checkAuthorization();

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Load WiFi credentials from EEPROM
  loadCredentials();

  // Attempt to connect to WiFi
  connectWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to saved WiFi. Starting AP mode.");
    startAPMode();
  } else {
    Serial.print("Connected to WiFi: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // You can add further logic here if WiFi connection is successful
  }

  // Start the web server
  server.begin();
  Serial.println("Web server started.");
}
//=========================================(Start Of Function)========================================







//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================

void loop() {
  // Main loop is empty, server handles requests and authorization handles endless loop.
}

//=========================================(Start Of Function)========================================









//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================

void checkAuthorization() {
  Serial.println("Checking Authorization (MAB-2710)...");
  String securityChipCode = getChipSecurityCode();
  Serial.print("Generated Security Code: ");
  Serial.println(securityChipCode);

  char eepromSecurityCode[MAX_SECURITY_CODE_LEN + 1] = "";
  for (int i = 0; i < securityChipCode.length(); ++i) {
    eepromSecurityCode[i] = EEPROM.read(SECURITY_CODE_ADDR + i);
  }
  eepromSecurityCode[securityChipCode.length()] = '\0';
  Serial.print("EEPROM Security Code: ");
  Serial.println(eepromSecurityCode);

  if (securityChipCode.equals(eepromSecurityCode)) {
    Serial.println("Authorization successful!");
    return;
  } else {
    Serial.println("Authorization failed!");
    Serial.println("Entering endless loop. Send 'Get_Chip_To_Unlock' to unlock.");
    while (true) {
      if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        Serial.print("Received command: ");
        Serial.println(command);
        if (command.equals("Get_Chip_To_Unlock")) {
          Serial.println("Unlocking chip and restarting...");
          writeSecurityCodeToEEPROM(securityChipCode);
          delay(100);
          ESP.restart();
        } else {
          Serial.println("Invalid command. Resetting...");
          delay(100);
          ESP.restart();
        }
      }
      delay(100);
    }
  }
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
void writeSecurityCodeToEEPROM(const String& code) {
  for (int i = 0; i < code.length(); ++i) {
    EEPROM.write(SECURITY_CODE_ADDR + i, code.charAt(i));
  }
  EEPROM.commit();
  Serial.println("Security Code written to EEPROM.");
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
void loadCredentials() {
  Serial.println("Loading credentials from EEPROM...");
  for (int i = 0; i < MAX_WIFI_SSID_LEN; ++i) storedSSID[i] = EEPROM.read(WIFI_SSID_ADDR + i);
  storedSSID[MAX_WIFI_SSID_LEN] = '\0';
  for (int i = 0; i < MAX_WIFI_PASS_LEN; ++i) storedPassword[i] = EEPROM.read(WIFI_PASS_ADDR + i);
  storedPassword[MAX_WIFI_PASS_LEN] = '\0';
  for (int i = 0; i < MAX_MQTT_SERVER_LEN; ++i) storedMqttServer[i] = EEPROM.read(MQTT_SERVER_ADDR + i);
  storedMqttServer[MAX_MQTT_SERVER_LEN] = '\0';
  for (int i = 0; i < MAX_MQTT_TOPIC_LEN; ++i) storedMqttTopic[i] = EEPROM.read(MQTT_TOPIC_ADDR + i);
  storedMqttTopic[MAX_MQTT_TOPIC_LEN] = '\0';

  Serial.print("Loaded SSID: ");
  Serial.println(storedSSID);
  Serial.print("Loaded Password: ");
  Serial.println(storedPassword);
  Serial.print("Loaded MQTT Server: ");
  Serial.println(storedMqttServer);
  Serial.print("Loaded MQTT Topic: ");
  Serial.println(storedMqttTopic);
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
void saveCredentials() {
  Serial.println("Saving credentials to EEPROM...");
  for (int i = 0; i < MAX_WIFI_SSID_LEN; ++i) EEPROM.write(WIFI_SSID_ADDR + i, storedSSID[i]);
  for (int i = 0; i < MAX_WIFI_PASS_LEN; ++i) EEPROM.write(WIFI_PASS_ADDR + i, storedPassword[i]);
  for (int i = 0; i < MAX_MQTT_SERVER_LEN; ++i) EEPROM.write(MQTT_SERVER_ADDR + i, storedMqttServer[i]);
  for (int i = 0; i < MAX_MQTT_TOPIC_LEN; ++i) EEPROM.write(MQTT_TOPIC_ADDR + i, storedMqttTopic[i]);

  EEPROM.commit();
  Serial.println("Credentials saved successfully!");
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
void connectWiFi() {
  Serial.println("Attempting to connect to WiFi...");
  WiFi.begin(storedSSID, storedPassword);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) { // Try for a few seconds
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  Serial.println("");
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
void startAPMode() {
  String apName = getMacAddress() + getChipID();
  String apPassword = getChipID();

  Serial.print("Starting Access Point: ");
  Serial.println(apName);
  Serial.print("Password: ");
  Serial.println(apPassword);

  WiFi.softAP(apName.c_str(), apPassword.c_str());
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // Set up web server routes for AP mode
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", generateConfigPage());
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true)) {
      String ssid = request->getParam("ssid", true)->value();
      ssid.toCharArray(storedSSID, MAX_WIFI_SSID_LEN + 1);
    }
    if (request->hasParam("password", true)) {
      String password = request->getParam("password", true)->value();
      password.toCharArray(storedPassword, MAX_WIFI_PASS_LEN + 1);
    }
    if (request->hasParam("mqtt_server", true)) {
      String mqtt_server = request->getParam("mqtt_server", true)->value();
      mqtt_server.toCharArray(storedMqttServer, MAX_MQTT_SERVER_LEN + 1);
    }
    if (request->hasParam("mqtt_topic", true)) {
      String mqtt_topic = request->getParam("mqtt_topic", true)->value();
      mqtt_topic.toCharArray(storedMqttTopic, MAX_MQTT_TOPIC_LEN + 1);
    }
    saveCredentials();
    request->send(200, "text/html", "Configuration saved. Restarting...");
    delay(2000);
    ESP.restart();
  });
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
String getChipSecurityCode() {
  String mac = getMacAddress();
#if defined(ESP32)
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xFF) << i;
  }
  uint32_t flashId = ESP.getFlashChipId();
  uint32_t flashSpeed = ESP.getFlashChipSpeed();
  return mac + String(chipId) + String(flashId) + String(flashSpeed);
#elif defined(ESP8266)
  uint32_t chipId = ESP.getChipId();
  uint32_t flashId = ESP.getFlashChipId();
  uint32_t flashSpeed = ESP.getFlashChipSpeed();
  return mac + String(chipId) + String(flashId) + String(flashSpeed);
#endif
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
String getChipID() {
#if defined(ESP32)
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xFF) << i;
  }
  return String(chipId);
#elif defined(ESP8266)
  return String(ESP.getChipId());
#endif
}

String getMacAddress() {
  return WiFi.macAddress();
}
//=========================================(Start Of Function)========================================

//====================================================================================================
//Function Syntax:
//Usage:
//Parameters:
//
//
//=========================================(Start Of Function)========================================
String generateConfigPage() {
  String html = R=====(
    <!DOCTYPE html>
    <html>
    <head>
      <title>ESP Configuration</title>
      <style>
        body { font-family: Arial, sans-serif; }
        h1 { text-align: center; }
        form { width: 80%; max-width: 400px; margin: 20px auto; padding: 20px; border: 1px solid #ccc; border-radius: 5px; }
        label { display: block; margin-bottom: 5px; }
        input[type="text"], input[type="password"] { width: 100%; padding: 8px; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 3px; box-sizing: border-box; }
        input[type="submit"] { background-color: #007bff; color: white; padding: 10px 15px; border: none; border-radius: 3px; cursor: pointer; font-size: 16px; }
        input[type="submit"]:hover { background-color: #0056b3; }
      </style>
    </head>
    <body>
      <h1>ESP Configuration</h1>
      <form action="/save" method="post">
        <label for="ssid">WiFi SSID:</label><br>
        <input type="text" id="ssid" name="ssid" value="=====) + String(storedSSID) + R=====("><br><br>

        <label for="password">WiFi Password:</label><br>
        <input type="password" id="password" name="password" value="=====) + String(storedPassword) + R=====("><br><br>

        <label for="mqtt_server">MQTT Server:</label><br>
        <input type="text" id="mqtt_server" name="mqtt_server" value="=====) + String(storedMqttServer) + R=====("><br><br>

        <label for="mqtt_topic">MQTT Main Topic:</label><br>
        <input type="text" id="mqtt_topic" name="mqtt_topic" value="=====) + String(storedMqttTopic) + R=====("><br><br>

        <input type="submit" value="Save & Restart">
      </form>
    </body>
    </html>
  =====);
  return html;
}
