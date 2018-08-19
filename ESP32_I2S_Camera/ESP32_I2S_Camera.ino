#include "OV7670.h"

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include "BMP.h"
#include "debugmacros.h"
#include <esp_wifi.h>
#include <lwip/err.h>
#include <lwip/sockets.h>

#pragma pack(1)

const int SIOD = 21; //SDA
const int SIOC = 22; //SCL

const int VSYNC = 34;
const int HREF = 35;

const int XCLK = 32;
const int PCLK = 33;

const int D0 = 27;
const int D1 = 17;
const int D2 = 16;
const int D3 = 15;
const int D4 = 14;
const int D5 = 13;
const int D6 = 12;
const int D7 = 4;

const int TFT_DC = 2;
const int TFT_CS = 5;
//DIN <- MOSI 23
//CLK <- SCK 18
#define MAX_PACKET_SIZE 1460
#define MAX_DATA_SIZE 1450

#define ssid        "debojit-dlink"
#define password    "India@123"
//#define ssid2        ""
//#define password2    ""

struct PacketHeader {
	uint8_t frame;
	uint32_t offset;
	uint32_t len;
};
int udp_server = -1;
struct sockaddr_in destination;

OV7670 *camera;

WiFiMulti wifiMulti;
WiFiServer server(80);
static byte packet[MAX_DATA_SIZE];

unsigned char bmpHeader[BMP::headerSize];

void serve()
{
	WiFiClient client = server.available();
	if (client)
	{
		DEBUG_PRINTLN("New Client.");
		String currentLine = "";
		while (client.connected())
		{
			if (client.available())
			{
				char c = client.read();
				//Serial.write(c);
				if (c == '\n')
				{
					if (currentLine.length() == 0)
					{
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println();
						client.print(
								"<style>body{margin: 0}\nimg{height: 100%; width: auto}</style>"
								"<img id='a' src='/camera' onload='this.style.display=\"initial\"; var b = document.getElementById(\"b\"); b.style.display=\"none\"; b.src=\"camera?\"+Date.now(); '>"
								"<img id='b' style='display: none' src='/camera' onload='this.style.display=\"initial\"; var a = document.getElementById(\"a\"); a.style.display=\"none\"; a.src=\"camera?\"+Date.now(); '>");
						client.println();
						break;
					}
					else
					{
						currentLine = "";
					}
				}
				else if (c != '\r')
				{
					currentLine += c;
				}

				if(currentLine.endsWith("GET /camera"))
				{
					client.println("HTTP/1.1 200 OK");
					client.println("Content-type:image/bmp");
					client.println();

					client.write(bmpHeader, BMP::headerSize);
					/*
					for(int i = 0; i < camera->xres * camera->yres * 2; i++){
						client.write(camera->frame[i]);

					}
					 */
					for (int i = 0; i < camera->frameBytes; i += MAX_DATA_SIZE) {
						bool end = i + MAX_DATA_SIZE > camera->frameBytes;
						int len = end ? camera->frameBytes - i : MAX_DATA_SIZE;
						memcpy((void *)packet , camera->frame + i, len);
						size_t bytes=client.write(packet, len);
						DEBUG_PRINT("Writing: " );DEBUG_PRINTLN(bytes);
					}
				}
			}
		}
		// close the connection:
		client.stop();
		DEBUG_PRINTLN("Client Disconnected.");
	}
}

void setup() 
{
	Serial.begin(115200);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	uint32_t delayMs=10000+millis();
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		INFO_PRINT(".");
		if(delayMs<millis()){
			INFO_PRINTF("Could not connect to ssid: %s", ssid);
			WiFi.disconnect();
			break;
		}
	}

	INFO_PRINT(F("WiFi ip is: "));INFO_PRINTLN(WiFi.localIP());
	INFO_PRINTLN(F("#################"));
	WiFi.printDiag(Serial);
	INFO_PRINTLN(F("setup done"));

	camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
	BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);

	// Init socket
	udp_server = socket(AF_INET, SOCK_DGRAM, 0);
	destination.sin_addr.s_addr = (uint32_t)IPAddress(192,168,0,100);
	destination.sin_family = AF_INET;
	destination.sin_port = htons(3333);

	Serial.println("Up, Waiting for clients");
	//server.begin();
}

void onClientChange(system_event_id_t event) {
	// Only start sending video after a client connects to avoid flooding
	// the channel when the client is attempting to connect.
}

void loop()
{
	DEBUG_PRINT(micros()); DEBUG_PRINTLN(": Start camera capture");
	camera->oneFrame();
	DEBUG_PRINT(micros()); DEBUG_PRINTLN(": End camera capture, start serve");
	//serve();
	processUDP();
	DEBUG_PRINT(micros()); DEBUG_PRINTLN(": End serve");
	DEBUG_PRINT("Free mem:");DEBUG_PRINTLN(ESP.getFreeHeap());
}

void processUDP(){
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
//		INFO_PRINT(header->frame);INFO_PRINT(',');
//		INFO_PRINT(header->offset);INFO_PRINT(',');
//		INFO_PRINTLN(header->len);
//		delay(200);
		//INFO_PRINTLN(sizeof(PacketHeader));
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
