/*******************************************************************************
* AmbaSat-1 
* Filename: main.cpp
*
* Example program to test the AmbaSat-1 library for the 
* SI1132 UV Sensor
* 
* Copyright (c) 2021 AmbaSat Ltd
* https://ambasat.com
*
* licensed under Creative Commons Attribution-ShareAlike 3.0
* ******************************************************************************/

#include <AmbaSatSI1132.h>
#include <Debugging.h>

AmbaSatSI1132 *ambasatSI1132;

// =============================================================================

void setup()
{
    Serial.begin(9600);

    while (!Serial)
	{
        delay(10);    
	}
		
    ambasatSI1132 = new AmbaSatSI1132();

    ambasatSI1132->setup();

    PRINTLN_DEBUG("Setup complete");  		
}

// =============================================================================

void loop()
{
    ambasatSI1132->readSensor();

    PRINT_DEBUG(F("UV: "));
    PRINTLN_DEBUG(ambasatSI1132->uv);    

    PRINT_DEBUG(F("VIS: "));
    PRINTLN_DEBUG(ambasatSI1132->vis);    

    PRINT_DEBUG(F("IR: "));
    PRINTLN_DEBUG(ambasatSI1132->ir);    

  delay(1000);
}
