#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPAsyncWebSrv.h>
//#include <debug.h>
#include <AsyncTCP.h>
#include <WiFiClientSecure.h>
//#include <WebSocketsClient.h>
#include <AsyncWebSocket.h>
#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include <ArduinoWebsockets.h>
#include <HTTPClient.h>

using namespace websockets;

WebsocketsClient client;
//using namespace websockets;
const char* serverIP = "esp-node.onrender.com";
const int serverPort = 443;
// AsyncWebServer server(80);
// AsyncWebSocket ws("/video");

 //AsyncWebSocketClient *client = NULL;
//WebSocketsClient webSocket;
void onMessageCallback(WebsocketsMessage message) {
  Serial.println("Received message: " + message.data());
  // Handle incoming messages if needed
}
void connectToWiFi() {
  WiFiManager wifiManager;
  
  if (!wifiManager.autoConnect("ESP32-CAM-AP")) {
   Serial.println("Failed to connect and hit timeout. Resetting and try again.");
    delay(1000);
    ESP.restart();
    delay(5000);
  }

  Serial.println("Connected to WiFi");
}

void performWebSocketHandshake() {
  HTTPClient http;

  String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/video";
  Serial.println("WebSocket Handshake URL: " + url);

  http.begin(url);

  int httpResponseCode = http.GET();
  if (httpResponseCode == HTTP_CODE_SWITCHING_PROTOCOLS) {
    Serial.println("WebSocket Handshake successful");
    // Now you can use the WebSocket connection
  } else {
    Serial.println("WebSocket Handshake failed, response code: " + String(httpResponseCode));
  }

  http.end();
}
void connectToWebSocket() {
    client.setInsecure();
  Serial.println("Connecting to WebSocket server...");
  if (client.connect("ws://"+String(serverIP) + ":"+ "/video")) {
    Serial.println("WebSocket connection established");
  } else {
    Serial.println("WebSocket connection failed");
  }
}
void updateDDNS() {

  String updateDDNSUrl = "https://nardi123:nardi123@dynupdate.no-ip.com/nic/update?hostname=nardos123.ddns.net";

  WiFiClientSecure wificlient;
  
 wificlient.setInsecure();

  Serial.print("Connecting to DDNS server...");
  if (wificlient.connect("dynupdate.no-ip.com", 443)) {
    Serial.println(" connected");

   
    wificlient.print(String("GET ") + updateDDNSUrl + " HTTP/1.1\r\n" +
                 "Host: dynupdate.no-ip.com\r\n" +
                 "Connection: close\r\n\r\n");

    while (wificlient.connected()) {
      String line = wificlient.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("Headers received");
        break;
      }
    }

    while (wificlient.available()) {
      String line = wificlient.readStringUntil('\n');
      Serial.println(line);
    }

    Serial.println("DDNS update complete");
  } else {
    Serial.println(" connection failed");
  }

 
  wificlient.stop();
}
// void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
//   // Handle WebSocket events here
//   if (type == WS_EVT_CONNECT) {
//     Serial.println("WebSocket client connected");
//   } else if (type == WS_EVT_DISCONNECT) {
//     Serial.println("WebSocket client disconnected");
//   } else if (type == WS_EVT_DATA) {
//     // Handle WebSocket data here
//   }
// }
// void sendFrameToServer(uint8_t *frameData, size_t frameSize) {
//   ws.binaryAll(frameData, frameSize);
// }



void setup() {
  Serial.begin(9600);

  connectToWiFi();
//WiFi.begin("Nardi", "nardi1234");
 // updateDDNS();

  camera_config_t config;
  
  config.ledc_channel = LEDC_CHANNEL_0;

  
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

  // parameters for image quality and size
  config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 15; //10-63 lower number means higher quality
  config.fb_count = 2;
  

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println(esp_err_to_name(err));
    ESP.restart();
  }
 performWebSocketHandshake();
  connectToWebSocket();
}


void loop() {
 // updateDDNS();
  
  // Capture a frame from the camera
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
 
 //client.beginSecure();
  
client.sendBinary(reinterpret_cast<const char*>(fb->buf), fb->len);



  esp_camera_fb_return(fb);

  delay(100);
}
