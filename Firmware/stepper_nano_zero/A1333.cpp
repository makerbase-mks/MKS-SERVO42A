/**********************************************************************
 *      Author: tstern
 *
 *	Misfit Tech invests time and resources providing this open source code,
 *	please support Misfit Tech and open-source hardware by purchasing
 *	products from Misfit Tech, www.misifittech.net!
 *
 *	Written by Trampas Stern  for Misfit Tech.
 *	BSD license, check license.txt for more information
 *	All text above, must be included in any redistribution
 *********************************************************************/
#include <Arduino.h>
#include "syslog.h"
#include "a1333.h"
#include "SPI.h"
#include <stdio.h>
#include "board.h"



#ifndef MKS_GCC_O0	//mks mark ("-O0")
#pragma GCC push_options
#pragma GCC optimize ("-Ofast")
#endif

#define A1333_CMD_NOP   (0x0000)

#define kNOERROR 0
#define kPRIMARYREADERROR 1
#define kEXTENDEDREADTIMEOUTERROR 2
#define kPRIMARYWRITEERROR 3
#define kEXTENDEDWRITETIMEOUTERROR 4

//const uint16_t ChipSelectPin = 10;
const uint16_t LEDPin = 13;

const uint32_t WRITE = 0x40;
const uint32_t READ = 0x00;
const uint32_t COMMAND_MASK = 0xC0;
const uint32_t ADDRESS_MASK = 0x3F;

unsigned long nextTime;
bool ledOn = false;

bool includeCRC = false;

/*
 * PrimaryRead
 * 
 * Read from the primary serial registers
 */
uint16_t PrimaryRead(uint16_t cs, uint16_t address, uint16_t& value)
{
#if 0
    if (includeCRC)
    {
        uint8_t crcValue;
        uint8_t crcCommand;

        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));

        // On the Teensy, SPI0_CTAR0 is used to describe the SPI transaction
        // for transfer (byte) while SPI0_CTAR1 is used to describe the SPI
        // transaction for transfer16.
        // To do a 20 bit transfer, change the length of the transaction to
        // 4 bits for transfer and do a transfer16 followed by a transfer.
        uint32_t oldSPI0_CTAR0 = SPI0_CTAR0;
        SPI0_CTAR0 = (SPI0_CTAR0 & 0x87FFFFFF) | SPI_CTAR_FMSZ(3);  // using SPI0_CTAR0
                                                                    // to send 4 bits

        // Combine the register address and the command into one byte
        uint16_t command = ((address & ADDRESS_MASK) | READ) << 8;

        crcCommand = CalculateCRC(command);

        // take the chip select low to select the device
        digitalWrite(cs, LOW);

        // send the device the register you want to read
        SPI.transfer16(command);
        SPI.transfer(crcCommand);

        digitalWrite(cs, HIGH);
        digitalWrite(cs, LOW);

        // send the command again to read the contents
        value = SPI.transfer16(command);
        crcValue = SPI.transfer(crcCommand);

        // take the chip select high to de-select
        digitalWrite(cs, HIGH);

        // Restore the 8 bit description
        SPI0_CTAR0 = oldSPI0_CTAR0;

        SPI.endTransaction();

        // Check the CRC value
        if (CalculateCRC(value) != crcValue)
        {
            return kCRCERROR;
        }
    }
    else
#endif
    {
        //SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));

        // Combine the register address and the command into one byte
        uint16_t command = ((address & ADDRESS_MASK) | READ) << 8;

        // take the chip select low to select the device
		GPIO_LOW(PIN_A1333_CS);

        // send the device the register you want to read
        SPI.transfer16(command);

		GPIO_HIGH(PIN_A1333_CS);
        GPIO_LOW(PIN_A1333_CS);

        // send the command again to read the contents
        value = SPI.transfer16(command);

        // take the chip select high to de-select
        GPIO_HIGH(PIN_A1333_CS);

        SPI.endTransaction();
    }

    return kNOERROR;
}

/*
 * PrimaryWrite
 * 
 * Write to the primary serial registers
 */
