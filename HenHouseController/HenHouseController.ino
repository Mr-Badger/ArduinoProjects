#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>

// Joystick input pins
const int xPin = 0;
const int yPin = 1;

const int lightSensorPin = 2;
const int tempSensorPin = 3;

// Output pins
const int openPin = 5;
const int closePin = 6;
const int pwmPin = 9;
const int ledPin = 10;

// Other constants
const int X_DEADZONE = 40;
const int Y_DEADZONE = 200;
const int TRIGGER_TIME = 1000;
const int HOLD_TIME = 25000;

const int TIMER_STEP = 30;

// LCD constants
const int SCREEN_ROWS = 2;
const int SCREEN_COLUMS = 16;

unsigned long time = 0;
unsigned long timeStarted = 0;
unsigned long timeHeld = 0;
unsigned long holdTimeStart = 0;

bool isHolding = false;
bool isOpening = true;

int jsXstate = 0; // Joy stick X state
int jsYstate = 0; // Joy stick Y state

static bool JSRight() { return jsYstate + Y_DEADZONE < 0; }
static bool JSLeft()  { return jsYstate - Y_DEADZONE > 0; }
static bool JSUp()    { return jsXstate + X_DEADZONE < 0; }
static bool JSDown()  { return jsXstate - X_DEADZONE > 0; }

int xPos = 0;

// Stored values
byte lightState = EEPROM[0];
unsigned int lightOnTimer = EEPROM[1] * 60 + EEPROM[2];
unsigned int lightOffTimer = EEPROM[3] * 60 + EEPROM[4];

bool pistonAutoOpen = EEPROM[5];
bool pistonAutoClose = EEPROM[6];
unsigned int pistonOpenTimer = EEPROM[7] * 60 + EEPROM[8];
unsigned int pistonClosedTimer = EEPROM[9] * 60 + EEPROM[10];

uint8_t upArrow[8] = { 0x0, 0x4, 0xE, 0x1B, 0x11, 0x0, 0x0 };
uint8_t downArrow[8] = { 0x0, 0x0, 0x11, 0x1B, 0xE, 0x4, 0x0 };

const char menuItems[][16] {
    "Piston manual",
    "Piston timer",
    "Light timer"
};
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

const char pistonMenu[][16] {
    "Auto open ",
    "Auto close",
    "Open ti ",
    "Close ti",
    "Back"
};
const int pistonMenuSize = sizeof(pistonMenu) / sizeof(pistonMenu[0]);

const char lightMenu[][16] {
    "Mode",
    "On time ",
    "Off time",
    "Back"
};
const int lightMenuSize = sizeof(lightMenu)  / sizeof(lightMenu[0]);

const char lightStates[][5] {
    "Auto",
    "On",
    "Off"
};
const int lightStatesSize = sizeof(lightStates)  / sizeof(lightStates[0]);

const char onOffStates[][4] {
    "On",
    "Off"
};


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
    while (true) {
        time = millis();

        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        int newPos = xPos;
        if (JSDown() && newPos < menuSize - 1) {
            newPos++;
        } else if (JSUp() && newPos > 0) {
            newPos--;
        } else if (JSRight()) {
            if (newPos == 0) {
                controlPiston();
            } else if (newPos == 1) {
                setPistonTimer();
            } else if (newPos == 2) {
                setLightTimer();
            }
            newPos = -1;
        }

        //Update screen
        if (xPos != newPos || first) {
            lcd.clear();

            // Prints menu arrows
            lcd.setCursor(0, 0);
            lcd.print(newPos != 0 ? (char)0 : ' ');
            lcd.setCursor(0, SCREEN_ROWS - 1);
            lcd.print(newPos != menuSize - 1 ? (char)1 : ' ');
            
            if (newPos == -1)
                newPos = 0;
            for (int i = 0; i < SCREEN_ROWS; i++) {
                if(newPos + i < menuSize) {
                    lcd.setCursor(1, i);
                    lcd.print(menuItems[i + newPos]);
                }
            }

            xPos = newPos;
            first = false;
        }
        delay(100);
    }
}

void controlPiston()
{
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Piston mode");
    lcd.setCursor(0, 1);
    lcd.print("< Back");

    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        if (JSLeft()) {
            break;
        }

        if (isHolding) {
            analogWrite(pwmPin, 255);
            analogWrite(ledPin, 255);
            if ((time > holdTimeStart + HOLD_TIME) || (!isOpening && JSDown()) || (isOpening && JSUp())) {
                analogWrite(ledPin, 0);
                isHolding = false;
            }
        } else if (JSUp()) {
            movePiston(jsXstate, false);
        } else if (JSDown()) {
            movePiston(jsXstate, true);
        } else {
            analogWrite(pwmPin, 0);
            resetTime();
        }
        delay(20);
    }
}

