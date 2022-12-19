#include <LiquidCrystal.h>

// Define LCD pins.
const int lcdRS = 13;
const int lcdEnable = 12;
const int lcdD4 = 11;
const int lcdD5 = 10;
const int lcdD6 = 9;
const int lcdD7 = 8;

// Define button pins.
const int buttonSelect = 7;
const int buttonRight = 6;

// Define ultrasonic pins.
const int usTrigger = 5;
const int usEcho = 4;

// Store the ultrasonic sensor data.
typedef struct usInfo {
    long duration;
    float distance;
    int min = 5;
    int max = 35;
} usInfo;
usInfo sensor;

// Button states
bool wasSelectPressed = false;
bool wasRightPressed = false;

// Program state
enum displayState {
    OPTION_SELECT,
    SETTING_SELECT,
    WAVE_SELECT,
    LIMIT_SELECT,
    OUTPUT_SELECT,
    MIN_SELECT,
    MAX_SELECT
};
displayState dState = OPTION_SELECT;

enum optionState {
    AMPLITUDE = 1,
    FREQUENCY = 2
};
optionState oState = AMPLITUDE;

enum waveState {
    SINE = 0,
    SQUARE = 1,
    TRIANGLE = 2,
    SAW = 3
};
waveState wState = SINE;

enum outputState {
    MIDI = 0,
    OSC = 1
};
outputState outState = MIDI;

int midiChannel = 0;

// Option names
const int optionCount = 4;
char *optionNames[] = {"Wave Type", "Amplitude", "Frequency", "Settings"};
const int settingCount = 3;
char *settingNames[] = {"Limits", "Output Format", "Back"};
const int waveCount = 5;
char *waveNames[] = {"Sine", "Square", "Triangle", "Saw", "Back"};
const int limitCount = 3;
char *limitNames[] = {"Set Min", "Set Max", "Back"};
const int outputCount = 3;
char *outputNames[] = {"MIDI", "OSC", "Back"};
const int minMaxCount = 3;
char *minMaxNames[] = {"Increment", "Decrement", "Back"};

// Option positions
int arrayPos = 0;

// Define the LCD.
LiquidCrystal lcd(lcdRS, lcdEnable, lcdD4, lcdD5, lcdD6, lcdD7);

// functions
void changeDisplayState(displayState newState)
{
    arrayPos = 0;
    dState = newState;
}

void changeSensorMin(int desiredChange)
{
    int newVal = sensor.min + desiredChange;

    if (newVal < 1 || newVal >= sensor.max)
    { // Check bounds of [1, sensor.max)
        return;
    }

    sensor.min = newVal;
}

void changeSensorMax(int desiredChange)
{
    int newVal = sensor.max + desiredChange;

    if (newVal <= sensor.min || newVal > 45)
    { // Check bounds of (sensor.min, 45]
        return;
    }

    sensor.max = newVal;
}

int scaleValue(float value, int oldMin, int oldMax, int newMin, int newMax)
{
    int scaledValue = ((newMax - newMin) * (value - oldMin) / (oldMax - oldMin)) + newMin;
    return scaledValue;
}

void sendMidiMessage(byte cmd, byte data1, byte data2)
{
    Serial.write(cmd);
    Serial.write(data1);
    Serial.write(data2);
}

void sendOscMessage()
{

}

void whenRightPressed()
{
    arrayPos++;
    switch (dState)
    {
        case OPTION_SELECT:
            if (arrayPos >= optionCount)
                arrayPos = 0;
            break;
        case SETTING_SELECT:
            if (arrayPos >= settingCount)
                arrayPos = 0;
            break;
        case WAVE_SELECT:
            if (arrayPos >= waveCount)
                arrayPos = 0;
            break;
        case LIMIT_SELECT:
            if (arrayPos >= limitCount)
                arrayPos = 0;
            break;
        case OUTPUT_SELECT:
            if (arrayPos >= outputCount)
                arrayPos = 0;
            break;
        case MIN_SELECT:
        case MAX_SELECT:
            if (arrayPos >= minMaxCount)
                arrayPos = 0;
            break;
    }
}

