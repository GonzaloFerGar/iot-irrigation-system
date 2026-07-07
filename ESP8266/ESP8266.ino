#include <ESP8266WiFi.h>
#include <ThingsBoard.h>

#define maxMessageLength 8
#define signalPin 4
#define ssid "<SSID>"
#define pass "<PASSWORD>"
#define server "<IP>"
#define token "<TOKEN>"

WiFiClient wifiClient;
int state = WL_IDLE_STATUS;
ThingsBoard tb(wifiClient);
bool subscribed = false;

// Method for initializing WiFi connection
void connectWiFi() {
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED)
    delay(1000);
}

// Method for reestablishing WiFi connection
void reconnectWiFi() {
  state = WiFi.status();
  if(state != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    while(state != WL_CONNECTED)
      delay(1000);
  }
}

// RPC method for processing watering switch change
RPC_Response processSwitchChange(const RPC_Data &data) {
  char params[10];
  serializeJson(data, params);
  String _params = params;
  if(_params == "true") {
    digitalWrite(signalPin, HIGH);
    return RPC_Response("wateringSwitch", "true");
  }
  else if(_params == "false") {
    digitalWrite(signalPin, LOW);
    return RPC_Response("wateringSwitch", "false");
  }
}

// RPC callbacks array for ThingsBoard subscription
const size_t callbacks_size = 1;
RPC_Callback callbacks[callbacks_size] = {
  { "wateringSwitch", processSwitchChange }
};

// Arduino setup function
void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  connectWiFi();
  pinMode(signalPin, OUTPUT);
}

// Arduino loop function
void loop() {
  delay(1000);

  if(state != WL_CONNECTED) {
    reconnectWiFi();
    return;
  }

  if(!tb.connected()) {
    subscribed = false;
    tb.connect(server, token);
    return;
  }

  if(!subscribed) {
    if(!tb.RPC_Subscribe(callbacks, callbacks_size)) {
      delay(1000);
      return;
    }
    subscribed = true;
  }

  while(Serial.available() > 0) {
    static char message[maxMessageLength];
    static unsigned int messagePos = 0;
    static unsigned int attribute = 0;

    char inByte = Serial.read();
    if(inByte != '\n' && (messagePos < maxMessageLength - 1)) {
      if(inByte != ',') {
        message[messagePos] = inByte;
        messagePos++;
      }
      else {
        message[messagePos] = '\0';
        
        if(attribute == 0)
          tb.sendTelemetryFloat("temperature", atof(message));
        if(attribute == 1)
          tb.sendTelemetryFloat("humidity", atof(message));
        if(attribute == 2)
          tb.sendTelemetryInt("rain", atoi(message));

        messagePos = 0;
        attribute++;
      }
    }
    else {
      message[messagePos] = '\0';
      tb.sendTelemetryInt("moisture", atoi(message));
      messagePos = 0;
      attribute = 0;
    }
  }

  tb.loop();
}
