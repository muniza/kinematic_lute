/***** Kinematic Lute Remote Code *****/
#include <avr/io.h>
#include "m_general.h"
#include "m_bus.h"
#include "m_usb.h"
#include "m_rf.h"

// Wireless Variables
#define CHANNEL 2
#define RXADDRESS 0xA2
#define TXADDRESS 0xA1
#define PACKET_LENGTH 3

// Light Variables
#define F_CPU 16000000
#define LED_STRIP_PORT PORTD
#define LED_STRIP_DDR  DDRD
#define LED_STRIP_PIN  7

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <math.h>

typedef struct rgb_color
{
  unsigned char red, green, blue;
} rgb_color;

int send = 0;
int i = 0;
int mode = 0;
int threshold = 100;
float blue = 0;
float green = 0;
float red = 0;
volatile int A = 0;
volatile int B = 0;
volatile int C = 0;
volatile int D = 0;
volatile int E = 0;
volatile int ready = 1;
//                            b  g  r
char buffer[PACKET_LENGTH] = {0, 0, 0};

// Interrupt to checks which pin is high
ISR(PCINT0_vect)
{
	if(ready) {
		if(check(PINB,1)) A=1;
		if(check(PINB,2)) B=1;
		if(check(PINB,3)) C=1;
		if(check(PINB,0)) D=1;
		E=1;
	}
	m_green(TOGGLE);
}

// Neopixel assembly code from pololu AVR
void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count) 
{
  // Set the pin to be an output driving low.
  LED_STRIP_PORT &= ~(1<<LED_STRIP_PIN);
  LED_STRIP_DDR |= (1<<LED_STRIP_PIN);

  cli();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.
  while(count--)
  {
    // Send a color to the LED strip.
    // The assembly below also increments the 'colors' pointer,
    // it will be pointing to the next color at the end of this loop.
    asm volatile(
        "ld __tmp_reg__, %a0+\n"
        "ld __tmp_reg__, %a0\n"
        "rcall send_led_strip_byte%=\n"  // Send red component.
        "ld __tmp_reg__, -%a0\n"
        "rcall send_led_strip_byte%=\n"  // Send green component.
        "ld __tmp_reg__, %a0+\n"
        "ld __tmp_reg__, %a0+\n"
        "ld __tmp_reg__, %a0+\n"
        "rcall send_led_strip_byte%=\n"  // Send blue component.
        "rjmp led_strip_asm_end%=\n"     // Jump past the assembly subroutines.

        // send_led_strip_byte subroutine:  Sends a byte to the LED strip.
        "send_led_strip_byte%=:\n"
        "rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
        "ret\n"

        // send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
        // high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
        // but this function always takes the same time (2 us).
        "send_led_strip_bit%=:\n"
        "sbi %2, %3\n"                           // Drive the line high.
        "rol __tmp_reg__\n"                      // Rotate left through carry.

#if F_CPU == 16000000
        "nop\n" "nop\n"
#elif F_CPU == 20000000
        "nop\n" "nop\n" "nop\n" "nop\n"
#else
#error "Unsupported F_CPU"
#endif

        "brcs .+2\n" "cbi %2, %3\n"              // If the bit to send is 0, drive the line low now.

#if F_CPU == 16000000
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
#elif F_CPU == 20000000
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n"
#endif

        "brcc .+2\n" "cbi %2, %3\n"              // If the bit to send is 1, drive the line low now.

        "ret\n"
        "led_strip_asm_end%=: "
        : "=b" (colors)
        : "0" (colors),         // %a0 points to the next color to display
          "I" (_SFR_IO_ADDR(LED_STRIP_PORT)),   // %2 is the port register (e.g. PORTC)
          "I" (LED_STRIP_PIN)     // %3 is the pin number (0-8)
    );

    // Uncomment the line below to temporarily enable interrupts between each color.
    //sei(); asm volatile("nop\n"); cli();
  }
  sei();          // Re-enable interrupts now that we are done.
  _delay_us(50);  // Hold the line low for 15 microseconds to send the reset signal.
}

#define LED_COUNT 1
rgb_color colors[LED_COUNT];

// Changes the light color depending on number of times pressed
void light()
{
	if (mode == 7) {
		mode = 0;
	}
	if (mode == 0) {
		colors[0] = (rgb_color){255, 255, 255};
	}
	else if (mode == 1) {
		colors[0] = (rgb_color){255, 0, 0};
	}
	else if (mode == 2) {
		colors[0] = (rgb_color){0, 255, 0};
	}
	else if (mode == 3) {
		colors[0] = (rgb_color){0, 0, 255};
	}
	else if (mode == 4) {
		colors[0] = (rgb_color){255, 255, 0};
	}
	else if (mode == 5) {
		colors[0] = (rgb_color){255, 0, 255};
	}
	else if (mode == 6) {
		colors[0] = (rgb_color){0, 255, 255};
	}
	mode = mode+1;
	led_strip_write(colors, LED_COUNT);
}

// Sets up the B0-B3 as pinchange interrupts
void setup_interrupts(void)
{
	clear(DDRB, 0);
	clear(DDRB, 1);
	clear(DDRB, 2);
	clear(DDRB, 3);
	set(PCICR, PCIE0);
	set(PCMSK0, PCINT0);
	set(PCMSK0, PCINT1);
	set(PCMSK0, PCINT2);
	set(PCMSK0, PCINT3);
}

int send_data(void)
{
	// Check for button presses and sets the flags accordingly
	if(A) {
		red = 0;
		blue = 1;
		green = 0;
	}
	else if(B) {
		red = 0;
		blue = 0;
		green = 1;
	}
	else if(C) {
		red = 1;
		blue = 0;
		green = 0;
	}
	if(D) {
		light();
	}
	A = 0;
	B = 0;
	C = 0;
	D = 0;
	m_red(TOGGLE);
	// Sends the button data through wireless link
	buffer[0] = (char)(blue);
	buffer[1] = (char)(green);
	buffer[2] = (char)(red);
	m_rf_send(TXADDRESS, buffer, PACKET_LENGTH);
	blue = 0;
	green = 0;
	red = 0;
	return 0;
}

int main(void)
{
	// Set the clock, ports, interrupts, and lights to default
	DDRE |= 1<<6;
	PORTE &= !(1<<6);
	m_clockdivide(0);
	light();
	m_bus_init();
	m_disableJTAG();
	setup_interrupts();
	sei();

	// Wait for the rf to finish setup
	while(!m_rf_open(CHANNEL, RXADDRESS, PACKET_LENGTH)){}
	
	while(1)
	{
		// Check for button press
		if (E) {
			ready=0;
			// Wait to prevent debouncing
			m_wait(100);
			send_data();
			E=0;
			// Ready to receive another button press
			ready=1;
		}
	}
	return 0;
}
