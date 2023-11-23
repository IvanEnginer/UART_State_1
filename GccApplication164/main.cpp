#define F_CPU 8000000UL
#define BAUD 9600
#define UBRR_VAL F_CPU/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>

void usart_init(unsigned int speed)
{
	UBRRH=(unsigned char)(speed>>8);
	UBRRL=(unsigned char) speed;
	UCSRA=0x00;
	UCSRB|=(1<<RXEN)|(1<<TXEN);
	UCSRB|=(1<<RXCIE);
	UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}

char Data;

bool _0_SW_ON = false;
bool _ISR_SW_ON = false;

bool State = false;
bool NewState = true;
bool SW_ON_PS1 = false;

bool EnableF1 = false;
bool F1 = false; 
bool _ISR_F1 =false;


ISR(USART_RXC_vect)
{
	Data = UDR;
	switch(Data)
	{
		case 'a':
			_0_SW_ON = true;
			_ISR_SW_ON = true;
		break;
		case 'b':
			_0_SW_ON = false;
			_ISR_SW_ON = true;
		break;
		case 'i':
			F1 = true;
			_ISR_F1 = true;
		break;
		case 'f':
			F1 = false;
			_ISR_F1 = true;
		break;		
	}
}

int StatCheck(void)
{
		if (bit_is_set(PINC, PINC1))
		{
			NewState = true;
			SW_ON_PS1 = true;
		}
		if (bit_is_clear(PINC, PINC1))
		{
			NewState =false;
			SW_ON_PS1 = false;
		}
		if (NewState != State)
		{
			if (SW_ON_PS1)
			{
				UDR = 'c';
				PORTB &= ~(1<<PB1); 
				EnableF1 = true;				
			} 
			else
			{
				UDR ='d';
				PORTB |= (1<<PB1);							
				EnableF1 = false;
			}
			State = NewState;
		}			
}
int main(void)
{
	DDRA = 0xFF;
	DDRB = 0xFF;
	DDRC = 0x00;
	PORTA = 0x00;
	PORTB = 0x07;
	
	usart_init(UBRR_VAL);
	sei();	
    while (1) 
    {		
		StatCheck();
		_delay_ms(1);
		if (_ISR_SW_ON)
		{
			if (_0_SW_ON)
			{
				PORTB &= ~(1<<PB0);
				PORTA |= (1<<PA1);				
			}
			else
			{
				PORTB |= (1<<PB0);
				PORTA &= ~(1<<PA1);
			}
			_ISR_SW_ON = false;
		}
		if (_ISR_F1)
		{
			if (EnableF1)
				{
				if (F1)
				{
					PORTB &= ~(1<<PB2);
					UDR = 'i'; 
				} 
				else
				{
					PORTB |= (1<<PB2);
					UDR = 'f';
				}
			}
		}
    }
}