uint16_t PrimaryWrite(uint16_t cs, uint16_t address, uint16_t value)
{
#if 0
    if (includeCRC)
    {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));

        // On the Teensy, SPI0_CTAR0 is used to describe the SPI transaction
        // for transfer (byte) while SPI0_CTAR1 is used to describe the SPI
        // transaction for transfer16.
        // To do a 20 bit transfer, change the length of the transaction to
        // 4 bits for transfer and do a transfer16 followed by a transfer.
        uint32_t oldSPI0_CTAR0 = SPI0_CTAR0;
        SPI0_CTAR0 = (SPI0_CTAR0 & 0x87FFFFFF) | SPI_CTAR_FMSZ(3);  // using SPI0_CTAR0
                                                                    // to send 4 bits

        // Combine the register address and the command into one byte
        uint16_t command = (((address & ADDRESS_MASK) | WRITE) << 8)  | ((value >> 8) & 0x0FF);
        uint8_t crcCommand = CalculateCRC(command);

        // take the chip select low to select the device:
        digitalWrite(cs, LOW);

        SPI.transfer16(command); // Send most significant byte of register data
        SPI.transfer(crcCommand); // Send the crc

        // take the chip select high to de-select:
        digitalWrite(cs, HIGH);

        command = ((((address + 1) & ADDRESS_MASK) | WRITE) << 8 ) | (value & 0x0FF);
        crcCommand = CalculateCRC(command);

        // take the chip select low to select the device:
        digitalWrite(cs, LOW);

        SPI.transfer16(command); // Send least significant byte of register data
        SPI.transfer(crcCommand); // Send the crc

        // take the chip select high to de-select:
        digitalWrite(cs, HIGH);

        // Restore the 8 bit description
        SPI0_CTAR0 = oldSPI0_CTAR0;

        SPI.endTransaction();
    }
    else
#endif		
    {
        //SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));

        // Combine the register address and the command into one byte
        uint16_t command = ((address & ADDRESS_MASK) | WRITE) << 8;

        // take the chip select low to select the device:
        //digitalWrite(cs, LOW);
		GPIO_LOW(PIN_A1333_CS);
        SPI.transfer16(command | ((value >> 8) & 0x0FF)); // Send most significant
                                                          // byte of register data

        // take the chip select high to de-select:
        //digitalWrite(cs, HIGH);
		GPIO_HIGH(PIN_A1333_CS);
        command = (((address + 1) & ADDRESS_MASK) | WRITE) << 8;
        // take the chip select low to select the device:
        //digitalWrite(cs, LOW);
		GPIO_LOW(PIN_A1333_CS);
        SPI.transfer16(command | (value & 0x0FF));  // Send least significant byte
                                                    // of register data

        // take the chip select high to de-select:
        //digitalWrite(cs, HIGH);
		GPIO_HIGH(PIN_A1333_CS);
        SPI.endTransaction();
    }

    return kNOERROR;
}

/*
 * ExtendedRead
 * 
 * Read from the EEPROM, Shadow or AUX
 */
uint16_t ExtendedRead(uint16_t cs, uint16_t address, uint32_t& value)
{
    uint16_t results;
    uint16_t readFlags;
    uint32_t timeout;
    uint16_t valueMSW;
    uint16_t valueLSW;
    uint32_t currentTime;

    // Write the address to the Extended Read Address register
    results = PrimaryWrite(cs, 0x0A, address & 0xFFFF);

    if (results != kNOERROR)
    {
        return results;
    }

    // Initiate the extended read
    results = PrimaryWrite(cs, 0x0C, 0x8000);
        
    if (results != kNOERROR)
    {
        return results;
    }

    timeout = millis() + 100L;

    do  // Wait for the read to be complete
    {
        results = PrimaryRead(cs, 0x0C, readFlags);
    
        if (results != kNOERROR)
        {
            return results;
        }

        // Make sure the read is not taking too long
        currentTime = millis();
        if (timeout < currentTime)
        {
            return kEXTENDEDREADTIMEOUTERROR;
        }
    } while ((readFlags & 0x0001) != 0x0001);
    
    // Read the most significant word from the extended read data
    results = PrimaryRead(cs, 0x0E, valueMSW);

    if (results != kNOERROR)
    {
        return results;
    }

    // Read the least significant word from the extended read data
    results = PrimaryRead(cs, 0x10, valueLSW);

    // Combine them
    value = ((uint32_t)valueMSW << 16) + valueLSW;

    return results;
}

/*
 * ExtendedWrite
 * 
 * Write to the EEPROM, Shadow or AUX
 */
uint16_t ExtendedWrite(uint16_t cs, uint16_t address, uint32_t value)
{
    uint16_t results;
    uint16_t writeFlags;
    uint32_t timeout;

    // Write into the extended address register
    results = PrimaryWrite(cs, 0x02, address & 0xFFFF);
    
    if (results != kNOERROR)
    {
        return results;
    }

    // Write the MSW (Most significant word) into the high order write data register
    results = PrimaryWrite(cs, 0x04, (value >> 16) & 0xFFFF);
        
    if (results != kNOERROR)
    {
        return results;
    }

    // Write the LSW (Least significant word) into the low order write data register
    results = PrimaryWrite(cs, 0x06, value & 0xFFFF);
        
    if (results != kNOERROR)
    {
        return results;
    }

    // Start the write process
    results = PrimaryWrite(cs, 0x08, 0x8000);
        
    if (results != kNOERROR)
    {
        return results;
    }

    timeout = millis() + 100;

    // Wait for the write to complete
    do
    {
        results = PrimaryRead(cs, 0x08, writeFlags);
    
        if (results != kNOERROR)
        {
            return results;
        }

        if (timeout < millis())
        {
            return kEXTENDEDWRITETIMEOUTERROR;
        }
    } while ((writeFlags & 0x0001) != 0x0001);

    return results;
}

