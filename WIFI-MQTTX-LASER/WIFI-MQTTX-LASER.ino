#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <driver/ledc.h>


const char* ssid = "GONBIDATUAK";
const char* password = "FAC3200BBC";

const char* mqtt_server = "7b97ddb1a02b45308522e93ed8f9bcd1.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "ledpatata";
const char* mqtt_password = "ABCD1234e";

// Definición de los canales a utilizar
const char* CONTROL_LED_TOPIC = "control-led";
const char* VALOR_ANALOGICO_TOPIC = "valor-analogico";

const int inputPin = 36;
const int laserPin = 23;
const int receiverPin = 4;

#define BUZZER_PIN 19 // Change this to the GPIO pin you've connected the buzzer to
#define BUZZER_CHANNEL 0 // You can use any available channel, from 0 to 15
#define BUZZER_FREQ 1000 // Adjust the frequency as needed

long previousPublishTime = 0;
long publishTimeInterval = 2000;

static const char* root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
unsigned long detectionTime = 0; // Tiempo de detección del láser
bool laserDetected = false; // Indicador de detección del láser

void setup_wifi() {
  delay(1000);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("------------------------------------------------------------------------");
  delay(2000);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(CONTROL_LED_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishMessage() {
  StaticJsonDocument<200> jsonDocument;
  
  // Añadimos los datos que deseamos enviar al objeto JSON
  jsonDocument["msg"] = "Alert! Intruder detected!";
  jsonDocument["time"] = millis(); // Ejemplo de agregar el tiempo actual
  
  // Convertimos el objeto JSON a un string
  char jsonString[200];
  serializeJson(jsonDocument, jsonString);
  
  // Publicamos el mensaje MQTT
  client.publish(CONTROL_LED_TOPIC, jsonString);
  //client.publish(CONTROL_LED_TOPIC, "Alert! Intruder detected!");
}

void setup() {
  pinMode(laserPin, OUTPUT);
  pinMode(receiverPin, INPUT);
  
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, 10); // 10-bit resolution
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  
  Serial.begin(9600);
  setup_wifi();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  long currentTime = millis();
  
  // Encender el láser
  digitalWrite(laserPin, HIGH);
  // Leer el estado del detector
  int detected = digitalRead(receiverPin);
  
  ledcWrite(BUZZER_CHANNEL, 0); //deafen
  
  if (detected == LOW) {
    if (!laserDetected) {
      // Si es la primera vez que se detecta el láser, guardar el tiempo
      detectionTime = millis();
      laserDetected = true;
    } else {
      // Si ya se había detectado antes, verificar si han pasado al menos 2 segundos
        if (currentTime - previousPublishTime >= publishTimeInterval) 
        {
          previousPublishTime = currentTime;
          Serial.println("Detected!");
          ledcWrite(BUZZER_CHANNEL, 512); // 50% duty cycle
          delay(300);
          publishMessage();

        }
    }
    
    

    
  } else {
    // Si no se detecta el láser, restablecer las variables
    laserDetected = false;
    detectionTime = 0;
    //Serial.println("No laser");
  }

  // Permitir que el cliente MQTT maneje los mensajes entrantes
  client.loop();
}
