#include <MQ135.h>
#include "DHT.h"
#include <utils.h>
#include "configuration.h"


/* The MQ135 library need calibration based on clean atmospheric air
* 1. comment out the upload() call and decrease sleep time to few seconds
* 2. put the sensor outside where air is not polluted for few minutes
*    until the voc rzero value stabilizes
* 3. put the rzero value to MQ135.h libarary to RZERO macro
* 4. uncoment upload() and set sleep to final value
* 5. recompile and reupload firmware to esp8266
*
* When rzero is set correctly, the corrected voc should be around 400 in clean air
*/

// DHT sensor config, used to calibrate MQ135
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
uint8_t DHT_PIN = D2;
DHT dht(DHT_PIN, DHTTYPE);

// MQ135 sensor init
#define MQ135_PIN A0
MQ135 mq135 = MQ135(MQ135_PIN);

// sensors' values
float temperature;
float humidity;
float corrected_voc;
float rzero;

void setup() {
  init_serial();

  // initialize DHT sensor
  pinMode(DHT_PIN, INPUT_PULLUP);
  dht.begin();

}

void loop() {
  //get sensors data
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  float voc = mq135.getPPM();
  corrected_voc = mq135.getCorrectedPPM(temperature, humidity);
  rzero = mq135.getRZero();

  // process all data
  logln("Temperature: "+ String(temperature));
  logln("Humidity: " + String(humidity));
  logln("VOC: "+ String(voc));
  logln("VOC corrected: "+ String(corrected_voc));
  logln("VOC rzero: "+ String(rzero));

  wifi_reconnect(ssid, wifi_password, ip_last_byte);
  upload();
  delay(refresh_rate*1000);
}


void upload() {
  Broker b = Broker(broker_url);
  b.addProperty("refresh_rate", String(refresh_rate));
  // report temp& humidity as debug - they are meant to be used for voc
  // correction only, not ambient meassurements as they are affected by 
  // the mq135 sensor's heat
  // report only 1 decimal place - saves space in HA dashboards
  b.addProperty("debug_temperature", String(temperature, 1));
  b.addProperty("debug_humidity", String(humidity, 1));
  b.addProperty("voc", String(corrected_voc, 1));
  b.addProperty("debug_rzero", String(rzero, 1));
  b.upload();
}
