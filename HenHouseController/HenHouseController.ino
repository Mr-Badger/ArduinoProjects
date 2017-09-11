#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

typedef void (*vvFunction)();

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

uint8_t upArrow[8] = { 0x0, 0x4, 0xE, 0x1B, 0x11, 0x0, 0x0 };
uint8_t downArrow[8] = { 0x0, 0x0, 0x11, 0x1B, 0xE, 0x4, 0x0 };

/*
const int TRIGGER_TIME = 1000;
const int HOLD_TIME = 25000;

unsigned long time = 0;
unsigned long timeStarted = 0;
unsigned long timeHeld = 0;
unsigned long holdTimeStart = 0;

bool isHolding = false;
bool isOpening = true;

const char lightStates[][5]{
    "Auto",
    "On",
    "Off"
};
const int lightStatesSize = sizeof(lightStates) / sizeof(lightStates[0]);

const char onOffStates[][4]{
    "On",
    "Off"
};
*/
class Selectable 
{
    String type;
public:
    Selectable() {
        type = "select";
    }


    String Type() 
    {
        return type;
    }
};

class Action
{ 
protected:
    String type;

public:
    Action() 
    {
    }
    
    String Type() 
    {
        return type;
    }
    
    virtual int Get()
    {
        return 0;
    }

    virtual void Update()
    {
    }

    virtual void Increase()
    {
    }

    virtual void Decrease()
    {
    }

    virtual char* GetFormat()
    {
        return "";
    }
};

class StoredTime : Action
{
    static const byte STEP = 10;
    
    int time;
    byte address;

public:
    StoredTime(byte address) : address(address)
    {
        time = EEPROM[address] * 60 + EEPROM[address + 1];
        type = "time";
    }

    virtual int Get()
    {
        return time;
    }

    virtual void Update()
    {
        EEPROM[address] = time / 60;
        EEPROM[address + 1] = time % 60;   
    }

    virtual void Increase()
    {
        time += STEP;
        if (time >= 1440) {
            time = time % 1440;
        }
    }

    virtual void Decrease()
    {
        time -= STEP;
        if (time < 0) {
            time = time + 1440;
        }
    }

    virtual char * GetFormat()
    {   
        char format[6];
        int hour = time / 60;
        int minutes = time % 60;

        format[0] = (hour / 10) + '0';
        format[1] = (hour % 10) + '0';
        format[2] = ':';
        format[3] = (minutes / 10) + '0';
        format[4] = (minutes % 10) + '0';
        return format;
    }
};

class StoredState : Action
{
    byte state;
    byte address;
    byte stateCount;

    const char values[3][5] {
        "On",
        "Off",
        "Auto"
    };

public:
    StoredState(byte address, byte count) 
        : address(address)
        , stateCount(count)
    {
        state = EEPROM[address];
        type = "state";
    }

    virtual int Get()
    {
        return state;
    }

    virtual void Update()
    {
        EEPROM[address] = state;
    }

    virtual void Increase()
    {
        state++;
        if (state >= stateCount)
            state = state % stateCount;
    }

    virtual void Decrease()
    {
        state--;
        if (state < 0)
            state = state + stateCount;  
    }

    virtual char* GetFormat()
    {
        return values[state];
    }
};

class MenuItem {
    const String title;
    
    Selectable action;
    
public:
    MenuItem(String title, Selectable action)
    : title(title)
    {
        this->action = action;
    }
    
    String GetTitle()
    {
        return title;
    }

    Selectable* GetAction()
    {
        return &action;
    }
};

class Menu {
    const Menu* parent;
    
    const MenuItem* menuItems;

    const byte size;
    
public:
    Menu(Menu* parent, MenuItem menuItems[], byte size)
        : parent(parent)
        , size(size)
    {
        this->menuItems = menuItems;
        //type = "menu";
    }
    
    Selectable* GetAction(int pos) {
        return menuItems[pos].GetAction();
    }
    
    int GetSize()
    {
        return size;
    }
    
    MenuItem* GetMenuItems()
    {
        return menuItems;
    }
    
    Menu* GetParent()
    {
        return parent;
    }
};

class MenuController {
    
    const Menu currentMenu;
    LiquidCrystal_I2C lcd;
    const byte columns;
    const byte rows;
    
    //int clickTime = 0;
     
    byte posX = 0;
    byte posY = 0;
    
    int stickX;
    int stickY;
    
    enum StickStates {
        IDLE,
        MOVEUP,
        MOVEDOWN,
        MOVELEFT,
        MOVERIGHT
    } stickState;
    
    int time;
    void proccessInputs() 
    {
        stickX = analogRead(xPin) - 512;
        stickY = analogRead(yPin) - 512;
        
        if(stickY + Y_DEADZONE < 0) {
            stickState = MOVERIGHT;
        }
        else if(stickY - Y_DEADZONE > 0) {
            stickState = MOVELEFT;
        }
        else if(stickX + X_DEADZONE < 0) {
            stickState = MOVEUP;
        }
        else if(stickX - X_DEADZONE > 0) {
            stickState = MOVEDOWN;
        }
        else {
            stickState = IDLE;
        }
        if (posY == 0) {
            if (stickState == MOVEDOWN && posX < currentMenu.GetSize() - 1) {
                posX++;
            } else if (stickState == MOVEUP && posX > 0) {
                posX--;
            } else if(stickState == MOVERIGHT) {
                Selectable* func = currentMenu.GetAction(posX);
                if(func != NULL) {
                    Serial.println(func->Type());
                    posY = 1;
                }
            }
        }
        if (posY == 1) {
            if(stickState == MOVELEFT) {
                posY = 0;
            } 
        }
    }

public:
    MenuController(Menu startMenu, LiquidCrystal_I2C lcd, int columns, int rows)
    : currentMenu(startMenu)
    , lcd(lcd)
    , columns(columns)
    , rows(rows)
    {
    }
    
