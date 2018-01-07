#include <WiFi.h>
#include <esp_wifi.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include "BMP.h"
#include "OV7670.h"
#include "wifi_internal.h"

const int SIOD = SDA;
const int SIOC = SCL;

const int VSYNC = 32;
const int HREF = 39;

const int XCLK = 18;
const int PCLK = 36;

const int D0 = 12;
const int D1 = 14;
const int D2 = 27;
const int D3 = 26;
const int D4 = 25;
const int D5 = 35;
const int D6 = 34;
const int D7 = 33;

#define SSID "Test"

#define MAX_PACKET_SIZE 1460
#define MAX_DATA_SIZE 1450

struct PacketHeader {
  uint8_t frame;
  uint32_t offset;
  uint32_t len;
};

OV7670 *camera;

byte bmpHeader[BMP::headerSize];
int udp_server = -1;
struct sockaddr_in destination;
bool video_running = false;

void setup() {
  Serial.begin(115200);

  WiFi.disconnect(true);

  // Force data rate
  wifi_internal_rate_t rate;
  rate.fix_rate = RATE_MCS4_SP;
  esp_wifi_internal_set_rate(100, 1, 4, &rate);

  WiFi.softAP(SSID);
  WiFi.onEvent(onClientChange, SYSTEM_EVENT_AP_STACONNECTED);
  WiFi.onEvent(onClientChange, SYSTEM_EVENT_AP_STADISCONNECTED);

  // Change beacon_interval because 100ms is crazy!
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_AP, &conf);
  conf.ap.beacon_interval = 3000;
  esp_wifi_set_config(WIFI_IF_AP, &conf);
  
  camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);

  // Init socket
  udp_server = socket(AF_INET, SOCK_DGRAM, 0);
  destination.sin_addr.s_addr = (uint32_t)IPAddress(255,255,255,255);
  destination.sin_family = AF_INET;
  destination.sin_port = htons(3333);

  Serial.println("Up, Waiting for clients");
}

void onClientChange(system_event_id_t event) {
  // Only start sending video after a client connects to avoid flooding
  // the channel when the client is attempting to connect.
  video_running = WiFi.softAPgetStationNum() != 0;
  if (video_running) {
    Serial.println("Video started!");
  } else {
    Serial.println("STOPPED");
  }
}

void loop() {
  if (!video_running) {
    return;
  }

  camera->oneFrame();

  static byte packet[MAX_PACKET_SIZE];
  static uint8_t framenum = 0;
  PacketHeader* header = (PacketHeader*)packet;
  header->frame = framenum++;
  header->len = camera->frameBytes;

  for (int i = 0; i < camera->frameBytes; i += MAX_DATA_SIZE) {
    bool end = i + MAX_DATA_SIZE > camera->frameBytes;
    int len = end ? camera->frameBytes - i : MAX_DATA_SIZE;
      header->offset = i;
      memcpy(packet + sizeof(PacketHeader), camera->frame + i, len);
    int sent = sendto(
      udp_server,
      packet, sizeof(PacketHeader) + len,
      0,
      (struct sockaddr*) &destination, sizeof(destination)
    );
  }

  // FPS counter
  static unsigned int frames = 0;
  static unsigned long last = millis();
  ++frames;
  unsigned long now = millis();
  if (now - last > 1000) {
    Serial.println(frames);
    last = now;
    frames = 0;
  }
}

