#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "src/OV2640.h"
#include <base64.h>

const char* ssid = "GONBIDATUAK";
const char* password = "FAC3200BBC";
const char* mqtt_server = "52.206.254.73";
const char* mqtt_topic = "control-led";
const char* mqtt_topic2 = "photo";
const int mqtt_port = 8883;
const char* mqtt_username = "ledpatata2";
const char* mqtt_password = "ABCD1234e";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Selecciona el modelo de cámara
#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
//#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

OV2640 cam;

void setup() {
  



  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  cam.init(config);

  Serial.begin(9600);
  client.setClient(espClient);
  setup_wifi();
  //espClient.setCACert(root_ca);
  client.setServer(mqtt_server, 1883);
  
  //client.connect("ESP32Client", mqtt_username, mqtt_password);
  client.setCallback(callback);



}

void setup_wifi() {
// Conecta al WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  if (strcmp(topic, mqtt_topic2) == 0) 
  {
    return;
  }

  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.println("] ");
  capture_photo();
}

void capture_photo2() {
  Serial.println("Capturando foto...");
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Error al capturar la imagen");
    return;
  }
  
  // Convertir los datos de la imagen a base64
  String imageBase64 = base64::encode(fb->buf, fb->len);
  Serial.println(imageBase64.c_str());
  // Publicar la imagen en base64
  if (!client.connected()) {
    Serial.println("Error: cliente MQTT no conectado");
    esp_camera_fb_return(fb);
    return;
  }

  // Publicar los datos de la imagen en base64
  if (!client.publish(mqtt_topic2, (const uint8_t*)imageBase64.c_str(), imageBase64.length(), false)) {
    Serial.println("Error al publicar la imagen");
  } else {
    Serial.println("Imagen publicada correctamente");
  }

  // Liberar la memoria utilizada por el framebuffer
  esp_camera_fb_return(fb);
}

void capture_photo() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Error al capturar la imagen");
    return;
  }
  
  // Convertir los datos de la imagen a hexadecimal
  String imageHex = "";
  for (size_t i = 0; i < fb->len; i++) {
    char hex[3];
    sprintf(hex, "%02X", fb->buf[i]);
    imageHex += hex;
  }
  
  // Publicar la imagen en formato hexadecimal
  if (!client.connected()) {
    Serial.println("Error: cliente MQTT no conectado");
    esp_camera_fb_return(fb);
    return;
  }

  Serial.println(imageHex.c_str());

  // Publicar los datos de la imagen en formato hexadecimal
  if (!client.publish(mqtt_topic2, imageHex.c_str())) {
    Serial.println("Error al publicar la imagen");
  } else {
    Serial.println("Imagen publicada correctamente");
  }

  // Liberar la memoria utilizada por el framebuffer
  esp_camera_fb_return(fb);
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión a MQTT...");
    if (client.connect("ESP32Client2")) {
      Serial.println("conectado");
      if (client.subscribe(mqtt_topic)) {
        Serial.print("Subscrito al topic ");
        Serial.println(mqtt_topic);
      } else {
        Serial.print("Error al suscribirse al topic");
        Serial.println(mqtt_topic);
      }

      if (client.subscribe(mqtt_topic2)) {
        Serial.print("Subscrito al topic ");
        Serial.println(mqtt_topic2);
      } else {
        Serial.print("Error al suscribirse al topic");
        Serial.println(mqtt_topic2);
      }
      client.setCallback(callback);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 1 segundos");
      delay(1000);
    }
  }
}

void loop() {
  if(!client.connected())
  {
    reconnect();
  }

  client.loop();
}
