
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#define F_CPU 16000000UL

#define BAUD 9600
#define BRC ((F_CPU / 16 / BAUD) - 1)

#define DARK_THRESHOLD 512


// =====================================================
// 7-SEGMENT MAP
// Common Anode
//
// Bit 0 = A
// Bit 1 = B
// Bit 2 = C
// Bit 3 = D
// Bit 4 = E
// Bit 5 = F
// Bit 6 = G
//
// 0 = segment OFF
// 1 = segment ON
//
// Sau đó ISR sẽ đảo bằng ~
// vì Common Anode dùng active LOW.
// =====================================================

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


// =====================================================
// 4 DIGIT DISPLAY BUFFER
// =====================================================

volatile uint8_t display_buffer[4] = {
    0, 0, 0, 0
};


// =====================================================
// FUNCTION PROTOTYPES
// =====================================================

void init_hardware(void);
uint16_t adc_read(uint8_t channel);
void uart_print(const char *str);
void update_display_buffer(uint16_t value);


// =====================================================
// TIMER0 OVERFLOW INTERRUPT
// 7-SEGMENT MULTIPLEXING
//
// Timer frequency:
//
// 16 MHz / 64 / 256
// ≈ 976 Hz
//
// 4 digits:
// 976 / 4 ≈ 244 Hz per digit
//
// Common Anode:
// Digit HIGH = OFF
// Digit LOW  = ON
// =====================================================

ISR(TIMER0_OVF_vect)
{
    static uint8_t current_digit = 0;


    // -------------------------------------------------
    // 1. TURN OFF ALL DIGITS
    // Common Anode / Active LOW
    //
    // HIGH = OFF
    // -------------------------------------------------

    PORTC |= (1 << PC1) |
             (1 << PC2) |
             (1 << PC3) |
             (1 << PC4);


    // -------------------------------------------------
    // 2. GET SEGMENT PATTERN
    // -------------------------------------------------

    uint8_t mask =
        ~digit_map[display_buffer[current_digit]];


    // -------------------------------------------------
    // 3. SEGMENTS A-F
    //
    // A -> PD2
    // B -> PD3
    // C -> PD4
    // D -> PD5
    // E -> PD6
    // F -> PD7
    // -------------------------------------------------

    PORTD = (PORTD & 0x03) |
            ((mask & 0x3F) << 2);


    // -------------------------------------------------
    // 4. SEGMENT G
    //
    // G -> PB0
    //
    // PB1 is unused because there is NO DP.
    // PB2 is LED, so preserve it.
    // -------------------------------------------------

    PORTB = (PORTB & 0xFE) |
            ((mask & 0x40) >> 6);


    // -------------------------------------------------
    // 5. TURN ON CURRENT DIGIT
    //
    // Common Anode:
    // LOW = ON
    // -------------------------------------------------

    PORTC &= ~(1 << (PC1 + current_digit));


    // -------------------------------------------------
    // 6. NEXT DIGIT
    // -------------------------------------------------

    current_digit++;

    if (current_digit > 3)
        current_digit = 0;
}


// =====================================================
// MAIN
// =====================================================

int main(void)
{
    init_hardware();

    char serial_buffer[50];

    // Enable global interrupts
    sei();


    uart_print("Light Sensor Active.\r\n");


    while (1)
    {
        // -------------------------------------------------
        // 1. READ LDR
        // ADC range = 0 to 1023
        // -------------------------------------------------

        uint16_t light_val = adc_read(0);


        // -------------------------------------------------
        // 2. UPDATE 4-DIGIT DISPLAY
        // -------------------------------------------------

        update_display_buffer(light_val);


        // -------------------------------------------------
        // 3. DARK / BRIGHT + LED
        //
        // Dark = ADC < threshold
        // -------------------------------------------------

        if (light_val < DARK_THRESHOLD)
        {
            // LED ON
            PORTB |= (1 << PB2);

            sprintf(
                serial_buffer,
                "ADC: %04d | State: DARK  | LED: ON\r\n",
                light_val
            );
        }
        else
        {
            // LED OFF
            PORTB &= ~(1 << PB2);

            sprintf(
                serial_buffer,
                "ADC: %04d | State: BRIGHT | LED: OFF\r\n",
                light_val
            );
        }


        // -------------------------------------------------
        // 4. SEND DATA TO PC
        // -------------------------------------------------

        uart_print(serial_buffer);


        // -------------------------------------------------
        // 5. READ LDR EVERY 500 ms
        //
        // Display multiplexing continues through
        // TIMER0 interrupt.
        // -------------------------------------------------

        _delay_ms(500);
    }


    return 0;
}


