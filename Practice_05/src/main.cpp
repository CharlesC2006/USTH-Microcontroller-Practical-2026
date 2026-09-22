#include <Arduino.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(6, 7, A2, A3, A4, A5);
volatile uint16_t capture1 = 0;
volatile uint16_t capture2 = 0;
volatile uint16_t period = 0;
volatile bool capture_done = false;
void PWM_init()
{
  TCCR2A = (1 << COM2B1)|(1<<WGM21)|(1<<WGM20);
  TCCR2B = (1<<CS22)|(1<<WGM22);
  OCR2A = 199;
  OCR2B = 49;
  DDRD |= (1 << PD3);
}
void INPUTCAPTURE_init()
{
  TCCR1A = 0;
  TCCR1B = (1 << ICES1) | (1 << CS11);
  TIMSK1 = (1 << ICIE1);
  DDRB &= ~(1 << PB0);
}
ISR(TIMER1_CAPT_vect)
{
    if (capture_done == false)
    {
        capture1 = ICR1;
        capture_done = true;
    }
    else
    {
        capture2 = ICR1;
        period = capture2 - capture1;
        capture_done = false;
    }
}
void setup()
{
    lcd.begin(16, 2);
    PWM_init();
    INPUTCAPTURE_init();
    sei();
}
void loop()
{
    if (period > 0)
    {
        float frequency = 2000000.0 / period;
        lcd.setCursor(0, 0);
        lcd.print("Freq: ");
        lcd.print(frequency);
        lcd.print(" Hz   ");
        lcd.setCursor(0, 1);
        lcd.print("Period: ");
        lcd.print(period);
        lcd.print(" tick ");
    }
}