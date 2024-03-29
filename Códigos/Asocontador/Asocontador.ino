#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>  // Incluye la biblioteca WiFiManager
#include <math.h>

LiquidCrystal lcd(3, 2, 4, 5, 6, 7);

#define Turbidy_sensor A2
float Tension = 0.0;
float NTU = 0.0;
#define SensorPin A0
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

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
const char* mqttTopic = "Asocontador/MonitorPrincipal";

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
    // Inicia WiFiManager para configuración de WiFi
  WiFiManager wifiManager;
  wifiManager.autoConnect("Asocontador_Config");
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

  DynamicJsonDocument doc(256);
  doc["pH"] = phValue;
  doc["Turbidity"] = turbidityValue;
  doc["Chlorine"] = chlorineValue;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  client.publish(mqttTopic, jsonBuffer);
  Serial.println("Datos enviados al broker MQTT:");
  Serial.println(jsonBuffer);

  delay(1000);  // Actualizar los valores cada segundo
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
      delay(5000);
    }
  }
}

float measurePH() {
   float pH;
  // Lee el valor de voltaje en el pin A0
  float voltage = analogRead(SensorPin) * (5.0 / 1024.0);
  pH = 1.4151 * (voltage) + 2.9487;
   return pH;
}

float measureTurbidity() {
    // float turbidez;
   //    float voltage = analogRead(Turbidy_sensor) * (5.0 / 1024.0);

 // turbidez = -108.61 * (voltage) + 1148.1;
 // return turbidez;
float turbidez;
 Tension = 0;  
  Tension = analogRead(Turbidy_sensor)/1024*5; // Mapeo de la lectura analógica  
  //Para compensar el ruido producido en el sensor tomamos 500 muestras y obtenemos la media  
  for(int i=0; i<500; i++)  
    {  
      Tension += ((float)analogRead(Turbidy_sensor)/1024)*5;  
    }  
    Tension = Tension/500;  
    Tension = redondeo(Tension,1);  
    //Para ajustarnos a la gráfica de la derecha  
    if(Tension < 2.5){  
      turbidez = 3000;  
    }else{  
             turbidez = -1120.4 * (Tension * Tension) + 5742.3 * Tension - 4352.9;   
  
    }  
return turbidez;
}

float measureChlorine() {
 float chlorine=0; //Cambiar Al momento de conectar el sensor.
 // int sensorValue = analogRead(ChlorinePin);
//  float voltage = sensorValue * (5.0 / 1024.0);
 // float chlorine = calibracionCloroK * voltage + calibracionCloroB;
  return chlorine;
}

float redondeo(float p_entera, int p_decimal)
{
  float multiplicador = powf( 10.0f, p_decimal);  //redondeo a 2 decimales
  p_entera = roundf(p_entera * multiplicador) / multiplicador;
  return p_entera;
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
