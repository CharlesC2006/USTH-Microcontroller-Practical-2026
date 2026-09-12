#include <Arduino.h>
#include <avr/interrupt.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
volatile uint16_t rawADC;
volatile float voltage;
volatile bool adcDone = false;
void ADC_init()
{
DDRC &= ~(1 << PC0);
ADMUX = (1 << REFS0); 
ADCSRA |= (1 << ADEN);
ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
ADCSRA |= (1 << ADIE);
sei();
}
ISR(ADC_vect)
{
  rawADC = ADC;
  voltage = rawADC * 5.0 / 1023.0;
  adcDone = true;
}
void setup()
{
  lcd.begin(16, 2);
  ADC_init();
  lcd.print("Voltage:");
}
void loop()
{
  ADCSRA |= (1 << ADSC);
    if (adcDone)
    {
        adcDone = false;

        lcd.setCursor(0, 1);
        lcd.print(voltage);
        lcd.print(" V   ");
    }
    delay(62);
}