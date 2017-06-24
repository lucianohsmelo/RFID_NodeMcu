#include "MFRC522.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define RST_PIN  D3 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  D4 // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4 
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

const char* ssid = "TIM WiFi Fon";
const char* password = "estoucomfome";
const char *mqtt_server = "192.168.1.10";
const int mqtt_port = 1883;
const char *mqtt_user = "localBroker";
const char *mqtt_pass = "1234";
const char *mqtt_client_name = "arduinoClient1"; // Client connections cant have the same connection name

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String message;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, HIGH);
  Serial.begin(115200);
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  message = String((char*)payload);
  Serial.println(message);
  digitalWrite(BUILTIN_LED, HIGH);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("inTopic", "NodeMcu/Alive");
      // ... and resubscribe
      client.subscribe("outTopic");
    }else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() { 

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
   
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  
  Serial.println();
  union {long total; byte a,b,c,d;};
  if (buffer[1] == 0x72){
    client.publish("outTopic","ID_Luciano");
  }else if (buffer[1] == 0xC5){
    client.publish("outTopic","ID_Joao");
  }else if (buffer[1] == 0xC7){
    client.publish("outTopic","ID_Maria");
  }
  
}
