#include <io.h>

void my_i2c_init(void){
    TWBR = 0x5C;        //    Baud rate is set by calculating 
    TWCR = (1<<TWEN);    //Enable I2C
    TWSR = 0x00;        //Prescaler set to 1    
    delay_ms(100);
}
        //Start condition
void my_i2c_start(void){    
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);    //start condition
    while (!(TWCR & (1<<TWINT)));   
    delay_ms(1);
}
void my_i2c_send_address_write(char x){                //Cpn esta funcion se escribe en el bus de TWDR
    x=x<<1;
    TWDR = x;                        //Move value to I2C
    TWCR = (1<<TWINT) | (1<<TWEN);    //Enable I2C and clear interrupt
    while  (!(TWCR & (1<<TWINT)));
    delay_ms(1);
}
        //I2C stop condition
void my_i2c_write(char x){                //Cpn esta funcion se escribe en el bus de TWDR
    TWDR = x;                        //Move value to I2C
    TWCR = (1<<TWINT) | (1<<TWEN);    //Enable I2C and clear interrupt
    while  (!(TWCR & (1<<TWINT))); 
    delay_ms(1);
}

char my_i2c_read(){
    TWCR  = (1<<TWEN) | (1<<TWINT);    //Enable I2C and clear interrupt
    while (!(TWCR & (1<<TWINT)));    //Read successful with all data received in TWDR
    return TWDR;  
    delay_ms(1);
}

void my_i2c_stop(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);		//--- Stop Condition as per Datasheet 
    delay_ms(1);     
}