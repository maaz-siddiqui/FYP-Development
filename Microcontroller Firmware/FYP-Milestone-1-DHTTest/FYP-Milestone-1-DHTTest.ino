#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
void setup() {
  Serial.begin(9600);
  dht.begin();

}

void loop() {
  delay(2000);
    float h, t;
  h = dht.readHumidity();
  t = dht.readTemperature();

  Serial.print("Humidity : ");
  Serial.print(h);

  Serial.print("Temperature : ");
  Serial.println(t);
}
