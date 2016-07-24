#include <math.h>
#include <ArduinoJson.h>
#include <IRremote.h>

#define LDR_ANALOG_PIN A0
#define TEMP_ANALOG_PIN A1
#define IR_EMIT_DIGITAL_PIN 3
#define IR_SENSOR_DIGITAL_PIN 8

IRrecv irrecv(IR_SENSOR_DIGITAL_PIN);
IRsend irsend;

decode_results results;

void setup()
{
  Serial.begin(9600);
  pinMode(IR_EMIT_DIGITAL_PIN, OUTPUT);
  irrecv.enableIRIn();
}

void loop()
{
  sensors();
}

void sensors() {
  sendMessage("temperature", calculateTemperature());
  delay(600);
  sendMessage("luminosity", calculateLuminosity());
  delay(600);

  String infraredSensor = decodeInfrared();
  if(infraredSensor.length() > 0) {
    sendMessage("infrared", infraredSensor);
  }
}

double calculateTemperature() {
  int rawValue = analogRead(TEMP_ANALOG_PIN);
  double resistance = (10240000 / rawValue) - 10000;

  // Steinhart-Hart Thermistor Equation
  double temp = log(resistance);
  temp = 1 / (0.001129148 + (0.000234125 * temp) + (0.0000000876741 * pow(temp,3)));

  // Celsius
  temp = temp - 273.15;

  return temp;
}

double calculateLuminosity() {
  double luminosity = analogRead(LDR_ANALOG_PIN);
  return (luminosity / 1024) * 100;
}

String decodeInfrared() {
  String infraredSensor = "";

  if (irrecv.decode(&results)) {
    infraredSensor = String(results.value, HEX);
    irrecv.resume();
  }

  return infraredSensor;
}

void sendMessage(String topic, double value) {
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = topic;
  message["value"] = value;

  message.printTo(Serial);
  Serial.println();
}

void sendMessage(String topic, String value) {
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = topic;
  message["value"] = value;

  message.printTo(Serial);
  Serial.println();
}

void serialEvent() {
  while (Serial.available()) {
    String rawMessage = Serial.readString();

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& message = jsonBuffer.parseObject(rawMessage);

    String topic = message["topic"];
    String value = message["value"];

    if(topic == "sender/infrared") {
      sendInfrared(value);
    }
  }
}

void sendInfrared(String value) {
  irsend.sendNEC(strtoul(value.c_str(), 0, 16), 32);
  irrecv.enableIRIn();
  irrecv.resume();
}
