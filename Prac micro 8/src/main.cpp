#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
void UART_init(void)
{
    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
char UART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
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
int UART_receive_angle(void)
{
    char buffer[5];
    int i = 0;
    char data;
    while (1)
    {
        data = UART_receive();
        if (data == '\r' || data == '\n')
        {
            if (i == 0)
                continue;
            break;
        }
        if (i < 4)
        {
            buffer[i++] = data;
        }
    }
    buffer[i] = '\0';
    return atoi(buffer);
}
void Servo_init(void)
{
    DDRB |= (1 << DDB1);
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) |
             (1 << WGM12) |
             (1 << CS11);
    ICR1 = 39999;
    OCR1A = 3000;
}
void Servo_set_angle(int angle)
{
    OCR1A = 1000 + ((long)angle * 4000) / 180;
}
void setup(void)
{
    UART_init();
    Servo_init();
    UART_transmit_string("Servo Control Ready\r\n");
}void loop(void)
{
    int angle = UART_receive_angle();

    if (angle >= 0 && angle <= 180)
    {
        Servo_set_angle(angle);

        UART_transmit_string("Servo moved to ");

        char buffer[5];
        itoa(angle, buffer, 10);

        UART_transmit_string(buffer);
        UART_transmit_string(" degrees\r\n");
    }
    else
    {
        UART_transmit_string(
            "Error: Angle must be between 0 and 180 degrees\r\n"
        );
    }
}