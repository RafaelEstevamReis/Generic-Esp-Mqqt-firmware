// Generic firmware to automate ESP-01
// Uses MQTT to receive and Publish commands
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*****************************
 Instructions
  1. Change Networking
     SSID, Pwd, mqtt host
  2. Change ESP_LOCATION
  3. Enable/Disable serial debug
     Serial debug on 9600
  4. Setup other stuff on setupStuff()
  5. Add loop related stuff on loopStuff()
     Runs around once every ~100ms
  6. Add wifi loop related stuff on loopCnnStuff()
     Runs around once every ~100ms only if connecteed
  7. Add subs and callbacks
*****************************/

#define SERIAL_DEBUG_ENABLED

/* -- Networking -- */
const char *ssid     = "Wifi name";
const char *password = "mysecretwifipassword";
#define MQTT_HOST "10.0.0.2"
#define MQTT_PORT 1883

/* -- ID -- */
#define ESP_LOCATION "LOCATION"
#define MQTT_BASE_PATH "home/" ESP_LOCATION "/"
#define MQTT_CLIENT_ID "esp." ESP_LOCATION

/* -- Functions -- */
void setupStuff(){
  
}
void loopStuff(){
  
}
void loopCnnStuff(){
  
}

// internal mqtt callback stuff
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))
typedef void (*subCallback) (byte* payload, unsigned int length);

// Put your mqtt subs and callbacks here: 
void callback_gpio02(byte* payload, unsigned int length) {  
  Serial.print("callback_gpio02 ");
  Serial.print(": [");
  Serial.print(length);
  Serial.print("] ");
  
  for(unsigned int i =0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
}

// [3] Add subs and callbacks here
String subscribers[] = {
  "gpio02"
};
subCallback callbacks[] = {
  callback_gpio02
};

/* -- Status -- */
#define MQTT_PUB_PING MQTT_BASE_PATH "ping"
#define MQTT_PUB_RSSI MQTT_BASE_PATH "signal"

/* -- Variables -- */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* -- Maintenance -- */
void setupWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay (1000);
    #ifdef SERIAL_DEBUG_ENABLED
    Serial.println("NoWiFi ");
    #endif
  }
  
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print("MAC: ");
  Serial.print(WiFi.macAddress());
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());
  #endif
}
void setupMqtt() {  
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(callback);
}
void setup() {
  delay(1000); // wait a little
  
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.begin(9600);
  Serial.println();
  Serial.println("ESP Init with DEBUG");
  Serial.print("Client: ");
  Serial.println(MQTT_CLIENT_ID);
  #endif

  setupWifi();
  setupMqtt();
  setupStuff();
}

bool checkConnection(){
  if(!checkWiFi()) return false;
  if(!checkMqtt()) return false;
  return true;
}
boolean checkWiFi(){
  if(WiFi.status() == WL_CONNECTED) return true;
  
  setupWifi();
  return false;
}
boolean checkMqtt() {
  if(mqttClient.connected())  return true;
  
    #ifdef SERIAL_DEBUG_ENABLED
    Serial.println("MQTT down, reconnecting...");
    #endif
    
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    for(int i = 0; i < NUMITEMS(subscribers); i++){
      String subs = MQTT_BASE_PATH+subscribers[i];

      #ifdef SERIAL_DEBUG_ENABLED
      Serial.print("MQTT Sub: ");
      Serial.print(subs);
      Serial.println();
      #endif
      
      mqttClient.subscribe(subs.c_str());
    }

    #ifdef SERIAL_DEBUG_ENABLED
    Serial.print("MQTT ping: ");
    Serial.println(MQTT_PUB_PING);
    Serial.print("MQTT signal: ");
    Serial.println(MQTT_PUB_RSSI);
    
    Serial.println("MQTT-OK");
    #endif
  }
  return mqttClient.connected();
}

void publishStatus(){
    mqttClient.publish(MQTT_PUB_PING, "online");
    long rssi = WiFi.RSSI();
    mqttClient.publish(MQTT_PUB_RSSI, String(rssi).c_str());
    
    #ifdef SERIAL_DEBUG_ENABLED
    Serial.print("RSSI: ");
    Serial.println(rssi);
    #endif
}

int status_timeout = 0;
void loop() {
  delay(100);

  loopStuff();
  
  if(!checkConnection()) {
    status_timeout = 0; // publish on reconnect
    return;
  }

  if(status_timeout-- <= 0){
    status_timeout = 10*60;
    publishStatus();
  }
  
  mqttClient.loop();
  
  loopCnnStuff();
}

void callback(char* topic, byte* payload, unsigned int length) {  
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print("CallBack on ");
  Serial.print(topic);
  Serial.print(": [");
  Serial.print(length);
  Serial.print("] ");
  
  for(unsigned int i =0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
  #endif

  if(length < 0) return;
  
  for(int i = 0; i < NUMITEMS(subscribers); i++){
    String subs = MQTT_BASE_PATH+subscribers[i];  
    if (subs.equals(topic)){
      (*callbacks[i])(payload, length);
    }
  }  
}

/**/
