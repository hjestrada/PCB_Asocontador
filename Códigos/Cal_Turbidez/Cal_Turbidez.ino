#include <LiquidCrystal.h>

LiquidCrystal lcd(3, 2, 4, 5, 6, 7);

#define SensorPin A2

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Medicion Voltaje");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  
  //set the resolution to 10 bits (0-1024)
  analogReadResolution(10);
}

void loop() {
  // Lee el valor de voltaje en el pin A0
  float voltage = analogRead(SensorPin) * (5.0 / 1024.0);

  // Muestra el valor de voltaje en la pantalla LCD
  lcd.print("Voltaje A0: ");
  lcd.print(voltage, 4);

  delay(1000);  // Actualizar el valor cada segundo
  lcd.clear();
}