    void Setup()
    {
        lcd.begin(columns, rows);
        lcd.createChar(0, upArrow);
        lcd.createChar(1, downArrow);
    }
    
    void Loop()
    {
        time = millis();
        proccessInputs();
        if(stickState != IDLE) {
            Display();
        }
    }
    
    void Display()
    {
        int size = currentMenu.GetSize();
        MenuItem* items = currentMenu.GetMenuItems();
        lcd.clear();
        
        // Prints menu arrows
        lcd.setCursor(0, 0);
        lcd.print(posX != 0 ? (char)0 : ' ');
        lcd.setCursor(0, rows - 1);
        lcd.print(posX != size - 1 ? (char)1 : ' ');
        
        for (int i = 0; i < rows; i++) {
            if (posX + i < size) {
                lcd.setCursor(1, i);
                lcd.print(items[i + posX].GetTitle());
            }
        }
        
        // Print blinking cursor when editing
        if (posY == 1) {
            lcd.setCursor(items[posX].GetTitle().length() + 1, 0);
            lcd.blink();
        } else {
            lcd.noBlink();
        }
    }
};


StoredTime lightsOnTime(0);
StoredTime lightsOffTime(2);
StoredState lightState(3, 3);

StoredState pistonAutoOpen(4, 2);
StoredState pistonAutoClose(5, 2);
StoredTime pistonOpenTime(6);
StoredTime pistonCloseTime(8);


MenuItem pistonList[] {
    MenuItem("Auto open ", Selectable()),
    MenuItem("Auto close", Selectable()),
    MenuItem("Open ", Selectable()),
    MenuItem("Close", Selectable()),
    MenuItem("Back", Selectable())
};

MenuItem lightList[] {
    MenuItem("Mode", Selectable()),
    MenuItem("On time ", Selectable()),
    MenuItem("Off time", Selectable()),
    MenuItem("Back", Selectable())
};

MenuItem mainList[] {
    MenuItem("Piston manual", Selectable()),
    MenuItem("Piston timer", Selectable()),
    MenuItem("Light timer", Selectable())
};

Menu mainMenu(NULL, mainList, 3);
Menu lightMenu(NULL, lightList, 4);
Menu pistonMenu(NULL, pistonList, 5);

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

MenuController ctrl(mainMenu, lcd, 16, 2);

void setup()
{
    Serial.begin(115200);
    pinMode(pwmPin, OUTPUT);
    pinMode(ledPin, OUTPUT);

    pinMode(openPin, INPUT);
    pinMode(closePin, INPUT);
    digitalWrite(openPin, HIGH);
    digitalWrite(closePin, LOW);
    ctrl.Setup();
    ctrl.Display();
}

void loop()
{
    ctrl.Loop();
    delay(200);
}
/*
void mainMenu2()
{
    if (JSDown() && xPos < menuSize - 1) {
        xPos++;
        updateS = true;
    } else if (JSUp() && xPos > 0) {
        xPos--;
        updateS = true;
    } else if (JSRight()) {
        menuTable[xPos]();
        updateS = true;
    }

    //Update screen
    if (updateS) {
        lcd.clear();

        // Prints menu arrows
        lcd.setCursor(0, 0);
        lcd.print(xPos != 0 ? (char)0 : ' ');
        lcd.setCursor(0, SCREEN_ROWS - 1);
        lcd.print(xPos != menuSize - 1 ? (char)1 : ' ');

        if (xPos == -1)
            xPos = 0;
        for (int i = 0; i < SCREEN_ROWS; i++) {
            if (xPos + i < menuSize) {
                lcd.setCursor(1, i);
                lcd.print(menuItems[i + xPos]);
            }
        }
        updateS = false;
    }
    delay(100);
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
                if (xPos + x < pistonMenuSize) {
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
            if (yPos == 1) {
                lcd.setCursor(strlen(pistonMenu[xPos]) + 1, 0);
                lcd.blink();
            } else {
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
                if (xPos + x < lightMenuSize) {
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
            if (yPos == 1) {
                lcd.setCursor(strlen(lightMenu[xPos]) + 1, 0);
                lcd.blink();
            } else {
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

void LightHeatTest()
{
    int heatState = analogRead(tempSensorPin);
    double temp = Thermistor(heatState);

    Serial.print(temp);
    Serial.println(" degrees C");

    int lightState = analogRead(lightSensorPin);
    Serial.print("Light: ");
    Serial.println(lightState);
}

double Thermistor(int RawADC)
{
    double Temp;
    Temp = log(10000.0 * ((1024.0 / RawADC - 1)));
    Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
    Temp = Temp - 273.15; // Convert Kelvin to Celcius
    return Temp;
}

void movePiston(int state, bool setOpen)
{
    setTimeElapsed();
    int volt = map(abs(state), 0, 512, 40, 255);
    analogWrite(pwmPin, volt);

    digitalWrite(openPin, setOpen ? HIGH : LOW);
    digitalWrite(closePin, setOpen ? LOW : HIGH);
    if (timeHeld > TRIGGER_TIME) {
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
*/