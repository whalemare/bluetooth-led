#include <SoftwareSerial.h>
#include <Time.h>
const int gRxPin = 8;
const int gTxPin = 9;

const int PIN_RED = 10;
const int PIN_GREEN = 11;
const int PIN_BLUE = 12;

const String PARAM_MODE = "mode: ";
const String PARAM_R = "r: ";
const String PARAM_G = "g: ";
const String PARAM_B = "b: ";


const int MODE_DEFAULT = 0; // свечение дефолтного цвета
const int MODE_RAINBOW = 1; // радуга
const int MODE_RGB = 2; // cвечение по переданному цвету
short mode = MODE_DEFAULT;

// Colors for rainbow
int red[3]    = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]   = { 0, 0, 100 };
int yellow[3] = { 40, 95, 0 };

SoftwareSerial BTSerial(gRxPin, gTxPin);

const String message[] = {
    " ___       __   ___  ___     ",
     "|\\  \\     |\\  \\|\\  \\|\\  \\    ",
      "\\ \\  \\    \\ \\  \\ \\  \\\\\\  \\   ",
      " \\ \\  \\  __\\ \\  \\ \\   __  \\  ",
      "  \\ \\  \\|\\__\\_\\  \\ \\  \\ \\  \\ ",
      "   \\ \\____________\\ \\__\\ \\__\\",
      "    \\|____________|\\|__|\\|__|",
      "Setup v1.0: Arduino LED-Bluetooth Bundle"
      };
       
void setup() {
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    BTSerial.begin(9600);
    Serial.begin(9600);

    delay(500);

    int i=0;
    for (i = 0; i < 8; i++) {
        Serial.println(message[i]);
        BTSerial.println(message[i]);
    }
}

void loop() {
    String input = parseBluetoothLine();
    if (input.length() > 0) {
        mode = parseMode(input);
    }

    switch (mode) {
    case MODE_DEFAULT:
        setColor(0, 255, 255);
        break;
    case MODE_RAINBOW:
        crossFade(red);

        input = parseBluetoothLine();
        if (input.length() > 0) {
            mode = parseMode(input);
        }
        if (mode != MODE_RAINBOW) {
            break;
        } else {
            crossFade(green);
        }

        input = parseBluetoothLine();
        if (input.length() > 0) {
            mode = parseMode(input);
        }
        if (mode != MODE_RAINBOW) {
            break;
        } else {
            crossFade(blue);
        }

        input = parseBluetoothLine();
        if (input.length() > 0) {
            mode = parseMode(input);
        }
        if (mode != MODE_RAINBOW) {
            break;
        } else {
            crossFade(yellow);
        }

        break;
    case MODE_RGB:
        if (input.length() > 0) {
            int r = parseColor(input, PARAM_R);
            int g = parseColor(input, PARAM_G);
            int b = parseColor(input, PARAM_B);
            input = "";
            setColor(r, g, b);
        }
        break;
    }
}

int parseColor(String input, String param) {
    int positionStart = input.indexOf(param);
    if (positionStart >= 0) {
        int positionParam = positionStart + param.length();
        String substring = input.substring(positionParam, positionParam + 3); // 3 its color length like 255, 000 and etc.
        int value = substring.toInt();

        BTSerial.print("parsed color by param: ");
        BTSerial.print(param);
        BTSerial.print(", value: ");
        BTSerial.println(value);
        return value;
    } else {
        return 0;
    }
}

String parseBluetoothLine() {
    String string = "";
    if (BTSerial.available()) {
        string = BTSerial.readString();
        if (string.length() > 0) {
            BTSerial.println(string);
            print("bluetooth >> ", string);
        } else {
            print("Input length = ", string.length());
        }
    }
    return string;
}


int parseMode(String input) {
    if (input.startsWith(PARAM_MODE)) {
        String substring = input.substring(PARAM_MODE.length());
        const int modeParsed = substring.toInt();
        print("parse mode int: ", modeParsed);
        BTSerial.println(modeParsed);
        return modeParsed;
    } else {
        print("Error parse input >> ", input);
        print("return current mode: ", mode);
        return mode;
    }
}

void println(String text) {
    Serial.println(text);
}

void print(String text, int value) {
    Serial.print(text);
    Serial.println(value);
}

void print(String text, String value) {
    Serial.print(text);
    Serial.println(value);
}

void setColor(int red, int green, int blue) {
#ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
#endif
    analogWrite(PIN_RED, red);
    analogWrite(PIN_GREEN, green);
    analogWrite(PIN_BLUE, blue);
}

/* BELOW THIS LINE IS THE MATH FOR RAINBOW -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
*
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10,
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
*
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
*
* The red rises from 0 to 10 in ten steps, the green from
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
*
* In the real program, the color percentages are converted to
* 0-255 values, and there are 1020 steps (255*4).
*
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(),
* which calculates the absolute gap between the start and end values,
* and then divides that gap by 1020 to determine the size of the step
* between adjustments in the value.
*/
int calculateStep(int prevValue, int endValue) {
    int step = endValue - prevValue; // What's the overall gap?
    if (step) {                      // If its non-zero,
        step = 1020 / step;            //   divide by 1020
    }
    return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1.
*  (R, G, and B are each calculated separately.)
*/
int calculateVal(int step, int val, int i) {
    if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
        if (step > 0) {              //   increment the value if step is positive...
            val += 1;
        } else if (step < 0) {       //   ...or decrement it if step is negative
            val -= 1;
        }
    }
    // Defensive driving: make sure val stays in the range 0-255
    if (val > 255) {
        val = 255;
    } else if (val < 0) {
        val = 0;
    }
    return val;
}

// Set initial color
int redVal = 0;
int grnVal = 0;
int bluVal = 0;

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

int wait = 10;      // 10ms internal crossFade delay; increase for slower fades
int hold = 0;       // Optional hold when a color is complete, before the next crossFade

/* crossFade() converts the percentage colors to a
*  0-255 range, then loops 1020 times, checking to see if
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/
void crossFade(int color[3]) {
    int DEBUG = 1;      // DEBUG counter; if set to 1, will write values back via serial

    // Convert to 0-255
    int R = (color[0] * 255) / 100;
    int G = (color[1] * 255) / 100;
    int B = (color[2] * 255) / 100;

    int stepR = calculateStep(prevR, R);
    int stepG = calculateStep(prevG, G);
    int stepB = calculateStep(prevB, B);

    for (int i = 0; i <= 1020; i++) {
        redVal = calculateVal(stepR, redVal, i);
        grnVal = calculateVal(stepG, grnVal, i);
        bluVal = calculateVal(stepB, bluVal, i);

        setColor(redVal, grnVal, bluVal);

        delay(wait); // Pause for 'wait' milliseconds before resuming the loop

        if (DEBUG) { // If we want serial output, print it at the
            Serial.print("Loop/RGB: #");
            Serial.print(i);
            Serial.print(" | ");
            Serial.print(redVal);
            Serial.print(" / ");
            Serial.print(grnVal);
            Serial.print(" / ");
            Serial.println(bluVal);
        }
    }
    // Update current values for next loop
    prevR = redVal;
    prevG = grnVal;
    prevB = bluVal;
    delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}