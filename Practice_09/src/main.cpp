#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

bool lastMotionState = false;
void UART_init(void)
{
    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void UART_transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;
}


void UART_print(const char *str)
{
    while (*str)
    {
        UART_transmit(*str);
        str++;
    }
}


int main(void)
{
    DDRD &= ~(1 << PD2);

    DDRD |= (1 << PD3);

    DDRD |= (1 << PD4);

    PORTD &= ~(1 << PD3);

    PORTD &= ~(1 << PD4);



    UART_init();

    UART_print("PIR warming up...\r\n");

    _delay_ms(2000);

    UART_print("PIR ready\r\n");



    while (1)
    {
        bool motionDetected = (PIND & (1 << PD2));


        if (motionDetected != lastMotionState)
        {
            lastMotionState = motionDetected;

            if (motionDetected)
            {
                PORTD |= (1 << PD3);

                PORTD |= (1 << PD4);

                UART_print("Motion detected\r\n");
            }
            else
            {
                PORTD &= ~(1 << PD3);

                PORTD &= ~(1 << PD4);

                UART_print("No motion\r\n");
            }
        }

        _delay_ms(1000);
    }

    return 0;
}