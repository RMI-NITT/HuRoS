/*#define BAUD 9600                          // define baud
#define BAUDRATE 103 //((F_CPU)/(BAUD*16UL)-1)    // set baudrate value for UBRR
 #include <util/delay.h> 
//#ifndef F_CPU
#define F_CPU 16000000UL*/                    // set the CPU clock
#include<avr/io.h>
#include <util/delay.h>

/* unsigned char ch;
void usart_init(void)
{UCSR0A=0x00;
  UCSR0B=(1<<RXEN0)|(1<<TXEN0);
UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);
//UBRR0H=(BAUDRATE>>8);
    UBRR0L=25;
}
void usart_send(char ch )
{
  while(UCSR0A!=(UCSR0A|(1<<UDRE0)));//waiting for UDRE to become high
  UDR0= ch;
}*/
long i;
long j;
ISR(INT0_vect)
{i++;
 Serial.println(i);
}
/*ISR(INT1_vect)
{j++;
  if(j%100==0)
 { Serial.println(j);
 }
}*/

int main(void)
{
  Serial.begin(38400);
  DDRB=0x03;
EIMSK|=(1<<INT0)|(1<<INT1);
EICRA=0xff;
sei();
 PORTB=0x00;
  while(1)
  { 
   
  }return 0;
}
