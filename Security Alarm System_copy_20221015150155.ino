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
const int echo = 2;
const int distance = 25;

const int speaker = 12;

const int power = 0x45;
const int one = 0xC;
const int nine = 0x4A;
const int seven = 0x42;
const int funcbutton = 0x47;

LiquidCrystal lcd(10);
NewPing sonar(trigger, echo, distance);


void setup() {
  // put your setup code here, to run once:

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  IrReceiver.begin(IRPin, ENABLE_LED_FEEDBACK);

  pinMode(red_rgb, OUTPUT);
  pinMode(green_rgb, OUTPUT);
  pinMode(blue_rgb, OUTPUT);

  lcd.print("Press Power");
  lcd.setCursor(0, 1);
  lcd.print("to start");

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned int lastButtonPushed;
  if (IrReceiver.decode()) {
    unsigned int buttonPushed = IrReceiver.decodedIRData.command;
    if (buttonPushed != lastButtonPushed && IrReceiver.decodedIRData.protocol == NEC) {
      switch (IrReceiver.decodedIRData.command) {
        case power:
          options();
          break;
        default:
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Press power");
          lcd.setCursor(0, 1);
          lcd.print("button to start");
      }
      IrReceiver.printIRResultShort(&Serial);
      lastButtonPushed = buttonPushed;
    }
    IrReceiver.resume();
  }
}
void options(){
  lcd.clear();
  char line1[] = "Hello World";
  char line2[] = "Where you";
  while (true){
    lcd.setCursor(0, 0);
    for (int i = 0; i < sizeof(line2); i++) {
      lcd.print(line2[i]);
      delay(500);
    }
    static unsigned int lastButtonPushed;
  }
}