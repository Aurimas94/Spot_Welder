#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define Address_For_Pulse_Width 1
#define Address_For_Weld_Time   3

// Declaration for an SSD1306 display )
Adafruit_SSD1306 display(128, 64, &Wire);

// Pin-Out declarations
const byte PinSW = 4;   // Rotary Encoder Switch
const byte PinDT = 3;    // DATA signal
const byte PinCLK = 2;    // CLOCK signal
const byte Driver_Pin = 10;
const byte FootPedal = 9;

// Variables to debounce Rotary Encoder
long TimeOfLastDebounce = 0;
long LastDebounce = 0;
int DelayofDebounce = 0.01;
int DebounceDelay = 160;

volatile byte State = LOW;
unsigned long currentMillis = 0; 
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long EndMillis = 0;
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int BState; 
 
// Store previous Pins state
int PreviousCLK;   
int PreviousDATA;
bool ValueChange = false;
int displaycounter = 0; // Store current counter value
int counter = 0;
bool Pulse_Width_Menu = false;
bool Weld_Time_Menu = false;
bool Button_pushed = false;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

uint8_t PWM_Val = 0;
uint8_t WT_Val = 0;


void setup() 
{
  PreviousCLK = 0;
  PreviousDATA = 0;
  Serial.begin(9600);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    delay(100);
    PWM_Val = EEPROM.read(Address_For_Pulse_Width);
    WT_Val = EEPROM.read(Address_For_Weld_Time);
    delay(100);
    pinMode(Driver_Pin, OUTPUT);
    pinMode(FootPedal, INPUT);
    attachInterrupt(0, Zero_Crossing, RISING);
    display.clearDisplay();
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.display();      // Show initial text
}
void loop() {
  buttonState = digitalRead(FootPedal);
      if(buttonState != lastButtonState) 
      {
        //reset the debouncing timer
        lastDebounceTime = millis();
       }


       if ((millis() - lastDebounceTime) > debounceDelay) 
       {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (buttonState != BState) {
      BState = buttonState;

      // only toggle the LED if the new button state is HIGH
      if (BState == LOW) {
        currentMillis = millis(); 
        EndMillis = currentMillis;  // Remember the time
        while((EndMillis - currentMillis <= WT_Val))
          { 
            State = HIGH;
            EndMillis = millis();
          }
         State = LOW;
      }
    }
  }
  lastButtonState = buttonState;
  // If enough time has passed check the rotary encoder
  if ((millis() - TimeOfLastDebounce) > DelayofDebounce) 
    {
      check_rotary();  // Rotary Encoder check routine below
      PreviousCLK = digitalRead(PinCLK);
      PreviousDATA = digitalRead(PinDT);
      TimeOfLastDebounce = millis();  // Set variable to current millis() timer
    }
  if ((millis() - LastDebounce) > DebounceDelay ) 
    {
      if(digitalRead(PinSW) == LOW)
        {
          Button_pushed = true;
        }
      else 
        {
          Button_pushed = false;
        }
      LastDebounce = millis();  // Set variable to current millis() timer
    }
  if(ValueChange == true)
  { 
    if(displaycounter == 0)
    {
      display.clearDisplay();
      display.setCursor(0, 10);
      display.write(16);
      display.setCursor(15, 10);
      display.println(F("Pulse Width"));
      display.setCursor(85, 10);
      display.println(String(PWM_Val)+F(" %"));
      display.setCursor(15, 20);
      display.println(F("Weld Time"));
      display.setCursor(75, 20);
      display.println(String(WT_Val)+F(" ms"));
      display.display();      // Show initial text
      
    }
    else if(displaycounter == 1)
    {
      display.clearDisplay();
      display.setCursor(15, 10);
      display.println(F("Pulse Width"));
      display.setCursor(85, 10);
      display.println(String(PWM_Val)+F(" %"));
      display.setCursor(0, 20);
      display.write(16);
      display.setCursor(15, 20);
      display.println(F("Weld Time"));
      display.setCursor(75, 20);
      display.println(String(WT_Val)+F(" ms"));
      display.display();      // Show initial text
    }
    
    else if(displaycounter > 2)
    {
      displaycounter = 1;
    }
    
    ValueChange = false;
  
  }
  if (Button_pushed && displaycounter == 0) 
        {
          Pulse_Width_Menu = true;
          Weld_Time_Menu = false;
          display.clearDisplay();
          display.setCursor(15, 10);
          display.println(F("Pulse Width"));
          display.display();      // Show initial text
          displaycounter = PWM_Val;
          while(Pulse_Width_Menu == true)
          {
          if ((millis() - TimeOfLastDebounce) > DelayofDebounce) 
              {
                check_rotary();  // Rotary Encoder check routine below
                PreviousCLK = digitalRead(PinCLK);
                PreviousDATA = digitalRead(PinDT);             
                TimeOfLastDebounce = millis();  // Set variable to current millis() timer
              }
            if ((millis() - LastDebounce) > DebounceDelay ) 
              {
                if(digitalRead(PinSW) == LOW)
                {
                  PWM_Val = displaycounter;
                  EEPROM.update(Address_For_Pulse_Width, PWM_Val);
                  Pulse_Width_Menu = false;
                  ValueChange = false;
                  Button_pushed = false;
                  displaycounter = 0; 
                }
                
                LastDebounce=millis();  // Set variable to current millis() timer
              }
              if(ValueChange == true)
                {
                  display.clearDisplay();
                  display.setCursor(15, 10);
                  display.println(F("Pulse Width"));
                  display.setCursor(85, 10);
                  display.println(String(displaycounter)+F(" %"));
                  display.display();      // Show initial text
                  ValueChange = false;
                }
                
          }      
      display.clearDisplay();
      display.setCursor(0, 10);
      display.write(16);
      display.setCursor(15, 10);
      display.println(F("Pulse Width"));
      display.setCursor(85, 10);
      display.println(String(PWM_Val)+F(" %"));
      display.setCursor(15, 20);
      display.println(F("Weld Time"));
      display.setCursor(75, 20);
      display.println(String(WT_Val)+F(" ms"));
      display.display();      // Show initial text         
        }           
   if (Button_pushed && displaycounter == 1) 
        {
          Pulse_Width_Menu = false;
          Weld_Time_Menu = true;
          display.clearDisplay();
          display.setCursor(15, 10);
          display.println(F("Weld Time"));
          display.display();      // Show initial text
          displaycounter = WT_Val;
          while(Weld_Time_Menu == true)
          {
          if ((millis() - TimeOfLastDebounce) > DelayofDebounce) 
              {
                check_rotary();  // Rotary Encoder check routine below
                PreviousCLK = digitalRead(PinCLK);
                PreviousDATA = digitalRead(PinDT);             
                TimeOfLastDebounce = millis();  // Set variable to current millis() timer
              }
            if ((millis() - LastDebounce) > DebounceDelay ) 
              {
                if(digitalRead(PinSW) == LOW)
                {
                  WT_Val = displaycounter;
                  EEPROM.update(Address_For_Weld_Time, WT_Val);
                  Weld_Time_Menu = false;
                  ValueChange = false;
                  Button_pushed = false;
                  displaycounter = 0; 
                }
                
                LastDebounce = millis();  // Set variable to current millis() timer
              }
              if(ValueChange == true)
                {
                  display.clearDisplay();
                  display.setCursor(15, 10);
                  display.println(F("Weld Time"));
                  display.setCursor(75, 10);
                  display.println(String(displaycounter)+F(" ms"));
                  display.display();      // Show initial text
                  ValueChange = false;
                }
                
          }
      display.clearDisplay();
      display.setCursor(15, 10);
      display.println(F("Pulse Width"));
      display.setCursor(85, 10);
      display.println(String(PWM_Val)+F(" %"));
      display.setCursor(0, 20);
      display.write(16);
      display.setCursor(15, 20);
      display.println(F("Weld Time"));
      display.setCursor(75, 20);
      display.println(String(WT_Val)+F(" ms"));
      display.display();      // Show initial text
          }
}


