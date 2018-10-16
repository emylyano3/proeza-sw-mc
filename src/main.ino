#include <ESPDomotic.h>

#ifdef ESP01
// usable pins GPIO2 (GPIO3 if using SERIAL_TX_ONLY)
const uint8_t SWITCH_PIN  = 3;
const uint8_t RELAY_PIN   = 2;
const uint8_t TX_PIN      = 1;
#elif NODEMCUV2
// usable pins D0,D1,D2,D5,D6,D7 (D10 is TX (GPIO1), D9 is RX (GPIO3), D3 is GPIO0, D4 is GPIO2, D8 is GPIO15)
const uint8_t SWITCH_PIN  = D5;
const uint8_t RELAY_PIN   = D1;
const uint8_t LED_PIN     = D7;
#else
// usable pins GPIO 4,5,12,13,14,16
const uint8_t SWITCH_PIN  = 4;
const uint8_t RELAY_PIN   = 5;
const uint8_t LED_PIN     = 12;
#endif

Channel _light ("B", "Relay", RELAY_PIN, OUTPUT, HIGH);
Channel _switch ("A", "Switch", SWITCH_PIN, INPUT, LOW, &_light);

template <class T> void log (T text) {
  #ifdef LOGGING
  Serial.print("*SW: ");
  Serial.println(text);
  #endif
}

template <class T, class U> void log (T key, U value) {
  #ifdef LOGGING
  Serial.print("*SW: ");
  Serial.print(key);
  Serial.print(": ");
  Serial.println(value);
  #endif
}

ESPDomotic  _domoticModule;
uint8_t     _prevSwithcState = LOW;

void setup() {
#ifdef ESP01
  //to avoid using pin 0 as input
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, TX_PIN);
#else
  Serial.begin(115200);
#endif
  delay(500);
  Serial.println();
  log("Starting module");
  String ssid = "Light switch " + String(ESP.getChipId());
  _domoticModule.setPortalSSID(ssid.c_str());
  #ifndef ESP01
  _domoticModule.setFeedbackPin(LED_PIN);
  #endif
  _domoticModule.setMqttConnectionCallback(mqttConnectionCallback);
  _domoticModule.setMqttMessageCallback(receiveMqttMessage);
  _domoticModule.setModuleType("light");
  _domoticModule.addChannel(&_switch);
  _domoticModule.addChannel(&_light);
  _domoticModule.init();
}

void loop() {
  _domoticModule.loop();
  processInput();
}

void processInput() {
  int read = digitalRead(_switch.pin);
  if (read != _prevSwithcState) {
    log(F("Input channel state has changed"), _switch.name);
    _prevSwithcState = read;
    _switch.slave->state = _switch.slave->state == LOW ? HIGH : LOW;
    digitalWrite(_switch.slave->pin, _switch.slave->state);
    _domoticModule.getMqttClient()->publish(_domoticModule.getChannelTopic(&_light, "feedback/state").c_str(), _switch.slave->state == LOW ? "1" : "0");
    log(F("Output channel state changeed to"), _switch.slave->state == LOW ? "1" : "0");
  }
}

void mqttConnectionCallback() {
  // no additional subsription is needed
}

void receiveMqttMessage(char* topic, uint8_t* payload, unsigned int length) {
  // no additional message to process
}