// Embedded C code for motor control and ENCODER feedback

#define F_CPU 16000000
#define BAUD 57600
#define BAUDRATE (((F_CPU / (BAUD * 16UL))) - 1)  // Baud rate of 9600 programmed.

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<stdlib.h>

int STATE = 0;

int VAL1 = 0, VAL2 = 0, VAL3 = 0;
int ENCODE1 = 0, ENCODE2 = 0, ENCODE3 = 0; 
int SUM1 = 0, SUM2 = 0, SUM3 = 0;
int PREV_ENCODE1 = 0, PREV_ENCODE2 = 0, PREV_ENCODE3 = 0;

//ISR(TIMER1_OVF_vect)
//{
//  if(STATE == 0)
//  {
//    PORTB = 0x01;
//  }
//  else if(STATE == 2)
//  {
//    PORTB = 0x02;
//  }
//  else 
//  {
//    PORTB = 0x00;
//  }
//  TCNT1 = 0x185EE; 
//  STATE++;
//  if(STATE == 4)
//    STATE = 0;
//}

ISR(INT2_vect)  //ISR for interrupt 0
{
//  DUMMY++;
//  serial_tx("INT2\n");
  ENCODE1 = (PIND & 0x03)<< 2;
  SUM1 = ENCODE1 | PREV_ENCODE1;
  if(SUM1 == 0b1101 || SUM1 == 0b0100 || SUM1 == 0b0010 || SUM1 == 0b1011)
    VAL1--;
  if(SUM1 == 0b1110 || SUM1 == 0b0111 || SUM1 == 0b0001 || SUM1 == 0b1000)
    VAL1++;
  PREV_ENCODE1 = ENCODE1 >> 2;
}

//ISR(INT3_vect)  //ISR for interrupt 1
//{
////  DUMMY--;
//    serial_tx("INT3\n");
//  ENCODE1 = (PIND & 0x03)<< 2;
//  SUM1 = ENCODE1 | PREV_ENCODE1;
//  if(SUM1 == 0b1101 || SUM1 == 0b0100 || SUM1 == 0b0010 || SUM1 == 0b1011)
//    VAL1--;
//  if(SUM1 == 0b1110 || SUM1 == 0b0111 || SUM1 == 0b0001 || SUM1 == 0b1000)
//    VAL1++;
//  PREV_ENCODE1 = ENCODE1 >> 2;
//}

ISR(INT0_vect)  //ISR for interrupt 1
{
//  DUMMY++;
//    serial_tx("INT0\n");
  ENCODE3 = (PINE & 0x30) >> 4;
  SUM3 = ENCODE3 | PREV_ENCODE3;
  if(SUM3 == 0b1101 || SUM3 == 0b0100 || SUM3 == 0b0010 || SUM3 == 0b1011)
    VAL3--;
  if(SUM2 == 0b1110 || SUM2 == 0b0111 || SUM3 == 0b0001 || SUM3 == 0b1000)
    VAL3++;
  PREV_ENCODE3 = ENCODE3 >> 2;
}

//ISR(INT1_vect)  //ISR for interrupt 1
//{
////  DUMMY--;
////    serial_tx("INT3\n");
//  ENCODE3 = (PINE & 0x30) >> 2;
//  SUM3 = ENCODE3 | PREV_ENCODE3;
//  if(SUM3 == 0b1101 || SUM3 == 0b0100 || SUM3 == 0b0010 || SUM3 == 0b1011)
//    VAL3--;
//  if(SUM2 == 0b1110 || SUM2 == 0b0111 || SUM3 == 0b0001 || SUM3 == 0b1000)
//    VAL3++;
//  PREV_ENCODE3 = ENCODE3 >> 2;
//}

ISR(INT4_vect)  //ISR for interrupt 1
{
//  DUMMY++;
//    serial_tx("INT4\n");
  ENCODE2 = (PIND & 0x0C);
  SUM2 = ENCODE2 | PREV_ENCODE2;
  if(SUM2 == 0b1101 || SUM2 == 0b0100 || SUM2 == 0b0010 || SUM2 == 0b1011)
    VAL2--;
  if(SUM2 == 0b1110 || SUM2 == 0b0111 || SUM2 == 0b0001 || SUM2 == 0b1000)
    VAL2++;
  PREV_ENCODE2 = ENCODE2 >> 2;
}