void whenSelectPressed()
{
    switch (dState)
    {
        case OPTION_SELECT:
            if (arrayPos == 0) // Wave type
            {
                // Go to a menu subtree
                changeDisplayState(WAVE_SELECT);
            }
            else if (arrayPos == 1) // Amplitude
            {
                oState = AMPLITUDE;
            }
            else if (arrayPos == 2) // Frequency
            {
                oState = FREQUENCY;
            }
            else if (arrayPos == 3) // Settings
            {
                // Go to a menu subtree
                changeDisplayState(SETTING_SELECT);
            }
            break;
        case SETTING_SELECT:
            if (arrayPos == 0) // Limits
            {
                // Go to a menu subtree
                changeDisplayState(LIMIT_SELECT);
            }
            else if (arrayPos == 1) // Output Fmt
            {
                changeDisplayState(OUTPUT_SELECT);
            }
            else if (arrayPos == 2) // Back
            {
                // Go up a level in the menu tree
                changeDisplayState(OPTION_SELECT);
            }
            break;
        case WAVE_SELECT:
            if (arrayPos == 0) // Sine
            {
                wState = SINE;
            }
            else if (arrayPos == 1) // Square
            {
                wState = SQUARE;
            }
            else if (arrayPos == 2) // Triangle
            {
                wState = TRIANGLE;
            }
            else if (arrayPos == 3) // Saw
            {
                wState = SAW;
            }
            else if (arrayPos == 4) // Back
            {
                // Go up a level in the menu tree
                changeDisplayState(OPTION_SELECT);
            }
            break;
        case LIMIT_SELECT:
            if (arrayPos == 0) // Set min
            {
                changeDisplayState(MIN_SELECT);
            }
            else if (arrayPos == 1) // Set max
            {
                changeDisplayState(MAX_SELECT);
            }
            else if (arrayPos == 2) // Back
            {
                // Go up a level in the menu tree
                changeDisplayState(SETTING_SELECT);
            }
            break;
        case OUTPUT_SELECT:
            if (arrayPos == 0) // MIDI
            {
                outState = MIDI;
            }
            else if (arrayPos == 1) // OSC
            {
                outState = OSC;
            }
            else if (arrayPos == 2) // Back
            {
                changeDisplayState(SETTING_SELECT);
            }
            break;
        case MIN_SELECT:
            if (arrayPos == 0) // Add
            {
                changeSensorMin(1);
            }
            else if (arrayPos == 1) // Subtract
            {
                changeSensorMin(-1);
            }
            else if (arrayPos == 2) // Back
            {
                changeDisplayState(LIMIT_SELECT);
            }
            break;
        case MAX_SELECT:
            if (arrayPos == 0) // Add
            {
                changeSensorMax(1);
            }
            else if (arrayPos == 1) // Subtract
            {
                changeSensorMax(-1);
            }
            else if (arrayPos == 2) // Back
            {
                changeDisplayState(LIMIT_SELECT);
            }
            break;
    }
}

void setup()
{
    // Setup LCD.
    lcd.begin(16, 2);

    // Set ultrasonic pin modes.
    pinMode(usEcho, INPUT);
    pinMode(usTrigger, OUTPUT);

    // Set up push buttons.
    pinMode(buttonSelect, INPUT);
    pinMode(buttonRight, INPUT);

    // Begin serial communication.
    Serial.begin(9600);
}

