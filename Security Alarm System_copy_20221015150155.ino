#define DECODE_NEC


#include <IRremote.hpp>
#include <ArxContainer.h>
#include <SPI.h>
#include <LiquidCrystal.h>
#include <NewTone.h>
#include <NewPing.h>


const int IRPin = 3;

const int red_rgb = 9;
const int green_rgb = 6;
const int blue_rgb = 5;

const int trigger = 8;
const int echo = 4;
const int distance = 150;

const int speaker = 2;

const int power = 0x45;
const int one = 0xC;
const int two = 0x18;
const int nine = 0x4A;
const int seven = 0x42;
const int funcbutton = 0x47;

bool getOutLoop = false;

int counter = 0;



LiquidCrystal lcd(10);
NewPing sonar(trigger, echo, distance);

arx::vector<int> password {one, nine, seven, nine};

void (*resetFunc)(void) = 0;




void setup() {
  // put your setup code here, to run once:

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  IrReceiver.begin(IRPin, ENABLE_LED_FEEDBACK);

  pinMode(red_rgb, OUTPUT);
  pinMode(green_rgb, OUTPUT);
  pinMode(blue_rgb, OUTPUT);

  lcd.print("Press power");
  lcd.setCursor(0, 1);
  lcd.print("to start");


  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  while (getOutLoop == false){
  static unsigned int lastButtonPushed;
  lighting(250, 20, 100);
  if (IrReceiver.decode()) {
    unsigned int buttonPushed = IrReceiver.decodedIRData.command;
    if (buttonPushed != lastButtonPushed && IrReceiver.decodedIRData.protocol == NEC) {
      switch (IrReceiver.decodedIRData.command) {
        case power:
          lcd.clear();
          options();
          getOutLoop = true;
          break;
        default:
          lcd.setCursor(0, 0);
          lcd.print("Please press");
          lcd.setCursor(0, 1);
          lcd.print("power to start");
      }
      IrReceiver.printIRResultShort(&Serial);
      lastButtonPushed = buttonPushed;
    }
    IrReceiver.resume();
  }
  }
}
void options() {
  lcd.clear();
  char options[] = "1 to arm and 2 to reset. ";
  bool condition = true;
  while (condition == true) {
    lcd.setCursor(0, 0);
    lighting(100, 100, 100);
    for (int i = 0; i < sizeof(options) - 1; i++) {
      lcd.print(options[i]);
      delay(250);
      if (i > 16) {
        lcd.autoscroll();
      }
      if (IrReceiver.decode()) {
        break;
      }
    }
    if (IrReceiver.decodedIRData.protocol == NEC) {
      switch (IrReceiver.decodedIRData.command) {
        case one:
          lcd.clear();
          lcd.noAutoscroll();
          armTheWeapon();
          condition = false;
          break;
        case two:
          lcd.clear();
          lcd.noAutoscroll();
          theGreatReset();
          condition = false;
          break;
      }
      IrReceiver.resume();
    }   
  }
}
void theGreatReset() {
  for (int i = 3; i != 0; i--) {
    lcd.setCursor(0, 0);
    lcd.print("Reset in " + String(i));
    delay(500);
  }
  resetFunc();
}
void armTheWeapon() {
  lcd.print("Armed");
  lighting(250, 0, 0);
  bool notDinged = true;
  unsigned int timeed;
  while (notDinged == true){
    delay(50);
    unsigned int ding = sonar.ping(); 
    if (ding < 150){
      lcd.clear();
      lcd.print("INTRUDER!"); 
      timeed = millis();             
      notDinged = false;

    }
  }
  while (millis() - timeed < 3000){
    NewTone(speaker, 2000);
    lighting(250, 150, 50);
  }
  lcd.clear();
  noNewTone(speaker);
  IrReceiver.resume();
  counter1();

  
  

}
void lighting(int red, int green, int blue){
  analogWrite(red_rgb, red);
  analogWrite(blue_rgb, blue);
  analogWrite(green_rgb, green);
}
void counter1(){
  counter++;
  unsigned int startTime = millis();
  lighting(0, 250, 0);
  while (millis() - startTime < 3000) {
    lcd.setCursor(0, 0);
    lcd.print("Caught " + String(counter) + " time(s)");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1 to rearm");
  lcd.setCursor(0, 1);
  lcd.print("2 to reset");
  while (true){
    static unsigned lastButtonPushed;
    if (IrReceiver.decode()) {
       unsigned int buttonPushed = IrReceiver.decodedIRData.command;
       if (buttonPushed != lastButtonPushed && IrReceiver.decodedIRData.protocol == NEC) {
         switch(IrReceiver.decodedIRData.command){
           case one:
              lcd.clear();
              armTheWeapon();
              break;
            case two:
              lcd.clear();
              theGreatReset();
              break;
            default:
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Please choose");
              lcd.setCursor(0, 1);
              lcd.print("1 or 2");
         }
         lastButtonPushed = buttonPushed;
       }
       IrReceiver.resume();
    }


  }
}
