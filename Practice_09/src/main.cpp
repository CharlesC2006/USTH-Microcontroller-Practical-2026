#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
bool lastMotionState = false;

void setup() {
    DDRD &= ~(1 << PD2);
    DDRD |= (1 << PD3);
    DDRD |= (1 << PD4);
    PORTD &= ~(1 << PD3);
    PORTD &= ~(1 << PD4);
    Serial.begin(9600);
    Serial.println("PIR warming up...");
    delay(2000);
    Serial.println("PIR ready");
}

void loop() {
    bool motionDetected = (PIND & (1 << PD2));
    if (motionDetected != lastMotionState) {

        lastMotionState = motionDetected;
        if (motionDetected) {
            PORTD |= (1 << PD3);
            PORTD |= (1 << PD4);
            Serial.println("Motion detected");
        }
        else {
            PORTD &= ~(1 << PD3);
            PORTD &= ~(1 << PD4);
            Serial.println("No motion");
        }
    }
    delay(1000);
}