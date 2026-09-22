
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#define F_CPU 16000000UL
#define BAUD 9600
#define BRC ((F_CPU / 16 / BAUD) - 1)
#define DARK_THRESHOLD 512
const uint8_t digit_map[10] = {
    0x3F,   // 0
    0x06,   // 1
    0x5B,   // 2
    0x4F,   // 3
    0x66,   // 4
    0x6D,   // 5
    0x7D,   // 6
    0x07,   // 7
    0x7F,   // 8
    0x6F    // 9
};

volatile uint8_t display_buffer[4] = {
    0, 0, 0, 0
};

void init_hardware(void);
uint16_t adc_read(uint8_t channel);
void uart_print(const char *str);
void update_display_buffer(uint16_t value);

ISR(TIMER0_OVF_vect)
{
    static uint8_t current_digit = 0;
    PORTC |= (1 << PC1) |
             (1 << PC2) |
             (1 << PC3) |
             (1 << PC4);
    uint8_t mask =
        ~digit_map[display_buffer[current_digit]];
    PORTD = (PORTD & 0x03) |
            ((mask & 0x3F) << 2);
    PORTB = (PORTB & 0xFE) |
            ((mask & 0x40) >> 6);
    PORTC &= ~(1 << (PC1 + current_digit));
    current_digit++;
    if (current_digit > 3)
        current_digit = 0;
}
int main(void)
{
    init_hardware();
    char serial_buffer[50];
    sei();
    uart_print("Light Sensor Active.\r\n");
    while (1)
    {
        uint16_t light_val = adc_read(0);
        update_display_buffer(light_val);
        if (light_val < DARK_THRESHOLD)
        {
            PORTB |= (1 << PB2);
            sprintf(
                serial_buffer,
                "ADC: %04d | State: DARK  | LED: ON\r\n",
                light_val
            );
        }
        else
        {
            PORTB &= ~(1 << PB2);
            sprintf(
                serial_buffer,
                "ADC: %04d | State: BRIGHT | LED: OFF\r\n",
                light_val
            );
        }
        uart_print(serial_buffer);
        _delay_ms(500);
    }
    return 0;
}
void init_hardware(void)
{
    UBRR0H = (BRC >> 8);
    UBRR0L = BRC;

    UCSR0B = (1 << TXEN0);

    UCSR0C = (1 << UCSZ01) |
             (1 << UCSZ00);
    ADMUX = (1 << REFS0);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
    TCCR0A = 0;

    TCCR0B = (1 << CS01) |
             (1 << CS00);

    TIMSK0 = (1 << TOIE0);
    DDRD |= 0xFC;
    DDRB |= (1 << PB0) |
            (1 << PB2);
    DDRC |= (1 << PC1) |
            (1 << PC2) |
            (1 << PC3) |
            (1 << PC4);
    PORTC |= (1 << PC1) |
             (1 << PC2) |
             (1 << PC3) |
             (1 << PC4);
    PORTD |= 0xFC;
    PORTB |= (1 << PB0);
    PORTB &= ~(1 << PB2);
}
uint16_t adc_read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF8) |
            (channel & 0x07);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
void update_display_buffer(uint16_t value)
{
    display_buffer[0] =
        (value / 1000) % 10;
    display_buffer[1] =
        (value / 100) % 10;
    display_buffer[2] =
        (value / 10) % 10;
    display_buffer[3] =
        value % 10;
}
void uart_print(const char *str)
{
    while (*str)
    {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *str++;
    }
}

