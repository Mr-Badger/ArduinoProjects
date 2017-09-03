#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Joystick input pins
const int xPin = 0;
const int yPin = 1;

// Output pins
const int openPin = 5;
const int closePin = 6;
const int pwmPin = 9;
const int ledPin = 10;

// Other constants
const int X_DEADZONE = 40;
const int Y_DEADZONE = 200;
const int triggerTime = 1000;
const int holdTime = 25000;

unsigned long time = 0;
unsigned long timeStarted = 0;
unsigned long timeHeld = 0;
unsigned long holdTimeStart = 0;

bool isHolding = false;
bool isOpening = true;

short jsXstate = 0; // Joy stick X state
short jsYstate = 0; // Joy stick Y state

short menuPos = 0;

// Timers in minutes
unsigned short lightOnTimer = 0;
unsigned short lightOffTimer = 0;
unsigned short pistonOpenTimer = 0;
unsigned short pistonClosedTimer = 0;

// Allowed symbols LCD
// abcdefghijklmnopqrstuvwxyz
// .:,;-_^<>!\"'#$%&/()[]{}|@=-
uint8_t upArrow[8] = { 0x0, 0x4, 0xE, 0x1B, 0x11, 0x0, 0x0 };
uint8_t downArrow[8] = { 0x0, 0x0, 0x11, 0x1B, 0xE, 0x4, 0x0 };

const char menuItems[5][16] = {
    "Piston manual  ",
    "Piston timer   ",
    "Light manual   ",
    "Light timer    ",
    "---------------"
};

/*typedef void (*vvFunction)();

vvFunction table[] =  {
    controlPiston, 
    controlPiston,
};*/

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

const short menuSize = sizeof(menuItems) / 16;

/*********************************************************/
void setup()
{
    Serial.begin(115200);
    pinMode(pwmPin, OUTPUT);
    pinMode(ledPin, OUTPUT);

    pinMode(openPin, INPUT);
    pinMode(closePin, INPUT);
    digitalWrite(openPin, HIGH);
    digitalWrite(closePin, LOW);
    
    lcd.begin(16, 2);

    lcd.createChar(0, upArrow);
    lcd.createChar(1, downArrow);

    lcd.setCursor(0, 1);
    lcd.print((char)1);
    for (int y = 0; y < 2; y++) {
        lcd.setCursor(1, y);
        lcd.print(menuItems[y + menuPos]);
    }
}
/*********************************************************/
void loop()
{
    time = millis();
    jsXstate = analogRead(xPin) - 512;
    jsYstate = analogRead(yPin) - 512;

    mainMenu();
}

void mainMenu()
{
    short nPos = menuPos;
    if (jsXstate - X_DEADZONE > 0 && nPos < 3) {
        nPos++;
    } else if (jsXstate + X_DEADZONE < 0 && nPos > 0) {
        nPos--;
    } else if (jsYstate - Y_DEADZONE > 0) {
        //do nothing
    } else if (jsYstate + Y_DEADZONE < 0) {
        if (pos == 0) {
            controlPiston();
        } else if (pos == 1) {
            setPistonTimer();
        } else if (pos == 2) {
            controlLights();
        } else if (pos == 3) {
            setLightTimer();
        }
        nPos = -1;
    }

    //Update screen
    if (menuPos != nPos) {
        lcd.clear();
        if (nPos == -1)
            nPos = 0;
        for (int y = 0; y < 2; y++) {
            lcd.setCursor(1, y);
            lcd.print(menuItems[y + nPos]);
        }
        lcd.setCursor(0, 0);
        lcd.print(nPos != 0 ? (char)0 : ' ');
        lcd.setCursor(0, 1);
        lcd.print(nPos != 3 ? (char)1 : ' ');

        menuPos = nPos;
        delay(150);
    }
    delay(100);
}

void controlPiston()
{
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Piston  Mode");
    lcd.setCursor(0, 1);
    lcd.print("< Back");

    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        if (jsYstate - Y_DEADZONE > 0) {
            break;
        }

        if (isHolding) {
            analogWrite(pwmPin, 255);
            analogWrite(ledPin, 255);
            if ((time > holdTimeStart + holdTime) || (!isOpening && jsXstate + X_DEADZONE < 0) || (isOpening && jsXstate - X_DEADZONE > 0)) {
                analogWrite(ledPin, 0);
                isHolding = false;
            }
        } else if (jsXstate - X_DEADZONE > 0) {
            movePiston(jsXstate, false);
        } else if (jsXstate + X_DEADZONE < 0) {
            movePiston(jsXstate, true);
        } else {
            analogWrite(pwmPin, 0);
            resetTime();
        }
        delay(20);
    }
}

void controlLights()
{
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Light Mode");
    lcd.setCursor(0, 1);
    lcd.print("< Back");

    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        if (jsYstate - Y_DEADZONE > 0) {
            break;
        }
    }
}

void setPistonTimer()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Open  time");
    lcd.setCursor(0, 1);
    lcd.print("Close time");
    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;
        
        lcd.setCursor(0, 11);
        printTimeLCD(pistonOpenTimer);
        lcd.setCursor(1, 11);
        printTimeLCD(pistonClosedTimer);

        delay(100);
    }
}

void setLightTimer()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("On  time");
    lcd.setCursor(0, 1);
    lcd.print("Off time");
    int yPos = 0;
    int xPos = 0;
    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        lcd.setCursor(0, 9);
        printTimeLCD(lightOnTimer);
        lcd.setCursor(1, 9);
        printTimeLCD(lightOffTimer);
        
        
        if(yPos == 0) {
            if (jsXstate - X_DEADZONE > 0) {
                lightOnTimer += 10;
            } else if (jsXstate + X_DEADZONE < 0) {
                lightOnTimer -= 10;
            } else if (jsYstate - Y_DEADZONE > 0) {
                
            } else if (jsYstate + Y_DEADZONE < 0) {
                break;
            }
        }
        else if(yPos == 1) {
            if(xPos == 0) {
                if (jsXstate - X_DEADZONE > 0) {
                    lightOnTimer += 10;
                } else if (jsXstate + X_DEADZONE < 0) {
                    lightOnTimer -= 10;
                }
            } else if(xPos == 1) {
                if (jsXstate - X_DEADZONE > 0) {
                    lightOffTimer += 10;
                } else if (jsXstate + X_DEADZONE < 0) {
                    lightOffTimer -= 10;
                }
            } else if (jsYstate + Y_DEADZONE < 0) {
                yPos = 0;
            }
        }
        delay(100);
    }
}

void printTimeLCD(unsigned short time) {
    lcd.print(time/60);
    lcd.print(':');
    lcd.print(time%60);
}

void movePiston(int state, bool setOpen)
{
    setTimeElapsed();
    int volt = map(abs(state), 0, 512, 40, 255);
    analogWrite(pwmPin, volt);

    digitalWrite(openPin, setOpen ? HIGH : LOW);
    digitalWrite(closePin, setOpen ? LOW : HIGH);
    if (timeHeld > triggerTime) {
        isHolding = true;
        isOpening = setOpen;
        holdTimeStart = time;
        resetTime();
    }
}

void setTimeElapsed()
{
    if (timeStarted == 0) {
        timeStarted = time;
    }
    timeHeld = time - timeStarted;
}

void resetTime()
{
    timeHeld = 0;
    timeStarted = 0;
}
