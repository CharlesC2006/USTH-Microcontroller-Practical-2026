#include <Arduino.h>
#include <avr/io.h>
void UART_init(void)
{
    // 9600 baud, F_CPU = 16 MHz
    UBRR0H = 0;
    UBRR0L = 103;

    // Enable transmitter
    UCSR0B = (1 << TXEN0);

    // 8 data bits, no parity, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
void PIR_init(void)
{
    // PD2 = input
    DDRD &= ~(1 << DDD2);
}
uint8_t PIR_read(void)
{
    return (PIND & (1 << PIND2));
}
void LED_init(void)
{
}

void UART_transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;
}

void UART_transmit_string(const char *str)
{
    while (*str)
    {
        UART_transmit(*str);
        str++;
    }
}
void setup(void)
{
    UART_init();
    PIR_init();
    LED_init();
}

void loop(void)
{
}