void loop()
{
    // Read inputs and calculate values.
    // Clear the trigger pin
    digitalWrite(usTrigger, LOW);
    delayMicroseconds(2);

    // Start a reading by triggering the sensor for 10 µs
    digitalWrite(usTrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(usTrigger, LOW);

    // Read the echo pin, getting the sound wave travel time in µs
    sensor.duration = pulseIn(usEcho, HIGH);
    // Calculate the distance (in cm)
    sensor.distance = sensor.duration * 0.034 / 2.;
    // Cap the sensor distance.
    sensor.distance = constrain(sensor.distance, sensor.min, sensor.max);

    // Read the buttons.
    bool selectPressed = digitalRead(buttonSelect);
    bool rightPressed = digitalRead(buttonRight) && !selectPressed;

    // ==============

    // Check button presses and change program state.
    if (rightPressed && !wasRightPressed)
    {
        wasRightPressed = true;
        whenRightPressed();
    }
    else if (!rightPressed && wasRightPressed)
    {
        wasRightPressed = false;
    }

    if (selectPressed && !wasSelectPressed)
    {
        wasSelectPressed = true;
        whenSelectPressed();

        // Register change messages (wave change, option change, output change)
        /* 
        Note: This currently "over-sends" a change message.
        When a menu option is selected that leads to another case that should send a message
        -- say, Pitch Bend or Back --
        it'll send the current setting for the menu just entered.
        This is unnecessary, but should be harmless.
        */
        switch (dState)
        {
            case WAVE_SELECT:
                // Send a sysex message. Vendor 4b, 1 byte of data detailing the wave type.
                sendMidiMessage(0xf0, 0x4b, wState);
                Serial.println();
                break;
        }
    }
    else if (!selectPressed && wasSelectPressed)
    {
        wasSelectPressed = false;
    }

    // Send actual messages.
    if (dState == OPTION_SELECT && outState == MIDI)
    {
        if (oState == AMPLITUDE)
        {
            // This is not an actual MIDI message. Send using SysEx.
            // 3 bytes of data: first byte notes message type.
            sendMidiMessage(0xf0, 0x4b, 0x10);

            int amplitude = scaleValue(sensor.distance, sensor.min, sensor.max, 0, 255);

            Serial.write(amplitude);
            Serial.println();
        }
        else if (oState == FREQUENCY)
        {
            // First byte notes message type
            sendMidiMessage(0xf0, 0x4b, 0x11);

            int frequency = scaleValue(sensor.distance, sensor.min, sensor.max, 0, 255);

            Serial.write(frequency);
            Serial.println();
        }
    }
    else if (dState == OPTION_SELECT && outState == OSC)
    {
        // TODO: Implement OSC!
    }

    // Write info to screen.
    // Declare variables.
    char line1[17];
    char line2[17];
    char *menuText;
    
    // Get current name.
    switch (dState)
    {
        case OPTION_SELECT:
            snprintf(line1, 17, "Mode: %-10s", optionNames[oState]);
            menuText = optionNames[arrayPos];
            break;
        case SETTING_SELECT:
            snprintf(line1, 17, "Cur Dist: %2d cm  ", (int)sensor.distance);
            menuText = settingNames[arrayPos];
            break;
        case WAVE_SELECT:
            snprintf(line1, 17, "Wave: %-10s", waveNames[wState]);
            menuText = waveNames[arrayPos];
            break;
        case LIMIT_SELECT:
            snprintf(line1, 17, "Min: %2d  Max: %2d", sensor.min, sensor.max);
            menuText = limitNames[arrayPos];
            break;
        case OUTPUT_SELECT:
            snprintf(line1, 17, "Format: %-8s", outputNames[outState]);
            menuText = outputNames[arrayPos];
            break;
        case MIN_SELECT:
            snprintf(line1, 17, "Min: %2d  Max: %2d", sensor.min, sensor.max);
            menuText = minMaxNames[arrayPos];
            break;
        case MAX_SELECT:
            snprintf(line1, 17, "Min: %2d  Max: %2d", sensor.min, sensor.max);
            menuText = minMaxNames[arrayPos];
            break;
    }

    snprintf(line2, 17, "%-16s", menuText);

    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);

    // ==========

    // Delay recommended amount for ultrasonic sensor.
    delay(10);
}
