#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <WiFi.h>

LiquidCrystal lcd(3, 2, 4, 5, 6, 7);

#define SensorPin A0
#define TurbidityPin A2
#define ChlorinePin A1
unsigned long int avgValue;
float b;
int buf[10], temp;

// Calibración para la conversión de voltaje a UNT (turbidez)
float calibracionTurbidezK = 1.0;
float calibracionTurbidezB = 0.0;

// Calibración para la conversión de voltaje a mg/L (cloro residual)
float calibracionCloroK = 1.0;
float calibracionCloroB = 0.0;

const char* ssid = "Teranet Estrada";
const char* password = "pitalito2023@";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
const char* mqttTopic = "Asocontador/lectura";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
//set the resolution to 10 bits (0-1024)
  analogReadResolution(10);
  lcd.begin(16, 2);
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
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  lcd.setCursor(0, 0);
  float phValue = measurePH();
  float turbidityValue = measureTurbidity();
  float chlorineValue = measureChlorine();
  displayValues(phValue, turbidityValue, chlorineValue);

  char message[50];
  snprintf(message, sizeof(message), "pH:%.2f, Turb:%.2f, Cl:%.2f", phValue, turbidityValue, chlorineValue);

  client.publish(mqttTopic, message);
  // Impresión de los datos en el puerto serie
  Serial.println("Datos enviados al broker MQTT:");
  Serial.println(message);

  delay(1000);  // Actualizar los valores cada segundo
}

void setup_wifi() {
  delay(10);
  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conexión WiFi...");
  }
  Serial.println("Conexión WiFi exitosa");
}

void reconnect() {
  // Bucle hasta que estemos reconectados
  while (!client.connected()) {
    Serial.print("Conexión MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Conectado al broker MQTT");
    } else {
      Serial.print("Error de conexión, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

float measurePH() {
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }

  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  avgValue = 0;
  for (int i = 2; i < 8; i++) {
    avgValue += buf[i];
  }

 float phValue = (float)avgValue * 3.3 / 1024 / 6;
  phValue = 3.5 * phValue;

  return phValue;
}

float measureTurbidity() {
  int sensorValue = analogRead(TurbidityPin);
  float voltage = sensorValue * (5.0 / 1024.0);

  // Ajustar la calibración para convertir a UNT
  float turbidity = calibracionTurbidezK * voltage + calibracionTurbidezB;

  return turbidity;
}

float measureChlorine() {
  int sensorValue = analogRead(ChlorinePin);
  float voltage = sensorValue * (5.0 / 1024.0);

  // Ajustar la calibración según tu sensor de cloro residual
  float chlorine = calibracionCloroK * voltage + calibracionCloroB;

  return chlorine;
}

void displayValues(float pH, float turbidity, float chlorine) {
  lcd.setCursor(0, 0);  // Primera fila
  lcd.print("pH: ");
  lcd.print(pH, 2);
  lcd.setCursor(11, 0);
  lcd.print("Unid");
  lcd.setCursor(0, 1);  // Segunda fila
  lcd.print("Turb: ");
  lcd.print(turbidity, 2);
  lcd.setCursor(12, 1);
  lcd.print("UNT");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);  // Mostrar el cloro residual
  lcd.print("Cl: ");
  lcd.print(chlorine, 2);
  lcd.setCursor(10, 0);
  lcd.print("mg/L");
  delay(3000);
  lcd.clear();
}
