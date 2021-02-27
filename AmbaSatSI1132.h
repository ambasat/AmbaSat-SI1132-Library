/*******************************************************************************
* AmbaSat-1 
* Filename: AmbaSatSI1132.h
*
* This library is designed specifically for AmbaSat-1 and the 
* SI1132 sensor
* with contributions by Michael F. Kamprath, https://github.com/michaelkamprath
*
* AmbaSat opensource code available on GitHub at:
* https://github.com/ambasat

* Copyright (c) 2021 AmbaSat Ltd
* https://ambasat.com
*
* licensed under Creative Commons Attribution-ShareAlike 3.0
* ******************************************************************************/

#ifndef __AmbaSatSI1132__
#define __AmbaSatSI1132__

#define SI1132_RESULTS_BUFFER_SIZE    9

#include <Arduino.h>
#include <Wire.h>
#include <Debugging.h>

class AmbaSatSI1132 
{
private:
    uint8_t measureRate0;
    uint8_t measureRate1;
     uint8_t adcGainVisible;
    uint8_t adcGainInfraRed;
    bool highSignalVisible;
    bool highSignalInfraRed;
    bool begin(void);
    void reset(void);
    bool waitUntilSleep(void);
    bool sendCommand(uint8_t cmd_value);
    bool setParameter(uint8_t param, uint8_t value);
    uint8_t readParameter(uint8_t param);
    uint8_t readResponseRegister(void);
    bool writeRegister(uint8_t address, uint8_t value);
    bool readRegister(uint8_t address, uint8_t& register_value);
    bool readRegisters(uint8_t address, uint8_t* data, size_t length)    ;
protected:
public:
    uint16_t uv;
    uint16_t vis;
    uint16_t ir;

    AmbaSatSI1132();
    void setup(void);
    bool readSensor(void);

    uint8_t getVisibleADCGain(void) const                                           { return adcGainVisible; }
    void setVisibleADCGain(uint8_t setting);
    uint8_t getInfraRedADCGain(void) const                                          { return adcGainInfraRed; }
    void setInfraRedADCGain(uint8_t setting);
    bool isVisibleHighSignalRange(void) const                                       { return highSignalVisible; }
    void setIsVisibleHighSignalRange(bool setting);
    bool isInfraRedHighSignalRange(void) const                                      { return highSignalInfraRed; }
    void setIsInfraRedHighSignalRange(bool setting);
};

#endif //__AmbaSatSI1132__