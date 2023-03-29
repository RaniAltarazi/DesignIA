#include <ezButton.h>
#include <LCD_I2C.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(0, 1);
ezButton button1(9);
ezButton button2(8);
LCD_I2C lcd(0x27, 16, 2);
const int HX711_dout = 2; 
const int HX711_sck = 3;
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;
unsigned long t = 0;
String unit[] = {"g","kg","oz","lbs"};
int currentU = 0;

void setup() {
  lcd.begin();
  lcd.backlight();
  Serial.begin(9600);
  BTSerial.begin(9600);
  Serial.println("Starting...");
  LoadCell.begin();
  float calibrationValue;
  #if defined(ESP8266)|| defined(ESP32)
  EEPROM.begin(512); 
  #endif
  EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
}

void loop() {
  button1.loop();
  button2.loop();
  if(button1.isPressed()){
    lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print("Button 1 Pressed");
    LoadCell.tareNoDelay();
  }
  if(button2.isPressed()){
    lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print("Button 2 Pressed");
    if(currentU<4){
      currentU = currentU +1;
    }
    if(currentU ==4){
      currentU = 0;
    }
    lcd.setCursor(13,0);
    
    lcd.print(unit[currentU]);
  }
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      //lcd.clear();

      lcd.setCursor(0,0);
      
      lcd.print(i);
      lcd.setCursor(13,0);
      lcd.print(unit[currentU]);
      
      //Serial.println(x);
      newDataReady = 0;
      t = millis();
    }
  }
  // receive command from serial terminal, send 't' to initiate tare operation:
  if (BTSerial.available()) {
      char data = Serial.read();
      Serial.println(data);
      if (data == 'a'){
      Serial.println("works");
      }

  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}