// =====================================================
// HARDWARE INITIALIZATION
// =====================================================

void init_hardware(void)
{
    // -------------------------------------------------
    // UART
    // 9600 baud
    // 8 data bits
    // 1 stop bit
    // TX only
    // -------------------------------------------------

    UBRR0H = (BRC >> 8);
    UBRR0L = BRC;

    UCSR0B = (1 << TXEN0);

    UCSR0C = (1 << UCSZ01) |
             (1 << UCSZ00);


    // -------------------------------------------------
    // ADC
    //
    // AVCC = 5 V reference
    // ADC clock = 16 MHz / 128 = 125 kHz
    // -------------------------------------------------

    ADMUX = (1 << REFS0);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);


    // -------------------------------------------------
    // TIMER0
    //
    // Prescaler = 64
    // Overflow ≈ 976 Hz
    //
    // Used for 7-segment multiplexing.
    // -------------------------------------------------

    TCCR0A = 0;

    TCCR0B = (1 << CS01) |
             (1 << CS00);

    TIMSK0 = (1 << TOIE0);


    // -------------------------------------------------
    // GPIO
    // -------------------------------------------------

    // -----------------------------------------------
    // PD2-PD7 = A-F
    //
    // PD0 / PD1 remain untouched for UART.
    // -----------------------------------------------

    DDRD |= 0xFC;


    // -----------------------------------------------
    // PB0 = G
    // PB2 = LED
    //
    // PB1 unused because no DP.
    // -----------------------------------------------

    DDRB |= (1 << PB0) |
            (1 << PB2);


    // -----------------------------------------------
    // PC1-PC4 = C1-C4
    //
    // PC0 remains ADC0 input for LDR.
    // -----------------------------------------------

    DDRC |= (1 << PC1) |
            (1 << PC2) |
            (1 << PC3) |
            (1 << PC4);


    // -------------------------------------------------
    // INITIAL STATE
    // -------------------------------------------------

    // All digits OFF
    // Common Anode -> HIGH = OFF

    PORTC |= (1 << PC1) |
             (1 << PC2) |
             (1 << PC3) |
             (1 << PC4);


    // All segments OFF
    // Common Anode -> HIGH = OFF
    //
    // A-F
    PORTD |= 0xFC;

    // G
    PORTB |= (1 << PB0);


    // LED OFF
    PORTB &= ~(1 << PB2);
}


// =====================================================
// ADC READ
// =====================================================

uint16_t adc_read(uint8_t channel)
{
    // Select ADC channel
    //
    // Keep REFS0 = 1
    // Change only MUX bits.

    ADMUX = (ADMUX & 0xF8) |
            (channel & 0x07);


    // Start ADC conversion
    ADCSRA |= (1 << ADSC);


    // Wait until conversion finishes
    while (ADCSRA & (1 << ADSC));


    // Return 10-bit ADC value
    return ADC;
}


// =====================================================
// UPDATE DISPLAY BUFFER
// =====================================================

void update_display_buffer(uint16_t value)
{
    // ADC = 0 ... 1023

    display_buffer[0] =
        (value / 1000) % 10;

    display_buffer[1] =
        (value / 100) % 10;

    display_buffer[2] =
        (value / 10) % 10;

    display_buffer[3] =
        value % 10;
}


// =====================================================
// UART PRINT
// =====================================================

void uart_print(const char *str)
{
    while (*str)
    {
        // Wait until transmit buffer is empty
        while (!(UCSR0A & (1 << UDRE0)));

        // Send character
        UDR0 = *str++;
    }
}

