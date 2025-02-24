#include "mbed.h"
#include "arm_book_lib.h"

#define RAPID_BLINK_RATE 100
#define SLOW_BLINK_RATE 1000
#define LOCKOUT_DURATION 60

DigitalIn enterButton(BUTTON1);
DigitalIn gasDetector(D2);
DigitalIn overTempDetector(D3);
DigitalIn aButton(D4);
DigitalIn bButton(D5);
DigitalIn cButton(D6);
DigitalIn dButton(D7);

DigitalOut alarmLed(LED1);
DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

bool alarmState = false;
bool emergencyMode = false;
bool systemLocked = false;
int numberOfIncorrectCodes = 0;
int lockoutCounter = 0;
Timer lockoutTimer;

void checkCode() {
    if (systemLocked) return;  // Prevent code entry when locked

    if (numberOfIncorrectCodes < 5){
        if (enterButton) {
        if (aButton && bButton && cButton && dButton) {  // Correct code
            alarmState = false;
            emergencyMode = false;
            numberOfIncorrectCodes = 0;
            alarmLed = OFF;
            incorrectCodeLed = OFF;
            systemBlockedLed = OFF;
        } else {  // Incorrect code
            numberOfIncorrectCodes++;
        }
        }
    }else{
        systemLocked = true;
        lockoutCounter = 0;
        lockoutTimer.start();
    }
}

int main() {
    gasDetector.mode(PullDown);
    overTempDetector.mode(PullDown);
    aButton.mode(PullDown);
    bButton.mode(PullDown);
    cButton.mode(PullDown);
    dButton.mode(PullDown);

    alarmLed = OFF;
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;

    while (true) {
        bool gasDetected = gasDetector.read();
        bool overTempDetected = overTempDetector.read();

        // Alarm activation 
        if (!alarmState) {
            if (gasDetected || overTempDetected) {
                alarmState = true;
                alarmLed = ON;
            }
        }

        // Emergency mode activation
        if (gasDetected && overTempDetected) {
            emergencyMode = true;
        }

        // LED behavior before 5 incorrect codes (Flashing LEDs for emergency)
        if (alarmState && emergencyMode && !systemLocked) {
            alarmLed = !alarmLed;
            incorrectCodeLed = !incorrectCodeLed;
            systemBlockedLed = !systemBlockedLed;
            ThisThread::sleep_for(RAPID_BLINK_RATE);
        }

        // Lockout mode behavior after 5 incorrect codes
        if (systemLocked) {
            alarmLed = OFF;            // LED1 off
            incorrectCodeLed = OFF;    // LED3 off
            systemBlockedLed = !systemBlockedLed;  // Blink only LED2
            ThisThread::sleep_for(SLOW_BLINK_RATE);

            lockoutCounter++;
            if (lockoutCounter >= LOCKOUT_DURATION) {
                systemLocked = false;
                numberOfIncorrectCodes = 0;
                systemBlockedLed = OFF;
                lockoutTimer.stop();
                lockoutTimer.reset();
            }
        } else {
            checkCode();
        }
    }
}
