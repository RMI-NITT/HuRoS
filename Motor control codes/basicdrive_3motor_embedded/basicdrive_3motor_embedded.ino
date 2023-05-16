// Embedded C code for motor control and encoder feedback

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

int main()
{ 
  DDRC |= (1<<PC2); // MOT1_DIR 35
  DDRG |= (1<<PG0); // MOT1_BRK 41
  DDRE |= (1<<PE3); // MOT1_PWM 5
  DDRC |= (1<<PC0); // MOT2_DIR 37
  DDRL |= (1<<PL6); // MOT2_BRK 43
  DDRH |= (1<<PH3); // MOT2_PWM 6
  DDRG |= (1<<PG2); // MOT3_DIR 39
  DDRL |= (1<<PL4); // MOT3_BRK 45
  DDRH |= (1<<PH4); // MOT3_PWM 7
  PORTE |= (1<<PE3); // MOT1_PWM 5
  PORTH |= (1<<PH3); // MOT2_PWM 6
  PORTH |= (1<<PH4); // MOT3_PWM 7
  while(1)
  { 
    // Forward for 4 seconds
    PORTG &= ~(1<<PG0); // MOT1_BRK 41
    PORTC |= (1<<PC2); // MOT1_DIR 35

    PORTL &= ~(1<<PL6); // MOT2_BRK 43
    PORTC |= (1<<PC0); // MOT2_DIR 37

    PORTL &= ~(1<<PL4); // MOT3_BRK 45
    PORTG |= (1<<PG2); // MOT3_DIR 39
    _delay_ms(4000);
    
    // Brake for 2 seconds
    PORTG |= (1<<PG0); // MOT1_BRK 41
    PORTL |= (1<<PL6); // MOT2_BRK 43
    PORTL |= (1<<PL4); // MOT3_BRK 45
    _delay_ms(2000);
    
    // Backward for 4 seconds
    PORTG &= ~(1<<PG0); // MOT1_BRK 41
    PORTC &= ~(1<<PC2); // MOT1_DIR 35

    PORTL &= ~(1<<PL6); // MOT2_BRK 43
    PORTC &= ~(1<<PC0); // MOT2_DIR 37

    PORTL &= ~(1<<PL4); // MOT3_BRK 45
    PORTG &= ~(1<<PG2); // MOT3_DIR 39
    _delay_ms(4000);
    
    // Brake for 2 seconds
    PORTG |= (1<<PG0); // MOT1_BRK 41
    PORTL |= (1<<PL6); // MOT2_BRK 43
    PORTL |= (1<<PL4); // MOT3_BRK 45
    _delay_ms(2000);
  }
  return 0;
}