// Check if Rotary Encoder was moved
void check_rotary() 
{
  if ((PreviousCLK == 1) && (PreviousDATA == 0)) {
      if ((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 1)) 
      {
        if(displaycounter <= 0)
        {
        displaycounter++;
        }
        ValueChange = true;
      }
      if ((digitalRead(PinCLK) == 0) && (digitalRead(PinDT) == 0)) 
      {
        if(displaycounter > 0)
        {
        displaycounter--;
        }
        ValueChange = true; 
      }
    }
  if ((PreviousCLK == 0) && (PreviousDATA == 0)) 
      {
      if ((digitalRead(PinCLK) == 1) && (digitalRead(PinDT) == 0)) 
      {
        if(displaycounter < 150)
        {
        displaycounter++;
        }
        ValueChange = true; 
      } 
      if ((digitalRead(PinCLK) == 1) && (digitalRead(PinDT) == 1)) 
      {
        if(displaycounter > 0)
        {
        displaycounter--;
        }
        ValueChange = true; 
      }
    }            
 }
  void Zero_Crossing()
  {
    
    if(State == HIGH)
    {
       delayMicroseconds(100);
       digitalWrite(Driver_Pin, HIGH);
       delayMicroseconds(5000);
       digitalWrite(Driver_Pin, LOW);
    }
    
  }
