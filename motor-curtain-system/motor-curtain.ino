#include <AccelStepper.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

///////////////////////
// Web server settings
///////////////////////

AsyncWebServer server(80);
const char* ssid = "";
const char* password =  ""; 
String hostname = "esp8266-motor-curtain";

///////////////////////
// Motor driver settings
///////////////////////

const int stepsPerRevolution = 2048;
int moveMotorFlag = 0;
int setMotorDirection = 1; // negative one = anti-clockwise

// ULN2003 Motor Driver Pins
#define IN1 5
#define IN2 4
#define IN3 14
#define IN4 12

AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

String moveMotor() {
  moveMotorFlag = 1;
  setMotorDirection = 1;
  StaticJsonDocument<200> doc;
  doc["status"] = "ok";
  String out;
  serializeJson(doc, out);
  return out;
}

String moveMotorCounter() {
  moveMotorFlag = 1;
  setMotorDirection = -1;
  StaticJsonDocument<200> doc;
  doc["status"] = "move counter ok";
  String out;
  serializeJson(doc, out);
  return out;
}

void setup() {
  // initialize the motor serial port
  Serial.begin(115200);
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(100);
  stepper.moveTo(stepsPerRevolution * 3);

  // start the NodeMCU as a server accessible by Wifi 
  WiFi.hostname(hostname.c_str());
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  
    delay(500);
    Serial.println("Waiting to connect to WiFi...");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", moveMotor());
  });
  server.on("/counter", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", moveMotorCounter());
  });
  server.begin();
  Serial.println("Server listening");
}

void loop() {
  // In each loop, program checks the state variables
  // to determine whether it needs to activate the 
  // motor and if so, what action it needs to accomplish
  if (setMotorDirection == -1 && moveMotorFlag == 1) {
    stepper.moveTo(-stepper.currentPosition());
    setMotorDirection = 0;
    Serial.println("Set counter direction");
  }

  if (setMotorDirection == 1 && moveMotorFlag == 1) {
    stepper.moveTo(stepsPerRevolution * 3);
    setMotorDirection = 0;
    Serial.println("Set clockwise direction");
  }

  if (stepper.distanceToGo() == 0) {
    moveMotorFlag == 0;
  }

  if (moveMotorFlag == 1) {
    stepper.run();
  }  
}

