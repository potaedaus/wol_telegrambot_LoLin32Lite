#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "WakeOnLan.h"
#include "WiFiUDP.h"

WiFiClientSecure secured_client;
WiFiUDP UDP;
WakeOnLan WOL(UDP);

const char *ssid = "Wifi_ID";
const char *password = "Password";
#define BOTtoken "GeneratedBotToken" // get this by creating a Telegram bot on Botfather
#define CHAT_ID "18364519"           // use whatever way to get your Telegram chat ID
#define MAC_ADDR "05:A9:7F:EE:F8:C8" // for PC1, paste your MAC address here, followed by PC2 and PC3 if needed below
#define MAC_ADDR2 "0F:A9:BF:DE:38:18"
#define MAC_ADDR3 "A5:EF:2F:3E:F8:C1"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 22;
bool ledState = HIGH;

void handleLED(int duration)
{
  digitalWrite(ledPin, LOW);
  delay(duration);
  digitalWrite(ledPin, HIGH);
}

void sendWOL(const char *macAddress)
{
  WOL.sendMagicPacket(macAddress); // send WOL on default port (9)
  handleLED(1500);
  delay(300);
}

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user. Sorry, " + bot.messages[i].from_name + ".");
      continue;
    }

    String text = bot.messages[i].text;

    if (text == "/wol" || text == "/wol2" || text == "/wol3")
    {
      const char *macAddress = (text == "/wol") ? MAC_ADDR : (text == "/wol2") ? MAC_ADDR2
                                                                               : MAC_ADDR3;
      sendWOL(macAddress);
      bot.sendMessage(chat_id, "WoL packet sent to " + String(macAddress));
    }
    else if (text == "/test")
    {
      bot.sendMessage(chat_id, "I'm awake.", "");
      handleLED(1000);
    }
    else if (text == "/start")
    {
      bot.sendMessage(chat_id, "Welcome to **Suis Komputer**, " + bot.messages[i].from_name + ".\n"
                                                                                              "Here are the commands you can send using this bot:\n\n"
                                                                                              "/wol : Wake up PC1\n"
                                                                                              "/wol2 : Wake up PC2\n"
                                                                                              "/wol3 : Wake up PC3\n"
                                                                                              "/test : Check if your ESP32 is awake\n",
                      "Markdown");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // for ESP32, ESP8266 takyah this line.

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

void loop()
{
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}