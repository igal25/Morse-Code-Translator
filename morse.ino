#include <LiquidCrystal_I2C.h>
#include <hidboot.h>
#include <usbhub.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

char input_option = 'n'; // value: n - no value yet, b - button as input, k - keyboard as input

int LED_PIN = 8;
int BUTTON_PIN = 2;

char key_pressed;

class KbdRptParser : public KeyboardReportParser
{
    void PrintKey(uint8_t mod, uint8_t key);

  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);

    void OnKeyDown  (uint8_t mod, uint8_t key);
    void OnKeyUp  (uint8_t mod, uint8_t key);
    void OnKeyPressed(uint8_t key);
};

void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  Serial.print("DN ");
  PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);

  if (c)
    OnKeyPressed(c);
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  Serial.print("UP ");
  Serial.println(key);
  if (key==40) Serial.println("Enter");
}

void KbdRptParser::OnKeyPressed(uint8_t key)
{
  Serial.print("ASCII: ");
  Serial.println((char)key);
  key_pressed=key;
};

USB     Usb;
//USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

KbdRptParser Prs;
int lastState = LOW;  // the previous state from the input pin
int currentState;     // the current reading from the input pin
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;
int wasPressedOnce = 0;
void setup()
{
  Serial.begin(9600);
  lcd.init();// initialize the lcd 
  lcd.backlight();
  lcd.print("press:short/long");
  lcd.setCursor(0,1);
  lcd.print("button/keyboard");
  pinMode(2, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  while(input_option == 'n') {
    currentState = digitalRead(BUTTON_PIN);

    if(lastState == HIGH && currentState == LOW)        // button is pressed
    {
      wasPressedOnce = 1;
      pressedTime = millis();
    }
    else if(lastState == LOW && currentState == HIGH) { // button is released
      releasedTime = millis();
  
      long pressDuration = releasedTime - pressedTime;
      Serial.println(pressDuration);
      if( pressDuration < 1000 && pressedTime != 0 && releasedTime != 0) {
        input_option = 'b';
      }
        
  
      else if( pressDuration > 1000 && pressedTime != 0 && releasedTime != 0)  {
        input_option = 'k';
      }
        
    }
    // save the the last state
    lastState = currentState;
  }
  if(input_option == 'k')
  {
    lcd.clear();
    lcd.print("u choose keyboard");
  }
  if(input_option == 'b')
  {
    lcd.clear();
    lcd.print("u choose button");
  }
  delay(4000);
  HidKeyboard.SetReportParser(0, &Prs);
  key_pressed=0;
  lcd.clear();
    lcd.print("lets start");
  delay(2000);
}

  static void shortSignal(){
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
  }

  static void longSignal(){
      digitalWrite(LED_PIN, HIGH);
      delay(900);
      digitalWrite(LED_PIN, LOW);
      delay(300);
  }
String code = "";
int len = 0;
char ch;
char new_char;
unsigned long pres_len = 0, rel_time, pres_time = 0, old_time_len = 0, old_pres = 0, space = 0;
int unit_delay = 250;
int min_delay = 10;
String Word ="";
int endOfWord = 0;

char MakeString()
{
  if (pres_len < (unit_delay*3) && pres_len > 50)
  {
    return '.';                        //if button press less than 0.6sec, it is a dot
  }
  else if (pres_len > (unit_delay*3))
  {
    return '-';                        //if button press more than 0.6sec, it is a dash
  }
}

char Morse_decod()
{
  static String morse[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....",
  "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-",
  ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "!"
  };
  int i = 0;
  while (morse[i] != "!")
  {
    if (morse[i] == code)
    {
      Serial.print(char('A' + i));
      Serial.print(" ");
      code = "";
      return char('A' + i);
      break;
    }
    i++;
  }
  if (morse[i] == "!")
  {
    Serial.println("");
    Serial.println("This code is not exist!");
  }
  code = "";
}
  
void loop()
{
  Usb.Task();
  lcd.clear();
  lcd.print("your input: ");
  char key = 0;
  if(input_option == 'b') {
    
    label:
    while (digitalRead(BUTTON_PIN) == HIGH) {}
    old_pres = rel_time;
    pres_time = millis();
    digitalWrite(LED_PIN, HIGH);
    while (digitalRead(BUTTON_PIN) == LOW) {}
    rel_time = millis();
    digitalWrite(LED_PIN, LOW);
    pres_len = rel_time - pres_time;
    space = pres_time - old_pres;
    if (pres_len > min_delay)
    {
      code += MakeString();
    }
    while ((millis() - rel_time) < (unit_delay * 3))
    {
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        goto label;
      }
    }

    key = Morse_decod();
  }
  else {
    key = key_pressed;
  }
  
  if (key>'a')key=key-('a'-'A');
  if (key != 0) {
    lcd.print(key);
    delay(2000);
    if(key=='0'){
      longSignal();
      longSignal();
      longSignal();
      longSignal();
      longSignal();
    }
    else if(key == '1') {
      shortSignal();
      longSignal();
      longSignal();
      longSignal();
      longSignal();
    }
    else if(key == '2') {
      shortSignal();
      shortSignal();
      longSignal();
      longSignal();
      longSignal();
    }
    else if(key == '3') {
      shortSignal();
      shortSignal();
      shortSignal();
      longSignal();
      longSignal();
    }
    else if(key == '4') {
      shortSignal();
      shortSignal();
      shortSignal();
      shortSignal();
      longSignal();
    }
    else if(key == '5') {
      shortSignal();
      shortSignal();
      shortSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == '6') {
      longSignal();
      shortSignal();
      shortSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == '7') {
      longSignal();
      longSignal();
      shortSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == '8') {
      longSignal();
      longSignal();
      longSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == '9') {
      longSignal();
      longSignal();
      longSignal();
      longSignal();
      shortSignal();
    }
    else if(key == 'A') {
      shortSignal();
      longSignal();
    }
    else if(key == 'B') {
      longSignal();
      shortSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == 'C') {
      longSignal();
      shortSignal();
      longSignal();
      shortSignal();
    }
    else if(key == 'D') {
      longSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == 'E') {
      longSignal();
    }
    else if(key == 'F') {
      shortSignal();
      shortSignal();
      longSignal();
      shortSignal();
    }
    else if(key == 'G') {
      longSignal();
      longSignal();
      shortSignal();
    }
    else if(key == 'H') {
      shortSignal();
      shortSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == 'I') {
      shortSignal();
      shortSignal();
    }
    else if(key == 'J') {
      shortSignal();
      longSignal();
      longSignal();
      longSignal();
    }
    else if(key == 'K') {
      longSignal();
      shortSignal();
      longSignal();
    }
    else if(key == 'L') {
      shortSignal();
      longSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == 'M') {
      longSignal();
      longSignal();
    }
    else if(key == 'N') {
      longSignal();
      shortSignal();
    }
    else if(key == 'O') {
      longSignal();
      longSignal();
      longSignal();
    }
    else if(key == 'P') {
      shortSignal();
      longSignal();
      longSignal();
      shortSignal();
    }
    else if(key == 'Q') {
      longSignal();
      longSignal();
      shortSignal();
      longSignal();
    }
    else if(key == 'R') {
      shortSignal();
      longSignal();
      shortSignal();
    }
    else if(key == 'S') {
      shortSignal();
      shortSignal();
      shortSignal();
    }
    else if(key == 'T') {
      longSignal();
    }
    else if(key == 'U') {
      shortSignal();
      shortSignal();
      longSignal();
    }
    else if(key == 'V') {
      shortSignal();
      shortSignal();
      shortSignal();
      longSignal();
    }
    else if(key == 'W') {
      shortSignal();
      longSignal();
      longSignal();
    }
    else if(key == 'X') {
      longSignal();
      shortSignal();
      shortSignal();
      longSignal();
    }
    else if(key == 'Y') {
      longSignal();
      shortSignal();
      longSignal();
      longSignal();
    }
    else if(key == 'Z') {
      longSignal();
      longSignal();
      shortSignal();
      shortSignal();
    }
    
    key_pressed=0;
    key=0;
  }
  else
    delay(1000);
  
}
