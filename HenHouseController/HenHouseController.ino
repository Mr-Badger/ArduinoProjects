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

class Action {
public:
    virtual int Get() { return 0; }

    virtual void Update() {}

    virtual void Increase(int amount) {}

    virtual void Decrease(int amount) {}

    virtual void Loop() {}

    virtual char* GetFormat() { return ""; }

    virtual ~Action() {}
};

class StoredTime : public Action {
    static const byte STEP = 10;

    int time;
    byte address;
    char* format = "00:00";

public:
    StoredTime(byte address)
        : address(address)
    {
        time = EEPROM[address] * 60 + EEPROM[address + 1];
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

    virtual void Increase(int amount)
    {
        time += STEP;
        if (time >= 1440) {
            time = time % 1440;
        }
    }

    virtual void Decrease(int amount)
    {
        time -= STEP;
        if (time < 0) {
            time = time + 1440;
        }
    }

    virtual char* GetFormat()
    {
        int hour = time / 60;
        int minutes = time % 60;

        format[0] = (hour / 10) + '0';
        format[1] = (hour % 10) + '0';
        format[3] = (minutes / 10) + '0';
        format[4] = (minutes % 10) + '0';
        return format;
    }

    virtual ~StoredTime() {}
};

class StoredState : public Action {
    byte address;
    byte state;
    byte count;

    const char values[3][5]{
        "On",
        "Off",
        "Auto"
    };

public:
    StoredState(byte address, byte count)
        : address(address)
        , count(count)
    {
        state = EEPROM[address];
    }

    virtual int Get()
    {
        return state;
    }

    virtual void Update()
    {
        EEPROM[address] = state;
    }

    virtual void Increase(int amount)
    {
        state++;
        if (state >= count)
            state %= count;
    }

    virtual void Decrease(int amount)
    {
        state--;
        if (state >= count)
            state %= count;
    }

    virtual char* GetFormat() { return values[state]; }

    virtual ~StoredState() {}
};

class ControlPiston : public Action {
    static const int TRIGGER_TIME = 1000;
    static const int HOLD_TIME = 25000;

    bool isHolding = false;
    bool isOpening = true;

    unsigned long time = 0;
    unsigned long timeStarted = 0;
    unsigned long timeHeld = 0;
    unsigned long holdTimeStart = 0;

    int lastState = 0;

    void movePiston(int state)
    {
        lastState = state;
        bool setOpen = state > 0;
        setTimeElapsed();
        analogWrite(pwmPin, map(abs(state), 0, 512, 40, 255));

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

public:
    virtual void Increase(int amount)
    {
        movePiston(amount);
    }

    virtual void Decrease(int amount)
    {
        movePiston(amount);
    }

    virtual char* GetFormat() { return ""; }

    virtual void Loop()
    {
        time = millis();
        if (isHolding) {
            analogWrite(pwmPin, 255);
            if ((time > holdTimeStart + HOLD_TIME) || (!isOpening && lastState > 0) || (isOpening && lastState < 0)) {
                isHolding = false;
            }
        } else {
            analogWrite(pwmPin, 0);
            resetTime();
        }
    }
};

class MenuItem {
    const String title;
    Action* action;

public:
    MenuItem(String title, Action* action)
        : title(title)
    {
        this->action = action;
    }

    String Title() { return title; }

    Action* Action() { return action; }
};

class MenuController {

    LiquidCrystal_I2C lcd;
    const MenuItem* menuItems;

    const byte columns;
    const byte rows;

    //int clickTime = 0;

    byte posX = 0;
    byte posY = 0;

    int stickX;
    int stickY;
    int size;

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
        if (stickX + X_DEADZONE < 0) {
            stickState = MOVEUP;
        } else if (stickX - X_DEADZONE > 0) {
            stickState = MOVEDOWN;
        } else if (stickY + Y_DEADZONE < 0) {
            stickState = MOVERIGHT;
        } else if (stickY - Y_DEADZONE > 0) {
            stickState = MOVELEFT;
        } else {
            stickState = IDLE;
        }
        if (posY == 0) {
            if (stickState == MOVEDOWN && posX < size - 1) {
                posX++;
            } else if (stickState == MOVEUP && posX > 0) {
                posX--;
            } else if (stickState == MOVERIGHT) {
                posY = 1;
            }
        }
        if (posY == 1) {
            Action* action = menuItems[posX].Action();
            if (stickState == MOVEDOWN) {
                action->Increase(stickX);
            } else if (stickState == MOVEUP) {
                action->Decrease(stickX);
            } else if (stickState == MOVELEFT) {
                action->Update();
                posY = 0;
            } else {
                action->Loop();
            }
        }
    }

public:
    MenuController(MenuItem menuItems[], byte size, LiquidCrystal_I2C lcd, byte columns, byte rows)
        : size(size)
        , lcd(lcd)
        , columns(columns)
        , rows(rows)
    {
        this->menuItems = menuItems;
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
        if (stickState != IDLE) {
            Display();
        }
    }

    void Display()
    {
        lcd.clear();

        // Prints menu arrows
        lcd.setCursor(0, 0);
        lcd.print(posX != 0 ? (char)0 : ' ');
        lcd.setCursor(0, rows - 1);
        lcd.print(posX != size - 1 ? (char)1 : ' ');

        for (int i = 0; i < rows; i++) {
            if (posX + i < size) {
                lcd.setCursor(1, i);
                lcd.print(menuItems[posX + i].Title() + ' ');
                lcd.print(menuItems[posX + i].Action()->GetFormat());
            }
        }

        // Print blinking cursor when editing
        if (posY == 1) {
            lcd.setCursor(menuItems[posX].Title().length() + 1, 0);
            lcd.blink();
        } else {
            lcd.noBlink();
        }
    }
};

MenuItem mainList[]{
    MenuItem("--PISTONS--", new Action()),
    MenuItem("Manual Piston", new ControlPiston()),
    MenuItem("Auto open ", new StoredState(0, 2)),
    MenuItem("Auto close", new StoredState(1, 2)),
    MenuItem("Open ", new StoredTime(2)),
    MenuItem("Close", new StoredTime(4)),
    MenuItem("--LIGHTS--", new Action()),
    MenuItem("Light mode", new StoredState(6, 3)),
    MenuItem("On time ", new StoredTime(7)),
    MenuItem("Off time", new StoredTime(9))
};

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

MenuController ctrl(mainList, 9, lcd, 16, 2);

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
*/