#include <Wire.h>
#include <Eeprom24C32_64.h>

#define EEPROM_ADDRESS  0x76

static Eeprom24C32_64 eeprom(EEPROM_ADDRESS);

void setup()
{
   
    Serial.begin(9600);
    eeprom.initialize();
    const word address = 10;
    Serial.println("Write byte to EEPROM memory...");
    eeprom.writeByte(address, 0xAA);
    
    delay(10);
    
    Serial.println("Read byte from EEPROM memory...");
    byte data = eeprom.readByte(address);
    
    Serial.print("Read byte = 0x");
    Serial.print(data, HEX);
    Serial.println("");
}

/**************************************************************************//**
 * \fn void loop()
 *
 * \brief
 ******************************************************************************/
void loop()
{

}