void setPistonTimer()
{
    int yPos = 0;
    xPos = 0;
    bool updateScreen = true;

    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        if (updateScreen) {
            lcd.clear();
            
            // Print menu arrows
            lcd.setCursor(0, 0);
            lcd.print(xPos != 0 ? (char)0 : ' ');
            lcd.setCursor(0, SCREEN_ROWS - 1);
            lcd.print(xPos != pistonMenuSize - 1 ? (char)1 : ' ');
            
            for (int x = 0; x < SCREEN_ROWS; x++) {
                if(xPos + x < pistonMenuSize) {
                    lcd.setCursor(1, x);
                    lcd.print(pistonMenu[x + xPos]);
                }
            }
            
            // Print current configuration
            if (0 <= xPos && xPos < 1) {
                lcd.setCursor(12, xPos);
                lcd.print(onOffStates[pistonAutoOpen]);
            }
            if (0 <= xPos && xPos < 2) {
                lcd.setCursor(12, 1 - xPos);
                lcd.print(onOffStates[pistonAutoClose]);
            }
            if (1 <= xPos && xPos < 3) {
                lcd.setCursor(10, 2 - xPos);
                printTimeLCD(pistonOpenTimer);
            }
            if (2 <= xPos && xPos < 4) {
                lcd.setCursor(10, 3 - xPos);
                printTimeLCD(pistonClosedTimer);
            }

            // Print blinking cursor when editing 
            if(yPos == 1) {
                lcd.setCursor(strlen(pistonMenu[xPos]) + 1, 0);
                lcd.blink();
            }  else {
                lcd.noBlink();
            }
            
            delay(150);
            updateScreen = false;
        }

        delay(150);
        if (yPos == 0) {
             if (xPos == 4 && (JSLeft() || JSRight())) {
                xPos = 0;
                break;
            }
            if (JSDown() && xPos < pistonMenuSize - 1) {
                xPos++;
                updateScreen = true;
            } else if (JSUp() && xPos > 0) {
                xPos--;
                updateScreen = true;
            } else if (JSRight()) {
                yPos = 1;
                updateScreen = true;
            }
        } else if (yPos == 1) {
            if (xPos == 0) {
                if (JSUp()) {
                    pistonAutoOpen = !pistonAutoOpen;
                    updateScreen = true;
                } else if (JSDown()) {
                    pistonAutoOpen = !pistonAutoOpen;
                    updateScreen = true;
                } else if (JSLeft()) {
                    EEPROM.update(5, pistonAutoOpen); 
                    yPos = 0;
                    updateScreen = true;
                }
            } else if (xPos == 1) {
                if (JSUp()) {
                    pistonAutoClose = !pistonAutoClose;
                    updateScreen = true;
                } else if (JSDown()) {
                    pistonAutoClose = !pistonAutoClose;
                    updateScreen = true;
                } else if (JSLeft()) {
                    EEPROM.update(6, pistonAutoClose); 
                    yPos = 0;
                    updateScreen = true;
                }
            } else if (xPos == 2) {
                if (JSUp()) {
                    advanceTimer(&pistonOpenTimer, TIMER_STEP);
                    updateScreen = true;
                } else if (JSDown()) {
                    advanceTimer(&pistonOpenTimer, -TIMER_STEP);
                    updateScreen = true;
                } else if (JSLeft()) {
                    EEPROM.update(7, pistonOpenTimer / 60);
                    EEPROM.update(8, pistonOpenTimer % 60);
                    yPos = 0;
                    updateScreen = true;
                }
            } else if (xPos == 3) {
                if (JSUp()) {
                    advanceTimer(&pistonClosedTimer, TIMER_STEP);
                    updateScreen = true;
                } else if (JSDown()) {
                    advanceTimer(&pistonClosedTimer, -TIMER_STEP);
                    updateScreen = true;
                } else if (JSLeft()) {
                    EEPROM.update(9, pistonClosedTimer / 60);
                    EEPROM.update(10, pistonClosedTimer % 60);
                    yPos = 0;
                    updateScreen = true;
                }
            }
        }
    }
}

