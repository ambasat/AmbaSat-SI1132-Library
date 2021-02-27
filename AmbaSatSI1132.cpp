/*******************************************************************************
* AmbaSat-1 
* Filename: AmbaSatSI1132.cpp
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

#include "AmbaSatSI1132.h"

#define SI1132_PART_ID_REG          0x00
#define SI1132_REV_ID_REG           0x01
#define SI1132_SEQ_ID_REG           0x02
#define SI1132_INT_CFG_REG          0x03
#define SI1132_IRQ_ENABLE_REG       0x04
#define SI1132_HW_KEY_REG           0x07
#define SI1132_MEAS_RATE0_REG       0x08
#define SI1132_MEAS_RATE1_REG       0x09
#define SI1132_UCOEF0_REG           0x13
#define SI1132_UCOEF1_REG           0x14
#define SI1132_UCOEF2_REG           0x15
#define SI1132_UCOEF3_REG           0x16
#define SI1132_PARAM_WR_REG         0x17
#define SI1132_COMMAND_REG          0x18
#define SI1132_RESPONSE_REG         0x20
#define SI1132_IRQ_STATUS_REG       0x21
#define SI1132_ALS_VIS_DATA0_REG    0x22
#define SI1132_ALS_VIS_DATA1_REG    0x23
#define SI1132_ALS_IR_DATA0_REG     0x24
#define SI1132_ALS_IR_DATA1_REG     0x25
#define SI1132_PS1_DATA0_REG        0x26
#define SI1132_AUX_DATA0_REG        0x2C
#define SI1132_AUX_DATA1_REG        0x2D
#define SI1132_PARAM_RD_REG         0x2E
#define SI1132_CHIP_STAT_REG        0x30

#define SI1132_CMD_RESET            0x01
#define SI1132_CMD_ALS_FORCE        0x06
#define SI1132_CMD_ALS_AUTO         0x0E
#define SI1132_CMD_GET_CAL_INDEX    0x11
#define SI1132_CMD_GET_CAL          0x12
#define SI1132_CMD_PARAM_QUERY      0x80
#define SI1132_CMD_PARAM_SET        0xA0

#define SI1132_PARAM_CHLIST                 0x01
#define SI1132_PARAM_ALS_IR_ADCMUX          0x0E
#define SI1132_PARAM_ALS_VIS_ADC_COUNTER    0x10
#define SI1132_PARAM_ALS_VIS_ADC_GAIN       0x11
#define SI1132_PARAM_ALS_VIS_ADC_MISC       0x12
#define SI1132_PARAM_ALS_IR_ADC_COUNTER     0x1D
#define SI1132_PARAM_ALS_IR_ADC_GAIN        0x1E
#define SI1132_PARAM_ALS_IR_ADC_MISC        0x1F

#define AMBASAT_SI1132_ADDR 0x60 // AmbaSat SI1132 Address 

// ======================================================================================

AmbaSatSI1132::AmbaSatSI1132()
{
    measureRate0 = SI1132_MEAS_RATE0_REG;
    measureRate1 = SI1132_MEAS_RATE1_REG;

    if (!begin())
    {
        PRINTLN_DEBUG(F("ERROR: Unable to initialize the Si1132"));
    } 
}

// ======================================================================================

bool AmbaSatSI1132::begin(void)
{
    uint8_t device_id = 0;
    readRegister(SI1132_PART_ID_REG, device_id);

    if (device_id != 0x32) 
    {
        PRINT_DEBUG(F("ERROR initialising Si1132: device_id = 0x"));
        PRINTLN_DEBUG(device_id);
        return false;
    }

    uint8_t revision_id = 0;
    uint8_t sequence_id = 0;
    readRegister(SI1132_REV_ID_REG, revision_id);
    readRegister(SI1132_SEQ_ID_REG, sequence_id);
    
    PRINT_DEBUG(F("Found Si1132 UV Light Sensor with revision ID = "));
    PRINT_DEBUG(revision_id);
    PRINT_DEBUG(F(", sequence ID = "));
    PRINTLN_DEBUG(sequence_id);

    if (sequence_id == 0x01) 
    {
        // handle a device-level software bug in sequence=0x01 models as documented in spec
        measureRate0 = 0x0A;
        measureRate1 = 0x08;
    }

    return true;
}

// ======================================================================================

bool AmbaSatSI1132::writeRegister(uint8_t address, uint8_t value)
{
    Wire.beginTransmission(AMBASAT_SI1132_ADDR);
    Wire.write(address);
    Wire.write(value);

    if (Wire.endTransmission() != 0) 
    {
        PRINTLN_DEBUG(F("ERROR ending transmission for sensor"));
        return false;
    }

    return true;
}

// ======================================================================================

bool AmbaSatSI1132::readRegister(uint8_t address, uint8_t& register_value)
{

    Wire.beginTransmission(AMBASAT_SI1132_ADDR);
    Wire.write(address);

    if (Wire.endTransmission() != 0) 
    {
        PRINTLN_DEBUG(F("ERROR ending transmission for sensor"));
        return false;
    }

    if (Wire.requestFrom(AMBASAT_SI1132_ADDR, 1) != 1) 
    {
        PRINTLN_DEBUG(F("ERROR requesting data for sensor"));
        return false;
    }

    int value = Wire.read();

    if (Wire.endTransmission() != 0) 
    {
        PRINTLN_DEBUG(F("ERROR ending transmission for sensor"));
        return false;
    }

    register_value = value;
    return true;
}

// ======================================================================================

void AmbaSatSI1132::reset(void)
{
    writeRegister(SI1132_MEAS_RATE0_REG, 0x00);
    writeRegister(SI1132_MEAS_RATE1_REG, 0x00);
    writeRegister(SI1132_IRQ_ENABLE_REG, 0x00);
    writeRegister(SI1132_INT_CFG_REG, 0x00);
    writeRegister(SI1132_IRQ_STATUS_REG, 0xFF);

    writeRegister(SI1132_COMMAND_REG, SI1132_CMD_RESET);
    delay(20);
    writeRegister(SI1132_HW_KEY_REG, 0x17);
    delay(20);
}

// ======================================================================================

void AmbaSatSI1132::setup(void)
{
    reset();
 
    // set UCOEF[0:3] to the default values
    if (!(writeRegister(SI1132_UCOEF0_REG, 0x7B)
            &&writeRegister(SI1132_UCOEF1_REG, 0x6B)
            &&writeRegister(SI1132_UCOEF2_REG, 0x01)
            &&writeRegister(SI1132_UCOEF3_REG, 0x00)
    )) {
        PRINTLN_DEBUG(F("ERROR could not set Si1132 UCOEF values"));
        return;
    }
 
    // turn on UV Index, ALS IR, and ALS Visible
    setParameter(SI1132_PARAM_CHLIST, 0xB0);
        
    // set up VIS sensor
    //  clock divide = 1
    uint8_t adcGain = getVisibleADCGain();
    if (!setParameter(SI1132_PARAM_ALS_VIS_ADC_GAIN, adcGain&0b00000111)) {
        PRINTLN_DEBUG(F("ERROR could not set SI1132_PARAM_ALS_VIS_ADC_GAIN"));
        return;
    }
    //  ADC count is the 1's complement of ADC gain, shifted by 4
    if (!setParameter(SI1132_PARAM_ALS_VIS_ADC_COUNTER, ((~adcGain) << 4)&0b01110000)) {
        PRINTLN_DEBUG(F("ERROR could not set SI1132_PARAM_ALS_VIS_ADC_COUNTER"));
        return;
    }
    // set for high signal (e.g., bright sun)
    if (!setParameter(
            SI1132_PARAM_ALS_VIS_ADC_MISC,
            isVisibleHighSignalRange() ? 0b00100000 : 0x00 
    )) {
        PRINTLN_DEBUG(F("ERROR could not set SI1132_PARAM_ALS_VIS_ADC_MISC"));
        return;
    }

    // set up IR sensor
    //  clock divide = 1
    adcGain = getInfraRedADCGain();
    if (!setParameter(SI1132_PARAM_ALS_IR_ADC_GAIN, adcGain&0b00000111)) {
        PRINTLN_DEBUG(F("ERROR could not set SI1132_PARAM_ALS_IR_ADC_GAIN"));
        return;
    }
   //  ADS count at 511
    if (!setParameter(SI1132_PARAM_ALS_IR_ADC_COUNTER, ((~adcGain) << 4)&0b01110000)) {
        PRINTLN_DEBUG(F("ERROR could not set SI1132_PARAM_ALS_IR_ADC_COUNTER"));
        return;
    }
    //  small IR photodiode
    if (!setParameter(SI1132_PARAM_ALS_IR_ADCMUX, 0x00)) {
        PRINTLN_DEBUG(F("ERROR could not set SI1132_PARAM_ALS_IR_ADCMUX"));
        return;
    }
    //  set IR_RANGE bit for high signal. Must do "read and modify" per spec
    uint8_t cur_value = readParameter(SI1132_PARAM_ALS_IR_ADC_MISC);
    if (!setParameter(
        SI1132_PARAM_ALS_IR_ADC_MISC, 
        isInfraRedHighSignalRange() ? (0b00100000|cur_value) : (0b11011111&cur_value)
    )) {
        PRINTLN_DEBUG(F("ERROR could not set SI1132_PARAM_ALS_IR_ADC_MISC"));
        return;
    }

    // Place in forced measurement mode
    if (!writeRegister(measureRate0, 0x00)) 
    {
        return;
    }

    if (!writeRegister(measureRate1, 0x00)) 
    {
        return;
    }

}

// ======================================================================================

uint8_t AmbaSatSI1132::readResponseRegister(void)
{
    uint8_t register_value = 0;
    
    if (!readRegister(SI1132_RESPONSE_REG, register_value)) 
    {
        return 0xFF;
    }
    return register_value;
}

// ======================================================================================

bool AmbaSatSI1132::sendCommand(uint8_t cmd_value)
{
    writeRegister(SI1132_COMMAND_REG, 0x00);
    uint8_t response = readResponseRegister();

    while (response != 0x00) 
    {
        writeRegister(SI1132_COMMAND_REG, 0x00);
        delay(5);
        response = readResponseRegister();
    }

    if (!writeRegister(SI1132_COMMAND_REG, cmd_value)) 
    {
        PRINTLN_DEBUG(F("ERROR writing to command Si1132 register"));
        return false;
    }

    int16_t counter = 0;
    response = readResponseRegister();

    while ((counter < 10) && (response == 0x00)) 
    {
        writeRegister(SI1132_COMMAND_REG, cmd_value);
        delay(5);
        response = readResponseRegister();
        counter++;
    }
    return (response != 0x00);
}

// ======================================================================================

bool AmbaSatSI1132::setParameter(uint8_t param, uint8_t value)
{
    uint8_t cmd = SI1132_CMD_PARAM_SET | param;
    // set PARAM_WR register
    writeRegister(SI1132_PARAM_WR_REG, value);
    // send command
    bool success = sendCommand(cmd);
   
    if (!success) 
    {
        // read parameter
        sendCommand(SI1132_CMD_PARAM_QUERY|param);
        uint8_t paramValue = 0;
        readRegister(SI1132_PARAM_RD_REG, paramValue);
        PRINT_DEBUG(F("ERROR sending param write command for Si1132 parameter = 0x"));
        PRINT_HEX(param);
        PRINT_DEBUG(F(", final parameter value = "));
        PRINT_HEX(paramValue);
        PRINT_DEBUG(F(", desired parameter value = "));
        PRINT_HEX(value);
        PRINT_DEBUG(F("\n"));
        return 0;
    }

    // read results in PARAM_RD
    uint8_t param_res = 0;
    readRegister(SI1132_PARAM_RD_REG, param_res);
    bool result = (param_res == value);

    if (!result) 
    {
        PRINT_DEBUG(F("ERROR setting Si1132 parameter 0x"));
        PRINT_HEX(param);
        PRINT_DEBUG(F(" to 0x"));
        PRINT_HEX(value);
        PRINT_DEBUG(F(" but got 0x"));
        PRINT_HEX(param_res);
        PRINT_DEBUG(F("\n"));
    }
    return result;
}

// ======================================================================================

uint8_t AmbaSatSI1132::readParameter(uint8_t param)
{
    uint8_t cmd = SI1132_CMD_PARAM_QUERY | param;

    bool success = sendCommand(cmd);

    if (!success) 
    {
        // read parameter
        PRINT_DEBUG(F("ERROR reading Si1132 parameter 0x"));
        PRINT_HEX(param);
        PRINT_DEBUG(F("\n"));
        return 0;
    }

    // read results in PARAM_RD
    uint8_t register_value = 0;
    if (!readRegister(SI1132_PARAM_RD_REG, register_value)) 
    {
        return 0;
    }

    return register_value;
}

// ======================================================================================

bool AmbaSatSI1132::readSensor(void)
{
    // start the measurements
    sendCommand(SI1132_CMD_ALS_FORCE);

    // wait for measurements
    delay(1000);

    uint8_t buffer[4];

    // UV
    if (!readRegisters(SI1132_AUX_DATA0_REG, buffer, 2 )) 
    {
        PRINTLN_DEBUG("ERROR reading UV data on Si1132");
        return false;
    }

    uv = (buffer[1] << 8) | buffer[0]; 

    // Visible and IR
    if (!readRegisters(SI1132_ALS_VIS_DATA0_REG, buffer, 4 )) 
    {
        PRINTLN_DEBUG("ERROR reading vis and ir data on Si1132");
        return false;
    }

    vis = (buffer[1] << 8) | buffer[0]; 
    ir = (buffer[3] << 8) | buffer[2]; 

    return true;
}

// ======================================================================================

bool AmbaSatSI1132::readRegisters(uint8_t address, uint8_t* data, size_t length)
{
    Wire.beginTransmission(AMBASAT_SI1132_ADDR);

    Wire.write(address);

    if (Wire.endTransmission(false) != 0) 
    {
        PRINTLN_DEBUG(F("ERROR ending transmission for device"));
        return false;
    }

    if (Wire.requestFrom(AMBASAT_SI1132_ADDR, length) != length) 
    {
        PRINTLN_DEBUG(F("ERROR requesting data from device"));
        return false;
    }

    for (size_t i = 0; i < length; i++) 
    {
        *data++ = Wire.read();
    }

    if (Wire.endTransmission() != 0) 
    {
        PRINTLN_DEBUG(F("ERROR ending transmission for device"));
        return false;
    }

    return true;
}
