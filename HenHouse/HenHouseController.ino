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

// LCD constants
const int SCREEN_ROWS = 2;
const int SCREEN_COLUMS = 16;

unsigned long time = 0;
unsigned long timeStarted = 0;
unsigned long timeHeld = 0;
unsigned long holdTimeStart = 0;

bool isHolding = false;
bool isOpening = true;

short jsXstate = 0; // Joy stick X state
short jsYstate = 0; // Joy stick Y state

short mPos = 0;

// Timers in minutes

unsigned short lightState = 0;
unsigned short lightOnTimer = 1080;
unsigned short lightOffTimer = 600;

bool pistonAutoOpen = false;
bool pistonAutoClose = false;
unsigned short pistonOpenTimer = 480;
unsigned short pistonClosedTimer = 1260;

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

const short menuSize = sizeof(menuItems) / 16;

/*typedef void (*vvFunction)();

vvFunction table[] =  {
    controlPiston, 
    controlPiston,
};*/

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

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
    
    lcd.begin(SCREEN_COLUMS, SCREEN_ROWS);

    lcd.createChar(0, upArrow);
    lcd.createChar(1, downArrow);
}
void loop()
{
    mainMenu();
}

void mainMenu()
{
    bool first = true;
    while(true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;
    
        short newPos = mPos;
        if (jsXstate - X_DEADZONE > 0 && newPos < 3) {
            newPos++;
        } else if (jsXstate + X_DEADZONE < 0 && newPos > 0) {
            newPos--;
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
            newPos = -1;
        }
        
        //Update screen
        if (mPos != newPos || first) {
            lcd.clear();
            if (newPos == -1)
            newPos = 0;
            for (int y = 0; y < SCREEN_ROWS; y++) {
                lcd.setCursor(1, y);
                lcd.print(menuItems[y + newPos]);
            }
            lcd.setCursor(0, 0);
            lcd.print(newPos != 0 ? (char)0 : ' ');
            lcd.setCursor(0, SCREEN_ROWS - 1);
            lcd.print(newPos != 3 ? (char)1 : ' ');
            
            mPos = newPos;
            delay(150);
            first = false;
        }
        delay(100);
    }
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

const char lightMenu[4][16] = {
    " Mode ", //ON, OFF, AUTO
    " On time ",
    " Off time ",
    " Back ",
};

int lightMenuSize = 4; 

const char lightStates[3][4] {
    "Auto",
    "On",
    "Off"
};

void setLightTimer()
{
    mPos = 0;
    short yPos = 0;
    bool first = true;

    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;
           
        short xPos = mPos;

        if(yPos == 0) {
            if (jsXstate - X_DEADZONE > 0 && xPos < lightMenuSize - 1) {
                xPos++;
            } else if (jsXstate + X_DEADZONE < 0 && xPos > 0) {
                xPos--;
            } if (jsYstate - Y_DEADZONE > 0) {
                yPos = 1;
            } else if (jsYstate + Y_DEADZONE < 0) {
                break;
            }
        }
        else if(yPos == 1) {
            if(xPos == 0) {
                if (jsXstate - X_DEADZONE > 0) {
                    lightState = (lightState + 1) % 3;
                } else if (jsXstate + X_DEADZONE < 0) {
                    lightState = (lightState != 0 ? lightState - 1 : 3; 
                }
            } else if(xPos == 1) {
                if (jsXstate - X_DEADZONE > 0) {
                    if(lightOnTimer == 1430) {
                        lightOnTimer = 0;
                    } else {
                        lightOnTimer += 10;
                    }
                } else if (jsXstate + X_DEADZONE < 0) {
                    if(lightOnTimer == 0) {
                        lightOnTimer = 1430;
                    } else {
                        lightOnTimer -= 10;
                    }
                }
            } else if(xPos == 2) {
                if (jsXstate - X_DEADZONE > 0) {
                    if(lightOffTimer == 1430) {
                        lightOffTimer = 0;
                    } else {
                        lightOffTimer += 10;
                    }
                } else if (jsXstate + X_DEADZONE < 0) {
                    if(lightOffTimer == 0) {
                        lightOffTimer = 1430;
                    } else {
                        lightOffTimer -= 10;
                    }
                }
            } else if (jsYstate + Y_DEADZONE < 0) {
                yPos = 0;
            }
        }

        lcd.clear();
        for (int x = 0; x < SCREEN_ROWS; x++) {
            lcd.setCursor(1, x);
            lcd.print(lightMenu[x + xPos]);
        }
        if(0 <= xPos && xPos < 1) {
            lcd.setCursor(xPos, 6);
            lcd.print(lightStates[lightState]);
        }
        if(0 <= xPos && xPos < 2) {
            lcd.setCursor(1 - xPos, 8);
            printTimeLCD(lightOnTimer);
        }
        if(1 <= xPos && xPos < 3) {
            lcd.setCursor(2 - xPos, 9);
            printTimeLCD(lightOffTimer);
        }
        
        lcd.setCursor(0, 0);
        lcd.print(xPos != 0 ? (char)0 : ' ');
        lcd.setCursor(0, SCREEN_ROWS - 1);
        lcd.print(xPos != lightMenuSize - 1 ? (char)1 : ' ');
        
        delay(150);
    }
}

void printTimeLCD(unsigned short time) {
    lcd.print(time / 60);
    lcd.print(':');
    lcd.print(time % 60);
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