/*
 * CalculateParity
 *
 * From the 16 bit input, calculate the parity
 */
bool CalculateParity(uint16_t input)
{
    uint16_t count = 0;
    
    // Count up the number of 1s in the input
    for (int index = 0; index < 16; ++index)
    {
        if ((input & 1) == 1)
        {
            ++count;
        }

        input >>= 1;
    }
    
    // return true if there is an odd number of 1s
    return (count & 1) != 0;
}

/*
 * CalculateCRC
 *
 * Take the 16 bit input and generate a 4bit CRC
 * Polynomial = x^4 + x^1 + 1
 * LFSR preset to all 1's
 */
uint8_t CalculateCRC(uint16_t input)
{
    bool CRC0 = true;
    bool CRC1 = true;
    bool CRC2 = true;
    bool CRC3 = true;
    int  i;
    bool DoInvert;
    uint16_t mask = 0x8000;
   
    for (i = 0; i < 16; ++i)
    {
        DoInvert = ((input & mask) != 0) ^ CRC3;         // XOR required?

        CRC3 = CRC2;
        CRC2 = CRC1;
        CRC1 = CRC0 ^ DoInvert;
        CRC0 = DoInvert;
        mask >>= 1;
    }

    return (CRC3 ? 8U : 0U) + (CRC2 ? 4U : 0U) + (CRC1 ? 2U : 0U) + (CRC0 ? 1U : 0U);
}

/*
 * SignExtendBitfield
 *
 * Sign extend a bitfield which is right justified
 */
int16_t SignExtendBitfield(uint16_t data, int width)
{
    int32_t x = (int32_t)data;
    int32_t mask = 1L << (width - 1);

    x = x & ((1 << width) - 1); // make sure the upper bits are zero

    return (int16_t)((x ^ mask) - mask);
}

	 uint16_t angle;
	 uint16_t angle15;
	 uint16_t temperature;
	 uint16_t fieldStrength;
	 uint16_t  turnsCount;

	volatile uint16_t angle_t;
	volatile uint16_t angle15_t;
	volatile uint16_t temperature_t;
	volatile uint16_t fieldStrength_t;
	volatile uint16_t  turnsCount_t;


void A1333_TEST()
{

	// Every second, read the angle, temperature, field strength and turns count
while(1)
	{	
	if (nextTime < millis())
	{
		if (PrimaryRead(PIN_A1333_CS, 0x20, angle) == kNOERROR)
		{
			if (CalculateParity(angle))
			{
				angle_t = (float)(angle & 0x0FFF) * 360.0 / 4096.0;
				Serial.print("Angle = ");
				Serial.print((float)(angle & 0x0FFF) * 360.0 / 4096.0);
				Serial.println(" Degrees");
			}
			else
			{
				Serial.println("Parity error on Angle read");
			}
		}
		else
		{
			Serial.println("Unable to read Angle");
		}

		if (PrimaryRead(PIN_A1333_CS, 0x32, angle15) == kNOERROR)
		{
				angle15_t = (float)((angle15) & 0x7FFF) * 360.0 / 32768.0;
				Serial.print("Angle = ");
				Serial.print((float)(angle15 & 0x7FFF) * 360.0 / 32768.0);
				Serial.println(" Degrees");
		}
		else
		{
			Serial.println("Unable to read Angle15");
		}


		if (PrimaryRead(PIN_A1333_CS, 0x28, temperature) == kNOERROR)
		{
			temperature_t = ((float)(temperature & 0x0FFF) / 8.0) + 25.0;
			Serial.print("Temperature = ");
			Serial.print(((float)(temperature & 0x0FFF) / 8.0) + 25.0);
			Serial.println(" C");
		}
		else
		{
			Serial.println("Unable to read Temperature");
		}

		if (PrimaryRead(PIN_A1333_CS, 0x2A, fieldStrength) == kNOERROR)
		{
			fieldStrength_t = fieldStrength & 0x0FFF;
			Serial.print("Field Strength = ");
			Serial.print(fieldStrength & 0x0FFF);
			Serial.println(" Gauss");
		}
		else
		{
			Serial.println("Unable to read Field Strength");
		}

		if (PrimaryRead(PIN_A1333_CS, 0x2C, turnsCount) == kNOERROR)
		{
			if (CalculateParity(turnsCount))
			{
				turnsCount_t = SignExtendBitfield(turnsCount, 12);
				Serial.print("Turns Count = ");
				Serial.println(SignExtendBitfield(turnsCount, 12));
			}
			else
			{
				Serial.println("Parity error on Turns Count read");
			}
		}
		else
		{
			Serial.println("Unable to read Turns Count");
		}

		nextTime = millis() + 500L;
        // Blink the LED every half second
        if (ledOn)
        {
            digitalWrite(LEDPin, LOW);
            ledOn = false;
        }
        else
        {
            digitalWrite(LEDPin, HIGH);
            ledOn = true;
        }


	}
}
}