void setLightTimer()
{
    int yPos = 0;
    xPos = 0;
    bool updateScreen = true;
    
    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        if (updateScreen) {
            lcd.clear();
            
            // Print menu arrows
            lcd.setCursor(0, 0);
            lcd.print(xPos != 0 ? (char)0 : ' ');
            lcd.setCursor(0, SCREEN_ROWS - 1);
            lcd.print(xPos != lightMenuSize - 1 ? (char)1 : ' ');
            
            for (int x = 0; x < SCREEN_ROWS; x++) {
                if(xPos + x < lightMenuSize) {
                    lcd.setCursor(1, x);
                    lcd.print(lightMenu[x + xPos]);
                }
            }
            
            // Print current configuration
            if (0 <= xPos && xPos < 1) {
                lcd.setCursor(6, xPos);
                lcd.print(lightStates[lightState]);
            }
            if (0 <= xPos && xPos < 2) {
                lcd.setCursor(10, 1 - xPos);
                printTimeLCD(lightOnTimer);
            }
            if (1 <= xPos && xPos < 3) {
                lcd.setCursor(10, 2 - xPos);
                printTimeLCD(lightOffTimer);
            }

            // Print blinking cursor when editing 
            if(yPos == 1) {
                lcd.setCursor(strlen(lightMenu[xPos]) + 1, 0);
                lcd.blink();
            }  else {
                lcd.noBlink();
            }
            
            delay(150);
            updateScreen = false;
        }

        delay(150);
        if (yPos == 0) {
             if (xPos == 3 && (JSLeft() || JSRight())) {
                xPos = 0;
                break;
            }
            if (JSDown() && xPos < lightMenuSize - 1) {
                xPos++;
                updateScreen = true;
            } else if (JSUp() && xPos > 0) {
                xPos--;
                updateScreen = true;
            } else if (JSRight()) {
                yPos = 1;
                updateScreen = true;
            }
        } else if (yPos == 1) {
            if (xPos == 0) {
                if (JSUp()) {
                    lightState = (lightState + 1) % lightStatesSize;
                    updateScreen = true;
                } else if (JSDown()) {
                    lightState = (lightState + (lightStatesSize - 1)) % lightStatesSize;
                    updateScreen = true;
                } else if (JSLeft()) {
                    EEPROM.update(0, lightState); 
                    yPos = 0;
                    updateScreen = true;
                }
            } else if (xPos == 1) {
                if (JSUp()) {
                    advanceTimer(&lightOnTimer, TIMER_STEP);
                    updateScreen = true;
                } else if (JSDown()) {
                    advanceTimer(&lightOnTimer, -TIMER_STEP);
                    updateScreen = true;
                } else if (JSLeft()) {
                    EEPROM.update(1, lightOnTimer / 60);
                    EEPROM.update(2, lightOnTimer % 60);
                    yPos = 0;
                    updateScreen = true;
                }
            } else if (xPos == 2) {
                if (JSUp()) {
                    advanceTimer(&lightOffTimer, TIMER_STEP);
                    updateScreen = true;
                } else if (JSDown()) {
                    advanceTimer(&lightOffTimer, -TIMER_STEP);
                    updateScreen = true;
                } else if (JSLeft()) {
                    EEPROM.update(3, lightOffTimer / 60);
                    EEPROM.update(4, lightOffTimer % 60);
                    yPos = 0;
                    updateScreen = true;
                }
            }
        }
    }
}

void LightHeatTest() {
    int heatState = analogRead(tempSensorPin);
    double temp = Thermistor(heatState);      
    
    Serial.print(temp); 
    Serial.println(" degrees C");
    
    int lightState = analogRead(lightSensorPin);
    Serial.print("Light: ");
    Serial.println(lightState);
    
}

double Thermistor(int RawADC) {
    double Temp;
    Temp = log(10000.0*((1024.0/RawADC-1))); 
    Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp )) * Temp );
    Temp = Temp - 273.15; // Convert Kelvin to Celcius
    return Temp;
  }

void advanceTimer(int* num, int add)
{
    *num += add;
    if(*num >= 1440) {
        *num = 0;
    } else if(*num < 0) {
        *num = 1440 - TIMER_STEP;
    }
}

void printTimeLCD(unsigned int time)
{
    int hour = time / 60;
    int minutes = time % 60;

    if (hour < 10)
        lcd.print('0');
    lcd.print(hour);
    lcd.print(':');
    if (minutes < 10)
        lcd.print('0');
    lcd.print(minutes);
}

void movePiston(int state, bool setOpen)
{
    setTimeElapsed();
    int volt = map(abs(state), 0, 512, 40, 255);
    analogWrite(pwmPin, volt);

    digitalWrite(openPin, setOpen ? HIGH : LOW);
    digitalWrite(closePin, setOpen ? LOW : HIGH);
    if (timeHeld > TRIGGER_TIME
) {
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
