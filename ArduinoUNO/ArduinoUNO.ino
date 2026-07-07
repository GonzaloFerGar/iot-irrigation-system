#include <DHT.h>

#define DHT_PIN 2
#define DHT_TYPE DHT11
#define FC_ANALOG_PIN A0
#define CSM_PIN A1
#define SIGNAL_PIN 12
#define PUMP_PIN 13

DHT dht(DHT_PIN, DHT_TYPE);
float temperature, humidity;
uint16_t rain;
uint16_t moisture;

// Arduino setup function
void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(FC_ANALOG_PIN, INPUT);
  pinMode(SIGNAL_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
}

// Arduino loop function
void loop() {
  delay(1000);

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  rain = analogRead(FC_ANALOG_PIN);
  rain = map(rain, 0, 1023, 100, 0);
  if(rain < 5)
    rain = 0;

  moisture = analogRead(CSM_PIN);
  moisture = map(moisture, 0, 1023, 100, 0);

  Serial.print(temperature);
  Serial.print(',');
  Serial.print(humidity);
  Serial.print(',');
  Serial.print(rain);
  Serial.print(',');
  Serial.println(moisture);

  digitalWrite(PUMP_PIN, digitalRead(SIGNAL_PIN));
}
