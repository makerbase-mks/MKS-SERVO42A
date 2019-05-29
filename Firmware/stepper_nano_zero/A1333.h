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
 
#ifndef __A1333_H__
#define __A1333_H__

#include <Arduino.h>
#define A1333_DEGREES_PER_BIT  (360.0/(float)(0x7FFF))


#define PIN_A1333_CS (16)//analogInputToDigitalPin(PIN_A2))


class A1333 {
  private:
    int chipSelectPin;
    int16_t readAddress(uint16_t addr);
    bool error=false;
    bool a1333=true;
  public:
    boolean begin(int csPin);
    int16_t readEncoderAngle(void);
    void diagnostics(char *ptrStr);
    int16_t readEncoderAnglePipeLineRead(void);
    bool getError(void) {return error;};
};

#endif //__A1333_H__
