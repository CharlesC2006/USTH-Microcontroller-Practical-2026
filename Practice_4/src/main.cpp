#include <Arduino.h>
#include <avr/interrupt.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(6, 7, A2, A3, A4, A5);
volatile uint16_t adcValue = 0;
volatile uint16_t sensorValue = 0;
volatile bool adcFlag = false;
volatile bool pwmFlag = false;
volatile bool sensorFlag = false;
volatile bool uartFlag = false;
void Timer1_init()
{
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
  OCR1A = 249;
  TIMSK1 |= (1 << OCIE1A);
  sei();
}
void ADC_init(){
  DDRC &= ~((1 << PC0)|(1 << PC1));
  ADMUX = (1 << REFS0);
  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}
uint16_t ADC_read(uint8_t channel)
{
    if (channel == 0)
    {
        ADMUX &= ~((1 << MUX3) | (1 << MUX2) |(1 << MUX1) |(1 << MUX0));
    }
    else if (channel == 1)
    {
        ADMUX &= ~((1 << MUX3) |(1 << MUX2) |(1 << MUX1));
        ADMUX |= (1 << MUX0);
    }
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
void Task_ADC(){
  adcValue = ADC_read(0);
    lcd.setCursor(0, 0);
    lcd.print("ADC: ");
    lcd.print(adcValue);
    lcd.print("       ");
}
uint8_t ledDuty = 0;
int8_t ledStep = 5;
void PWM_LED_init()
{
    DDRB |= (1 << PB3);
    TCCR2A = (1 << COM2A1)| (1 << WGM21)| (1 << WGM20);
    TCCR2B = (1 << CS22);
    OCR2A = 0;
}
void Task_PWM_LED(){
ledDuty += ledStep;
    if (ledDuty >= 250 || ledDuty == 0)
        ledStep = -ledStep;
    OCR2A = ledDuty;
}
void Task_Sensor(){
 sensorValue = ADC_read(1);
    lcd.setCursor(0, 1);
    lcd.print("Sensor: ");
    lcd.print(sensorValue);
    lcd.print("       ");
}
void Task_UART()
{
    Serial.print("ADC0: ");
    Serial.print(adcValue);
    Serial.print(" | Sensor: ");
    Serial.print(sensorValue);
    Serial.print(" | LED: ");
    Serial.println(ledDuty);
}
ISR(TIMER1_COMPA_vect)
{
    static uint16_t count = 0;
    count++;
    if (count % 10 == 0)
        adcFlag = true;
    if (count % 20 == 0)
        pwmFlag = true;
    if (count % 100 == 0)
        sensorFlag = true;
    if (count >= 1000)
        {count = 0;
          uartFlag = true;}
}
void setup()
{
  Serial.begin(9600);
    lcd.begin(16, 2);
    ADC_init();
    PWM_LED_init();
    Timer1_init();
    lcd.setCursor(0, 0);
    lcd.print("ADC: ");
    lcd.setCursor(0, 1);
    lcd.print("Sensor: ");
}
void loop()
{
    if (adcFlag)
    {
        adcFlag = false;
        Task_ADC();
    }
    if (pwmFlag)
    {
        pwmFlag = false;
        Task_PWM_LED();
    }
    if (sensorFlag)
    {
        sensorFlag = false;
        Task_Sensor();
    }
    if (uartFlag)
    {
        uartFlag = false;
        Task_UART();
    }
}