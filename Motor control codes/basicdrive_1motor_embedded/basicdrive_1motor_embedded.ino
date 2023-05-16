// Embedded C code for motor control and encoder feedback

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

int main()
{ 
  DDRC |= (1<<PC2); // MOT1_DIR 35
  DDRG |= (1<<PG0); // MOT1_BRK 41
  DDRE |= (1<<PE3); // MOT1_PWM 5
  PORTE |= (1<<PE3); // MOT1_PWM 5
  while(1)
  { 
    // Forward for 4 seconds
    PORTG &= ~(1<<PG0); // MOT1_BRK 41
    PORTC |= (1<<PC2); // MOT1_DIR 35
    _delay_ms(4000);
    
    // Brake for 2 seconds
    PORTG |= (1<<PG0); // MOT1_BRK 41
    _delay_ms(2000);
    
    // Backward for 4 seconds
    PORTG &= ~(1<<PG0); // MOT1_BRK 41
    PORTC &= ~(1<<PC2); // MOT1_DIR 35
    _delay_ms(4000);
    
    // Brake for 2 seconds
    PORTG |= (1<<PG0); // MOT1_BRK 41
    _delay_ms(2000);
  }
  return 0;
}
