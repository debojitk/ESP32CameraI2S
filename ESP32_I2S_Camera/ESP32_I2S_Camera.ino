#include "OV7670.h"

//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library

#include <WiFi.h>
#include <esp_wifi.h>
#include <lwip/err.h>
#include "BMP.h"
#include "raw_wifi.h"
#include "wifi_headers.h"

const int SIOD = SDA; //SDA
const int SIOC = SCL; //SCL

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

//const int TFT_DC = 2;
//const int TFT_CS = 5;
//DIN <- MOSI 23
//CLK <- SCK 18

#define ssid1        "XXX"
#define password1    "XXX"
//#define ssid2        ""
//#define password2    ""

//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, 0/*no reset*/);
OV7670 *camera;

//WiFiMulti wifiMulti;
//WiFiServer server(80);

//uint8_t MAC[6];
//uint8_t MAC_BROADCAST[6];

unsigned char bmpHeader[BMP::headerSize];
WiFiUDP udp;

//void serve()
//{
//  WiFiClient client = server.available();
//  if (client) 
//  {
//    //Serial.println("New Client.");
//    String currentLine = "";
//    while (client.connected()) 
//    {
//      if (client.available()) 
//      {
//        char c = client.read();
//        //Serial.write(c);
//        if (c == '\n') 
//        {
//          if (currentLine.length() == 0) 
//          {
//            client.println("HTTP/1.1 200 OK");
//            client.println("Content-type:text/html");
//            client.println();
//            client.print(
//              "<style>body{margin: 0}\nimg{height: 100%; width: auto}</style>"
//              "<img id='a' src='/camera' onload='this.style.display=\"initial\"; var b = document.getElementById(\"b\"); b.style.display=\"none\"; b.src=\"camera?\"+Date.now(); '>"
//              "<img id='b' style='display: none' src='/camera' onload='this.style.display=\"initial\"; var a = document.getElementById(\"a\"); a.style.display=\"none\"; a.src=\"camera?\"+Date.now(); '>");
//            client.println();
//            break;
//          } 
//          else 
//          {
//            currentLine = "";
//          }
//        } 
//        else if (c != '\r') 
//        {
//          currentLine += c;
//        }
//        
//        if(currentLine.endsWith("GET /camera"))
//        {
//            client.println("HTTP/1.1 200 OK");
//            client.println("Content-type:image/bmp");
//            client.println();
//
//            client.write(bmpHeader, BMP::headerSize);
//            client.write(camera->frame, camera->xres * camera->yres * 2);
//        }
//      }
//    }
//    // close the connection:
//    client.stop();
//    //Serial.println("Client Disconnected.");
//  }  
//}

void setup() 
{
  Serial.begin(115200);

  WiFi.disconnect(true);

  // Force data rate
  wifi_internal_rate_t rate;
  rate.fix_rate = RATE_MCS4_SP;
  esp_wifi_internal_set_rate(100, 1, 4, &rate);

  WiFi.softAP("Test");

  // Change beacon_interval because 100ms is crazy!
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_AP, &conf);
  conf.ap.beacon_interval = 5000;
  esp_wifi_set_config(WIFI_IF_AP, &conf);

  //WiFi.macAddress(MAC);
 // memset(MAC_BROADCAST, 0xFF, 6);

//  wifiMulti.addAP(ssid1, password1);
//  //wifiMulti.addAP(ssid2, password2);
//  Serial.println("Connecting Wifi...");
//  if(wifiMulti.run() == WL_CONNECTED) {
//      Serial.println("");
//      Serial.println("WiFi connected");
//      Serial.println("IP address: ");
//      Serial.println(WiFi.localIP());
//  }
  
  camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);
  
//  tft.initR(INITR_BLACKTAB);
//  tft.fillScreen(0);
  //server.begin();
}

void displayRGB565(unsigned char * frame, int xres, int yres)
{
  udp.beginPacket("255.255.255.255", 3333);
  udp.write(frame, xres * yres * 2);
  udp.endPacket();
//  static uint16_t sequence = 0;
//  uint8_t data[1500];
//  WiFiHeader* pkt = (WiFiHeader*)data;
//  memset(data, 0, sizeof(WiFiHeader));
//  pkt->frame_control.type = WIFI_TYPE_DATA;
//  pkt->frame_control.subtype = 0;
//  pkt->frame_control.to_ds = 1;
//  memcpy(pkt->address_1, MAC_BROADCAST, 6);
//  memcpy(pkt->address_2, MAC_BROADCAST, 6);
//  memcpy(pkt->address_3, MAC, 6);
//  pkt->sequence.sequence_number = sequence++;
//
//  int size = xres * yres * 2;
//  int data_size = sizeof(data) - sizeof(WiFiHeader);
//  int fragment = 0;
//
//  for (int i = 0; i < size; i += data_size) {
//    if (size - i < data_size) {
//      data_size = size - i;
//    }
//    memcpy(data + sizeof(WiFiHeader), frame + i, data_size);
//    pkt->sequence.fragment_number = fragment++;
//    int result = esp_wifi_80211_tx(WIFI_IF_AP, data, sizeof(WiFiHeader) + data_size, false);
//    switch (result) {
//      case ERR_OK: Serial.println("ERR_OK"); break;
//      case ERR_MEM: Serial.println("ERR_MEM"); break;
//      case ERR_IF: Serial.println("ERR_IF"); break;
//      case ERR_ARG: Serial.println("ERR_ARG"); break;
//      default: Serial.println(result);
//    }
//    
//  }
  
}

void loop()
{
  Serial.println("loop");
  camera->oneFrame();
  //serve();
  displayRGB565(camera->frame, camera->xres, camera->yres);
  //delay(500);
}
