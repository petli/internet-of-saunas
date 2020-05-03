// -*-c++-*-

#include <RF24.h>
#include <DHT.h>

#define RF_CE_PIN       8
#define RF_CSN_PIN     11
#define DHT_PIN         2

static byte gateway_address[] = "IoSGW";

static RF24 radio(RF_CE_PIN, RF_CSN_PIN);

#define MSG_COUNTER  0
#define MSG_TEMP     1
#define MSG_HUMIDITY 2
#define MSG_SIZE 3

static byte message[MSG_SIZE];

static DHT dht(DHT_PIN, DHT22);

void setup()
{
  Serial.begin(115200);
  delay(5000);
  Serial.println(F("Sauna sensor starting (build " __DATE__ " " __TIME__ ")"));

  pinMode(LED_BUILTIN, OUTPUT);

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
  }
  else {
    message[MSG_TEMP] = (int)t;
    message[MSG_HUMIDITY] = (int)h;
  }

  message[MSG_COUNTER]++;

  radio.powerUp();

  if (!radio.write(message, MSG_SIZE)) {
    Serial.println(F("Failed to send sensor reading to gateway"));
    blink_error();
  }

  radio.powerDown();

  blink();
}


static void blink()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
}

static void blink_error()
{
  blink();
  delay(200);
  blink();
  delay(200);
  blink();
  delay(200);
  blink();
  delay(200);
  blink();
  delay(200);
}
