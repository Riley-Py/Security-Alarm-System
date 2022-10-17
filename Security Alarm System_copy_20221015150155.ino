/*
Riley Cant
TEJ3M
Security Alarm System
October 17th, 2022
*/

//Needed for the IR
#define DECODE_NEC

/*Arx is for reading the passwords and SPI is for the shift register used
for reducing the LCD pins from 6 to 3.  NewTone/NewPing are needed for the ultrasonic/the speaker */
#include <IRremote.hpp>
#include <ArxContainer.h>
#include <SPI.h>
#include <LiquidCrystal.h>
#include <NewTone.h>
#include <NewPing.h>


const int IRPin = 3;

//Used for different colors for the RGB
const int red_rgb = 9;
const int green_rgb = 6;
const int blue_rgb = 5;

const int trigger = 8;
const int echo = 4;
const int distance = 150;

const int speaker = 2;

//Used with the remote
const int power = 0x45;
const int one = 0xC;
const int two = 0x18;

//Getting out of the main loop for the other loops
bool getOutLoop = false;

int counter = 0;


//Only need one pin as that is the latch pin
LiquidCrystal lcd(10);
NewPing sonar(trigger, echo, distance);

//Vector lists that are used for comparing the user input and the password (1979)
arx::vector<int> password{ 1, 9, 7, 9 };
arx::vector<int> userGuess;

//Used to reset the program when called
void (*resetFunc)(void) = 0;




void setup() {
  // put your setup code here, to run once:

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  //Starts the receiving process of the IR sensor
  IrReceiver.begin(IRPin, ENABLE_LED_FEEDBACK);

  pinMode(red_rgb, OUTPUT);
  pinMode(green_rgb, OUTPUT);
  pinMode(blue_rgb, OUTPUT);

  lcd.print("Press power");
  lcd.setCursor(0, 1);
  lcd.print("to start");
}

void loop() {
  // put your main code here, to run repeatedly:

  //Loop used to get out of the main loop
  while (getOutLoop == false) {

    //Using the button pressing logic from the lab done
    static unsigned int lastButtonPushed;
    lighting(250, 20, 100);

    /*This piece of logic below  will be seen throughout. Here's how it goes: when the IR receiver recieves a signal from the remote,
    it stores it as a button pushed and then it goes and sees if the button pushed isn't the same one as before and if the protocol
    is NEC (hex). If it is, it takes the command and switches it into a switch case.  If there isn't one, it defaults into a generic
    statement.  After all of this, it stores the button pressed as the last button pressed and the IR receiver will get rid of the value
    already receive in order to read new values (if they come)*/
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
        lastButtonPushed = buttonPushed;
      }
      IrReceiver.resume();
    }
  }
}

