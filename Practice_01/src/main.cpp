#include <Arduino.h>

void ledButton() {
    DDRB |= (1 << PB1);       // PB1 = output
    PORTB |= (1 << PB1);      // LED OFF initially

    DDRB &= ~(1 << PB0);      // PB0 = input
    PORTB &= ~(1 << PB0);     // pull-up OFF
}

void setup() {
    ledButton();
}

void loop() {
    if (PINB & (1 << PB0)) {
        PORTB &= ~(1 << PB1);     // Button = 1 → LED ON
    } 
    else {
        PORTB |= (1 << PB1);      // Button = 0 → LED OFF
    }
}