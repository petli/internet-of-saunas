// -*-c++-*-

#include <RF24.h>
#include <DHT.h>
#include <LiquidCrystal.h>

#define LCD_RS          2
#define LCD_E           3
#define LCD_D4          4
#define LCD_D5          5
#define LCD_D6          6
#define LCD_D7          7
#define DHT_PIN         8
#define RF_CE_PIN       9
#define RF_CSN_PIN     10

static byte gateway_address[] = "IoSGW";

static RF24 radio(RF_CE_PIN, RF_CSN_PIN);

#define MSG_COUNTER  0
#define MSG_TEMP     1
#define MSG_HUMIDITY 2
#define MSG_SIZE 3

static byte message[MSG_SIZE];

static DHT dht(DHT_PIN, DHT22);

static LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

static char lcd_text1[20];
static char lcd_text2[20];

void setup()
{
  lcd.begin(16, 2);
  display("Sensor starting", "...");

  Serial.begin(115200);
  delay(5000);
  Serial.println(F("Sauna sensor starting (build " __DATE__ " " __TIME__ ")"));

  // pinMode(LED_BUILTIN, OUTPUT);

  dht.begin();

  radio.begin();
  radio.openWritingPipe(gateway_address);
  radio.setPayloadSize(MSG_SIZE);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
}

void loop()
{
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor"));
    message[MSG_TEMP] = message[MSG_HUMIDITY] = 0xff;
    blink_error();

    display("Error reading", "temperature!");
  }
  else {
    int it = (int)(t + 0.5);
    int ih = (int)(h + 0.5);

    message[MSG_TEMP] = it;
    message[MSG_HUMIDITY] = ih;

    sprintf_P(lcd_text1, PSTR("Temperature %3dC"), it);
    sprintf_P(lcd_text2, PSTR("Humidity    %3d%%"), ih);
    display(lcd_text1, lcd_text2);
  }

  message[MSG_COUNTER]++;

  radio.powerUp();

  if (!radio.write(message, MSG_SIZE)) {
    Serial.println(F("Failed to send sensor reading to gateway"));
    blink_error();
    display("Error sending", "msg to gateway!");
  }

  radio.powerDown();

  blink();
}


static void display(const char* text1, const char* text2)
{
  lcd.clear();
  lcd.print(text1);
  lcd.setCursor(0, 1);
  lcd.print(text2);
}

static void blink()
{
//  digitalWrite(LED_BUILTIN, HIGH);
//  delay(20);
//  digitalWrite(LED_BUILTIN, LOW);
}

static void blink_error()
{
//  blink();
//  delay(200);
//  blink();
//  delay(200);
//  blink();
//  delay(200);
//  blink();
//  delay(200);
//  blink();
//  delay(200);
}
