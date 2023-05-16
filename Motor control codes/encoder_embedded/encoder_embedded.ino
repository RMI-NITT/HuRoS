// Embedded C code for motor control and encoder feedback

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

int VAL = 0;
int ENCODE = 0, SUM = 0, PREV_ENCODE = 0;
int STATE = 0;

ISR(TIMER1_OVF_vect)
{
  if(STATE == 0)
  {
    PORTB = 0x01;
  }
  else if(STATE == 2)
  {
    PORTB = 0x02;
  }
  else 
  {
    PORTB = 0x00;
  }
  TCNT1 = 0x185EE; 
  STATE++;
  if(STATE == 4)
    STATE = 0;
}

ISR(INT0_vect)  //ISR for interrupt 0
{
  ENCODE = (PIND & 0x08) | (PIND & 0x04);
  SUM = ENCODE | PREV_ENCODE;
  if(SUM == 0b1101 || SUM == 0b0100 || SUM == 0b0010 || SUM == 0b1011)
    VAL--;
  if(SUM == 0b1110 || SUM == 0b0111 || SUM == 0b0001 || SUM == 0b1000)
    VAL++;
  PREV_ENCODE = ENCODE >> 2;
}

ISR(INT1_vect)  //ISR for interrupt 1
{
  ENCODE = (PIND & 0x08) | (PIND & 0x04);
  SUM = ENCODE | PREV_ENCODE;
  if(SUM == 0b1101 || SUM == 0b0100 || SUM == 0b0010 || SUM == 0b1011)
    VAL--;
  if(SUM == 0b1110 || SUM == 0b0111 || SUM == 0b0001 || SUM == 0b1000)
    VAL++;
  PREV_ENCODE = ENCODE >> 2;
}

int main()
{ 
  int VAL1=100;
  DDRB |= 0x03;
  DDRD |= 0x80;
  PORTD |= 0x80;
  PORTB = 0x02;
  Serial.begin(9600);
  sei();              // Enable global interrupts
  TCNT1 = 0x185EE;
  TIMSK1 = 1<<TOIE1;
  TCCR1B = (1<<CS12)|(1<<CS10);
  EIMSK|= (1<<INT0)|(1<<INT1);  // Enable INT0
  EICRA|= (1<<ISC00)|(1<<ISC10); // Pin level change trigger
  while(1)
  { 
    Serial.println(VAL);
  }
  return 0;
}
