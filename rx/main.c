/***** Kinematic Lute RX *****/

#include <avr/io.h>
#include "m_general.h"
#include "m_bus.h"
#include "m_usb.h"
#include "m_rf.h"

// Wireless Variables
#define CHANNEL 2
#define RXADDRESS 0xA1
#define TXADDRESS 0xA2
#define PACKET_LENGTH 3

volatile int receive = 0;
int debugInit = 1;
int i = 0;
float blue = 0;
float green = 0;
float red = 0;
//                            b  g  r
char buffer[PACKET_LENGTH] = {0, 0, 0};

//When Packet Arrives get ready to play
ISR(INT2_vect) 
{
	m_green(TOGGLE);
	receive = 1;
}

//To figure out what the wireless is saying
int debug(void)
{
	if(debugInit){
		m_usb_init();
		debugInit = 0;
	}
	
	if(m_usb_isconnected())
	{
		m_usb_tx_string("\r\n Wireless: ");
		m_usb_tx_int(blue);
		m_usb_tx_string(" ");
		m_usb_tx_int(green);
		m_usb_tx_string(" ");
		m_usb_tx_int(red);
	}
	
	return 0;
}

int main(void)
{
	// Setup clock speed, interrupts, and ports
	DDRE |= 1<<6;
	PORTE &= !(1<<6);
	m_clockdivide(0);
	m_bus_init();
	m_disableJTAG();
	sei();
	set(DDRF, 7);
	set(DDRF, 6);
	set(DDRF, 5);
	
	// Wait for wireless to setup
	while(!m_rf_open(CHANNEL, RXADDRESS, PACKET_LENGTH)){}
	m_red(OFF);
	m_green(ON);
	while(1)
	{
		if(receive)
		{
			// Once we read a packet, set/clear the pins
			m_rf_read(buffer, PACKET_LENGTH);
			blue = (int)buffer[0];
			green = (int)buffer[1];
			red = (int)buffer[2]; 
			if (blue == 1) {
				set(PORTF, 7);
			} else {
				clear(PORTF,7);
			}
			if (green == 1) {
				set(PORTF, 6);
			} else {
				clear(PORTF,6);
			}
			if (red == 1) {
				set(PORTF, 5);
			} else {
				clear(PORTF,5);
			}
			//debug();
			// Wait 20ms so that the devices can read
			m_wait(20);
			receive = 0;
		} else {
			clear(PORTF,5);
			clear(PORTF,6);
			clear(PORTF,7);
		}
	}
	return 0;
}
