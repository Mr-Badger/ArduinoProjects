#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>

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

// Timers in minutes

short lightState = EEPROM[0];
unsigned short lightOnTimer = EEPROM[1] * 60 + EEPROM[2];
unsigned short lightOffTimer = EEPROM[3] * 60 + EEPROM[4];

bool pistonAutoOpen = false;
bool pistonAutoClose = false;
unsigned short pistonOpenTimer = 480;
unsigned short pistonClosedTimer = 1260;

uint8_t upArrow[8] = { 0x0, 0x4, 0xE, 0x1B, 0x11, 0x0, 0x0 };
uint8_t downArrow[8] = { 0x0, 0x0, 0x11, 0x1B, 0xE, 0x4, 0x0 };

const char menuItems[4][16] {
    "Piston manual",
    "Piston timer",
    "Light timer",
    "---------------"
};

const short menuSize = sizeof(menuItems) / 16 - 1;

const char lightMenu[5][16] {
    "Mode",
    "On time ",
    "Off time",
    "Back",
    "---------------"
};

const short lightMenuSize = sizeof(lightMenu) / 16 - 1;

const char lightStates[3][5] {
    "Auto",
    "On",
    "Off"
};

const int lightStatesSize = sizeof(lightStates) / 5; 


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

        short newPos = xPos;
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

            lcd.setCursor(0, 0);
            lcd.print(newPos != 0 ? (char)0 : ' ');
            lcd.setCursor(0, SCREEN_ROWS - 1);
            lcd.print(newPos != menuSize - 1 ? (char)1 : ' ');
            
            if (newPos == -1)
                newPos = 0;
            for (int y = 0; y < SCREEN_ROWS; y++) {
                lcd.setCursor(1, y);
                lcd.print(menuItems[y + newPos]);
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
            if ((time > holdTimeStart + holdTime) || (!isOpening && JSDown()) || (isOpening && JSUp())) {
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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Open  time");
    lcd.setCursor(0, 1);
    lcd.print("Close time");
    while (true) {
        time = millis();
        jsXstate = analogRead(xPin) - 512;
        jsYstate = analogRead(yPin) - 512;

        lcd.setCursor(11, 0);
        printTimeLCD(pistonOpenTimer);
        lcd.setCursor(11, 1);
        printTimeLCD(pistonClosedTimer);

        if (JSLeft()) {
            break;
        }
        delay(100);
    }
}

void setLightTimer()
{
    short yPos = 0;
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
                lcd.setCursor(1, x);
                lcd.print(lightMenu[x + xPos]);
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

void advanceTimer(int* num, int add)
{
    *num += add;
    if(*num >= 1440) {
        *num = 0;
    } else if(*num < 0) {
        *num = 1440 - TIMER_STEP;
    }
}

void printTimeLCD(unsigned short time)
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