//ISR(INT5_vect)  //ISR for interrupt 1
//{
////  DUMMY--;
////    serial_tx("INT3\n");
////  ENCODE2 = (PIND & 0x0C);
//  SUM2 = ENCODE2 | PREV_ENCODE2;
//  if(SUM2 == 0b1101 || SUM2 == 0b0100 || SUM2 == 0b0010 || SUM2 == 0b1011)
//    VAL2--;
//  if(SUM2 == 0b1110 || SUM2 == 0b0111 || SUM2 == 0b0001 || SUM2 == 0b1000)
//    VAL2++;
//  PREV_ENCODE2 = ENCODE2 >> 2;
//}

void serial_tx(char str[10])
{
  for(int i=0;i<strlen(str);i++)   // Loop through the string
   {
    while (!( UCSR0A & (1<<UDRE0)));  // Wait till buffer is empty
    UDR0 = str[i];    // Load the character to be transmitted
   }     
}

int i=0;
char i_str[10];

int main()
{ 
  DDRC |= (1<<PC2); // MOT1_DIR 35
  DDRG |= (1<<PG0); // MOT1_BRK 41
  DDRE |= (1<<PE3); // MOT1_PWM 5
  DDRC |= (1<<PC0); // MOT2_DIR 37
  DDRL |= (1<<PL6); // MOT2_BRK 43
  DDRH |= (1<<PH3); // MOT2_PWM 6
  PORTE |= (1<<PE3); // MOT1_PWM 5
  PORTH |= (1<<PH3); // MOT2_PWM 6
//  Serial.begin(9600);
  UBRR0L = (BAUDRATE);     // Set the required baud rate, upper register
  UBRR0H = (BAUDRATE>>8);  // Set the required baud rate, lower register
  UCSR0B|= (1<<TXEN0);              // Enable TRANSMISSION
  serial_tx("START\n");
  sei();              // Enable global interrupts
//  TCNT1 = 0x185EE;
//  TIMSK1 = 1<<TOIE1;
//  TCCR1B = (1<<CS12)|(1<<CS10);
  EIMSK|= (1<<INT4)|(1<<INT2)|(1<<INT0);  // Enable INT2 and INT3
  EICRA|= (1<<ISC20)|(1<<ISC00); // Pin level change trigger
  EICRB|= (1<<ISC40);
  while(1)
  { 
    // Forward for 4 seconds
    PORTG &= ~(1<<PG0); // MOT1_BRK 41
    PORTC |= (1<<PC2); // MOT1_DIR 35

    PORTL &= ~(1<<PL6); // MOT2_BRK 43
    PORTC |= (1<<PC0); // MOT2_DIR 37

    for(i=0;i<3000;i++)
    {
//      serial_tx("LOOP1\n");
      itoa(VAL1,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL2,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL3,i_str,DEC);
      serial_tx(i_str);
      serial_tx("\n");
    }
    
    // Brake for 2 seconds
    PORTG |= (1<<PG0); // MOT1_BRK 41
    PORTL |= (1<<PL6); // MOT2_BRK 43
    
    for(i=0;i<3000;i++)
    {
//      serial_tx("LOOP2\n");
      itoa(VAL1,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL2,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL3,i_str,DEC);
      serial_tx(i_str);
      serial_tx("\n");
    }
    
    // Backward for 4 seconds
    PORTG &= ~(1<<PG0); // MOT1_BRK 41
    PORTC &= ~(1<<PC2); // MOT1_DIR 35

    PORTL &= ~(1<<PL6); // MOT2_BRK 43
    PORTC &= ~(1<<PC0); // MOT2_DIR 37
    
    for(i=0;i<3000;i++)
    {
//      serial_tx("LOOP3\n");
      itoa(VAL1,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL2,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL3,i_str,DEC);
      serial_tx(i_str);
      serial_tx("\n");
    }
    
    // Brake for 2 seconds
    PORTG |= (1<<PG0); // MOT1_BRK 41
    PORTL |= (1<<PL6); // MOT2_BRK 43
    for(i=0;i<3000;i++)
    {
//      serial_tx("LOOP4\n");
      itoa(VAL1,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL2,i_str,DEC);
      serial_tx(i_str);
      serial_tx(" | ");
      itoa(VAL3,i_str,DEC);
      serial_tx(i_str);
      serial_tx("\n");
    }
  }
  return 0;
}
