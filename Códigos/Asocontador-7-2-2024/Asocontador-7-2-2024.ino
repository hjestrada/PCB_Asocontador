#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

LiquidCrystal lcd(3, 2, 4, 5, 6, 7);

#define Turbidy_sensor A2
float Tension = 0.0;
float NTU = 0.0;
#define SensorPin A0
#define ChlorinePin A1
unsigned long int avgValue;
float b;
int buf[10], temp;

float calibracionTurbidezK = 1.0;
float calibracionTurbidezB = 0.0;

float calibracionCloroK = 1.0;
float calibracionCloroB = 0.0;

float phValue, turbidityValue, chlorineValue;
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
const char* mqttTopic = "Asocontador/MonitorPrincipal";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
 
VerificacionInicial();
  // Intenta conectar o crea un punto de acceso si no puede conectar
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect("Asocontador_Config")) {
    Serial.println("Fallo al conectar al WiFi. Reinicia el dispositivo y configura manualmente.");
    delay(5000);
    Sininternet();
    ESP.restart();  // Cambio aquí: 'reset()' a 'restart()'
    delay(1000);
  }

  // Una vez conectado, muestra la dirección IP del punto de acceso (hotspot)
  Serial.begin(9600);
  Serial.println("Conectado al WiFi. IP del hotspot:");
  Serial.println(WiFi.softAPIP());
  analogReadResolution(10);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Tecnoparque");
  lcd.setCursor(2, 1);
  lcd.print("Nodo Pitalito");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calidad del agua");
  lcd.setCursor(2, 1);
  lcd.print("ASOCONTADOR");
  lcd.clear();
  client.setServer(mqttServer, mqttPort);
}
void VerificacionInicial(){
 lcd.begin(16, 2);
 lcd.setCursor(0, 0);
  lcd.print("Sin Internet");
  lcd.setCursor(0, 1);
  lcd.print("Error al iniciar");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Verificar Wifi");
  lcd.setCursor(0, 1);
  lcd.print("reinicie Tarjeta");
  delay(2000);
   }
void loop() {

  if (!client.connected()) {
    reconnect();
    displayValues(phValue, turbidityValue, chlorineValue);
    Sininternet();
  }

  lcd.setCursor(0, 0);
  phValue = measurePH();
  turbidityValue = measureTurbidity();
  chlorineValue = measureChlorine();
  displayValues(phValue, turbidityValue, chlorineValue);

  if (client.connected()) {
    DynamicJsonDocument doc(256);
    doc["pH"] = phValue;
    doc["Turbidity"] = turbidityValue;
    doc["Chlorine"] = chlorineValue;

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    client.publish(mqttTopic, jsonBuffer);
    Serial.println("Datos enviados al broker MQTT:");
    Serial.println(jsonBuffer);
  }

  delay(1000);
}
void Sininternet() {
  lcd.setCursor(0, 0);
  lcd.print("Sin Internet");
  delay(2000);
  lcd.clear();
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conexión MQTT...");
    if (client.connect("Monitor", mqttUser, mqttPassword)) {
      Serial.println("Conectado al broker MQTT");
    } else {
      Serial.print("Error de conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos");
      displayValues(phValue, turbidityValue, chlorineValue);
      delay(5000);
      Sininternet();
    }
  }
}

float measurePH() {
  float pH;
  float voltage = analogRead(SensorPin) * (5.0 / 1024.0);
  pH = 1.4151 * (voltage) + 2.9487;
  return pH;
}

float measureTurbidity() {
  float turbidez;
  float voltage = analogRead(Turbidy_sensor) * (5.0 / 1024.0);
  turbidez = (-108.61 * (voltage) + 1148.1)/1000;
  return turbidez;
}

float measureChlorine() {
  float chlorine = 0;
  return chlorine;
}

void displayValues(float pH, float turbidity, float chlorine) {
  lcd.setCursor(0, 0);
  lcd.print("pH: ");
  lcd.print(pH, 2);
  lcd.setCursor(11, 0);
  lcd.print("Unid");
  lcd.setCursor(0, 1);
  lcd.print("Tbz:");
  lcd.print(turbidity, 2);
  lcd.setCursor(13, 1);
  lcd.print("UNT");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cl: ");
  lcd.print(chlorine, 2);
  lcd.setCursor(10, 0);
  lcd.print("mg/L");
  delay(3000);
  lcd.clear();
}
