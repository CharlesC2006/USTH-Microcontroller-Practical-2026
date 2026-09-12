#include <Arduino.h>
#include <avr/io.h>
#define EN_PIN   PD6
#define DIR_PIN  PD5
#define PUL_PIN  PD4
#define BTN_CW   PC1
#define BTN_CCW  PC2
const unsigned long stepHalfPeriod = 1500;
unsigned long lastStepTime = 0;
bool stepPinState = false;
enum MotorDir
{
    STOP,
    FORWARD,
    BACKWARD
};
MotorDir currentDir = STOP;
void Stepper_Init()
{
    DDRD |= (1 << PUL_PIN) |
            (1 << DIR_PIN) |
            (1 << EN_PIN);
    DDRC &= ~((1 << BTN_CW) |
              (1 << BTN_CCW));
    PORTC |= (1 << BTN_CW) |
             (1 << BTN_CCW);
    PORTD &= ~(1 << PUL_PIN);
    PORTD &= ~(1 << EN_PIN);
}
void setup()
{
    Stepper_Init();
}
void loop()
{
    bool cwPressed =
        !(PINC & (1 << BTN_CW));
    bool ccwPressed =
        !(PINC & (1 << BTN_CCW));
    MotorDir desiredDir;
    if (cwPressed && ccwPressed)
    {
        desiredDir = STOP;
    }
    else if (cwPressed)
    {
        desiredDir = FORWARD;
    }
    else if (ccwPressed)
    {
        desiredDir = BACKWARD;
    }
    else
    {
        desiredDir = STOP;
    }
    if (desiredDir != currentDir)
    {
        if (desiredDir == FORWARD)
        {
            PORTD |= (1 << DIR_PIN);
        }
        else if (desiredDir == BACKWARD)
        {
            PORTD &= ~(1 << DIR_PIN);
        }
        PORTD &= ~(1 << PUL_PIN);
        stepPinState = false;
        lastStepTime = micros();
        currentDir = desiredDir;
    }
    if (currentDir != STOP)
    {
        if (micros() - lastStepTime >= stepHalfPeriod)
        {
            lastStepTime = micros();
            stepPinState = !stepPinState;
            if (stepPinState)
            {
                PORTD |= (1 << PUL_PIN);
            }
            else
            {
                PORTD &= ~(1 << PUL_PIN);
            }
        }
    }
}