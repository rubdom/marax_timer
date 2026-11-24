#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <Timer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>

// ================= USER WIFI CONFIG =====================
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mdnsName = "marax"; // Accessible at http://marax.local

// ================= HARDWARE PINS ========================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define PUMP_PIN     D7

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial mySerial(D5, D6); // RX, TX
Timer t;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// ================= PERSISTENT NTFY CONFIG ===============
struct NotifyConfig {
  uint32_t magic;
  char serverUrl[64];
  char topic[48];
  char title[48];
  char message[96];
  int readyTempC;
  bool enabled;
};

NotifyConfig notifyCfg;
const uint32_t CONFIG_MAGIC = 0x4D415241; // "MARA"

// ================= MARAX STATE ==========================
const byte numChars = 32;
char receivedChars[numChars];
static byte ndx = 0;
char endMarker = '\n';
char rc;

int timerCount = 0;
int prevTimerCount = 0;
bool timerStarted = false;
long timerStartMillis = 0;
long timerStopMillis = 0;
long timerDisplayOffMillis = 0;
long serialUpdateMillis = 0;
long lastSerialReceivedMillis = 0;

bool displayOn = true;
bool machineOnline = false;
bool readyAlertSent = false;
String lastAlertStatus = "Ready (waiting for warm-up)";

// ================= BOUNCING ESPRESSO CUP ART ============
const unsigned char espressoCupBitmap[] PROGMEM = {
  0x00, 0x08, 0x20, 0x00,
  0x00, 0x10, 0x40, 0x00,
  0x00, 0x10, 0x40, 0x00,
  0x00, 0x08, 0x20, 0x00,
  0x00, 0x04, 0x10, 0x00,
  0x00, 0x08, 0x20, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x03, 0xFF, 0xF8, 0x00,
  0x07, 0xFF, 0xFC, 0x00,
  0x04, 0x00, 0x04, 0x00,
  0x05, 0xFF, 0xF4, 0x00,
  0x05, 0xFF, 0xF7, 0xC0,
  0x05, 0xFF, 0xF4, 0x60,
  0x05, 0xFF, 0xF4, 0x20,
  0x05, 0xFF, 0xF4, 0x20,
  0x05, 0xFF, 0xF4, 0x60,
  0x03, 0xFF, 0xF7, 0xC0,
  0x03, 0xFF, 0xF8, 0x00,
  0x01, 0xFF, 0xF0, 0x00,
  0x00, 0xFF, 0xE0, 0x00,
  0x00, 0x7F, 0xC0, 0x00,
  0x1F, 0xFF, 0xFF, 0x00,
  0x0F, 0xFF, 0xFE, 0x00,
  0x03, 0xFF, 0xF8, 0x00
};

int cupX = 20;
int cupY = 14;
int cupVX = 2;
int cupVY = 1;
const int CUP_W = 32;
const int CUP_H = 24;

// ================= EEPROM HELPERS =======================
void loadNotifyConfig() {
  EEPROM.begin(512);
  EEPROM.get(0, notifyCfg);
  if (notifyCfg.magic != CONFIG_MAGIC) {
    notifyCfg.magic = CONFIG_MAGIC;
    strncpy(notifyCfg.serverUrl, "https://ntfy.sh", sizeof(notifyCfg.serverUrl));
    strncpy(notifyCfg.topic, "marax-alerts", sizeof(notifyCfg.topic));
    strncpy(notifyCfg.title, "Lelit MaraX Ready", sizeof(notifyCfg.title));
    strncpy(notifyCfg.message, "Machine is up to temp and ready to pull!", sizeof(notifyCfg.message));
    notifyCfg.readyTempC = 93;
    notifyCfg.enabled = true;
    EEPROM.put(0, notifyCfg);
    EEPROM.commit();
  }
}

void saveNotifyConfig() {
  notifyCfg.magic = CONFIG_MAGIC;
  EEPROM.put(0, notifyCfg);
  EEPROM.commit();
}

// ================= NTFY PUSH SENDER =====================
bool sendNtfyPush(const String& title, const String& body, int currentTemp, bool isTest) {
  if (!isTest && !notifyCfg.enabled) return false;
  if (WiFi.status() != WL_CONNECTED) {
    lastAlertStatus = "Failed: WiFi disconnected";
