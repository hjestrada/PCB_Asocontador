#include <LiquidCrystal.h>

LiquidCrystal lcd(3, 2, 4, 5, 6, 7);

#define SensorPin A0
#define TurbidityPin A2
#define ChlorinePin A1  // Puerto analógico para el sensor de cloro residual
unsigned long int avgValue;
float b;
int buf[10], temp;

// Calibración para la conversión de voltaje a UNT (turbidez)
float calibracionTurbidezK = 1.0; // Ajusta según la calibración de tu sensor de turbidez
float calibracionTurbidezB = 0.0; // Ajusta según la calibración de tu sensor de turbidez

// Calibración para la conversión de voltaje a mg/L (cloro residual)
float calibracionCloroK = 1.0; // Ajusta según la calibración de tu sensor de cloro residual
float calibracionCloroB = 0.0; // Ajusta según la calibración de tu sensor de cloro residual

void setup() {
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
  Serial.begin(9600); // Iniciar la comunicación serial a 9600 baudios
 
}

void loop() {
  lcd.setCursor(0, 0);
  float phValue = measurePH();
  float turbidityValue = measureTurbidity();
  float chlorineValue = measureChlorine();
  displayValues(phValue, turbidityValue, chlorineValue);

  delay(1000);  // Actualizar los valores cada segundo
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

  float phValue = (float)avgValue * 5.0 / 1024 / 6;
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