boolean A1333::begin(int csPin)
{

	uint16_t unused=0;
	uint16_t flags=0;
	uint16_t angle=0;
	uint32_t flagsAndZeroOffset=0;

	digitalWrite(PIN_A1333_CS,LOW); //pull CS LOW by default (chip powered off)
	digitalWrite(PIN_MOSI,LOW);
	digitalWrite(PIN_SCK,LOW);
	digitalWrite(PIN_MISO,LOW);
	pinMode(PIN_MISO,OUTPUT);
	delay(1000);


	digitalWrite(PIN_A1333_CS,HIGH); //pull CS high

	pinMode(PIN_MISO,INPUT);

	error=false;
	SPISettings settingsA(5000000, MSBFIRST, SPI_MODE3);             ///400000, MSBFIRST, SPI_MODE3);
	chipSelectPin=csPin;

	LOG("csPin is %d",csPin);
	pinMode(chipSelectPin,OUTPUT);
	digitalWrite(chipSelectPin,HIGH); //pull CS high by default
	delay(1);
	SPI.begin();    					//A1333 SPI uses mode=3 (CPOL=1, CPHA=1)
	LOG("Begin A1333...");

	SPI.beginTransaction(settingsA);
	SPI.transfer16(A1333_CMD_NOP);
	delay(10);

	// Make sure all of the SPI pins are
	// ready by doing a read
	PrimaryRead(PIN_A1333_CS, 0x0, unused);

	// Unlock the device
    PrimaryWrite(PIN_A1333_CS, 0x3C, 0x2700);
    PrimaryWrite(PIN_A1333_CS, 0x3C, 0x8100);
    PrimaryWrite(PIN_A1333_CS, 0x3C, 0x1F00);
    PrimaryWrite(PIN_A1333_CS, 0x3C, 0x7700);

	// Make sure the device is unlocked
	uint16_t t0=100;
	PrimaryRead(PIN_A1333_CS, 0x3C, flags);
	while ((flags & 0x0001) != 0x0001)
	{
		delay(1);
		t0--;
		if (t0==0)
		{
			ERROR("Device is not Unlocked");
			error=true;
			a1333 = false;
			return false;
		}
		PrimaryRead(PIN_A1333_CS, 0x3C, flags);
	}

#if 0
	// Zero the angle
	// Extended location 0x06 contains flags in the MSW and the Zero Angle values in the LSW
	// so get both and zero out ZeroAngle
    ExtendedRead(PIN_A1333_CS, 0x5C, flagsAndZeroOffset);
    flagsAndZeroOffset = flagsAndZeroOffset & 0x00FFF000;
    ExtendedWrite(PIN_A1333_CS, 0x5C, flagsAndZeroOffset);    // Zero out the Shadow
	
	// Get the current angle. It is now without the ZeroAngle correction
    PrimaryRead(PIN_A1333_CS, 0x20, angle);
	
	
	// Copy the read angle into location 0x5C preserving the flags
    flagsAndZeroOffset = flagsAndZeroOffset | (angle & 0x000FFF);
    ExtendedWrite(PIN_A1333_CS, 0x5C, flagsAndZeroOffset);
#endif	

	//A1333_TEST();
	
	return true;

}




//read the encoders 
int16_t A1333::readAddress(uint16_t addr)
{
	uint16_t data = 0;
	
	PrimaryRead(PIN_A1333_CS, addr, data);

	return data;
}

//read the encoders 
int16_t A1333::readEncoderAngle(void)
{
	uint16_t read_angle=0;
	if (a1333)
		{
		PrimaryRead(PIN_A1333_CS, 0x32, read_angle);
		}
	return (read_angle & 0x7FFF);	 
}

//pipelined read of the encoder angle used for high speed reads, but value is always one read behind
int16_t A1333::readEncoderAnglePipeLineRead(void)
{

	int16_t data = 0;
	return data;
}


void A1333::diagnostics(char *ptrStr)
{

}

#pragma GCC pop_options