//Function for what the user should do
void options() {
  lcd.clear();
  char options[] = "1 to arm and 2 to reset. ";
  bool condition = true;

  //This is like the main loop (void loop).  There are many of these in the program
  while (condition == true) {
    lcd.setCursor(0, 0);
    lighting(100, 100, 100);

    //This will scroll the text from left to right until it receives a signal from the IR remote
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
    //Sees if it's a valid signal and then if it is, it will read the command and see what it should do
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

//Restarts the program after 1.5 seconds
void theGreatReset() {
  for (int i = 3; i != 0; i--) {
    lcd.setCursor(0, 0);
    lcd.print("Reset in " + String(i));
    delay(500);
  }
  resetFunc();
}

//Arms the security alarm system
void armTheWeapon() {
  //Prints that it's armed on the LCD
  lcd.print("Armed");
  lcd.setCursor(0, 1);
  lcd.print("Pass:");
  lighting(250, 0, 0);

  //Variables for other options
  bool notDinged = true;
  unsigned int timeed;
  int count = 0;

  //Resumes it just in case it will take the previous value
  IrReceiver.resume();
  //Loop to see if the alarm (or not valid password) is entered
  while (notDinged == true) {
    delay(50);
    unsigned int ding = sonar.ping();
    static unsigned int lastButtonPushed;
    //If the ding is less than 150, then it detects an intruder
    if (ding < 150) {
      lcd.clear();
      lcd.print("INTRUDER!");
      timeed = millis();
      notDinged = false;
    }
    /*Same as the very first loop, but some differences.  First, if it does detect a signal, 
    it will print it to the LCD screen and put the value into the userGuess vector.  It will 
    also increment count (which is use to place the numbers from left to right) and also makes it 
    so that you can't spam random numbers*/
    if (IrReceiver.decode()) {
      unsigned int buttonPushed = IrReceiver.decodedIRData.command;
      if (lastButtonPushed != buttonPushed && IrReceiver.decodedIRData.protocol == NEC) {
        lcd.setCursor(count + 5, 1);
        userGuess.push_back(translator(buttonPushed));
        lcd.print(userGuess.back());
        lastButtonPushed = buttonPushed;
        count++;
      }
      IrReceiver.resume();
    }
    /*If the size is equal between the vectors, then it checks if the elements are equal and if they are,
    it goes into the the disarm function.  If not, it berates you and forces you to re-enter the password*/
    if (userGuess.size() == password.size()) {
      if (userGuess == password) {
        disarm();
        notDinged = false;
        userGuess.clear();
      } else {
        lcd.clear();
        lcd.print("WRONG");
        delay(1000);
        lcd.setCursor(0, 0);
        lcd.print("Armed");
        lcd.setCursor(0, 1);
        lcd.print("Pass:");
        userGuess.clear();
      }
    }
  }
  //After detection, it plays an annoying speaker for 3 seconds
  while (millis() - timeed < 3000) {
    NewTone(speaker, 2000);
    lighting(250, 150, 50);
  }
  //After the annoying speaker, it resumes receiving the IR remote and goes to the counter function
  lcd.clear();
  noNewTone(speaker);
  IrReceiver.resume();
  counter1();
}
//Used for the RGB LEDs
void lighting(int red, int green, int blue) {
  analogWrite(red_rgb, red);
  analogWrite(blue_rgb, blue);
  analogWrite(green_rgb, green);
}
//Counts how many times that a pesron has been caught
void counter1() {
  counter++;
  unsigned int startTime = millis();
  lighting(0, 250, 0);
  //Displays the caught message on LCD for 3 seconds
  while (millis() - startTime < 3000) {
    lcd.setCursor(0, 0);
    lcd.print("Caught " + String(counter) + " time(s)");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1 to rearm");
  lcd.setCursor(0, 1);
  lcd.print("2 to reset");
  //Same as loop 1 and it basically resets the program
  while (true) {
    static unsigned lastButtonPushed;
    if (IrReceiver.decode()) {
      unsigned int buttonPushed = IrReceiver.decodedIRData.command;
      if (buttonPushed != lastButtonPushed && IrReceiver.decodedIRData.protocol == NEC) {
        switch (IrReceiver.decodedIRData.command) {
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
//Translates the hex from the IR and returns an integer
int translator(int command) {
  switch (command) {
    case 0x16:
      return 0;
      break;
    case 0xC:
      return 1;
      break;
    case 0x18:
      return 2;
      break;
    case 0x5E:
      return 3;
      break;
    case 0x8:
      return 4;
      break;
    case 0x1C:
      return 5;
      break;
    case 0x5A:
      return 6;
      break;
    case 0x42:
      return 7;
      break;
    case 0x52:
      return 8;
      break;
    case 0x4A:
      return 9;
      break;
  }
}
//Runs when the password was correctly entered
void disarm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Disarmed");
  lcd.setCursor(0, 1);
  lcd.print("1 to rearm, 2 to reset");
  //Same as loop one, but either rearms the weapon or resets the entire program
  while (true) {
    static unsigned int lastButtonPushed;
    if (IrReceiver.decode()) {
      unsigned int buttonPushed = IrReceiver.decodedIRData.command;
      if (lastButtonPushed != buttonPushed && IrReceiver.decodedIRData.protocol == NEC) {
        switch (buttonPushed) {
          case one:
            lcd.clear();
            armTheWeapon();
            break;
          case two:
            lcd.clear();
            theGreatReset();
            break;
        }
        lastButtonPushed = buttonPushed;
      }
      IrReceiver.resume();
    }
  }
}
