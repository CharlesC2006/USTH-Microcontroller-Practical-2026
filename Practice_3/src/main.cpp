#include <Arduino.h>

void PWM_init()
{
  TCCR1A = (1 << COM1A1)|(1<<WGM11);
  TCCR1B = (1<<CS11)|(1<<WGM13)|(1<<WGM12);
  ICR1 = 39999;
  OCR1A = 2000;
  DDRB |= (1 << PB1);
}
void setup()
{
  PWM_init();
}
void loop()
{
    OCR1A = 2000;   
    delay(1000);
    OCR1A = 3000;   
    delay(1000);
    OCR1A = 4000;   
    delay(1000);
    OCR1A = 3000;  
    delay(1000);

}