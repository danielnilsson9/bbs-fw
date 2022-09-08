#include <Arduino.h>
#include <AltSoftSerial.h>>

#include "ComProxy.h"

// NOTE:
// This project uses a Arduno Nano based on Atmega328p.
// This MCU only has a single UART.
//
// Two differenst software serial libraries are used
// to compensate for the lack of HW ports (should have used another MCU).
//
// This causes a few problems since the entire CPu
// is stalled when doing transmit. You may therefore see 
// error code 30 pop up on the display birefly if a lot is 
// printed to the log since the response is not fast enough.

SoftwareSerial logSerial(11, 12);
AltSoftSerial controllerSerial(8, 9);
// HW Serial used for display connection

ComProxy proxy(controllerSerial, Serial, logSerial);


void initProxy()
{
    controllerSerial.begin(1200);
    controllerSerial.listen();

    pinMode(A2, INPUT);
    pinMode(A3, INPUT);

    // display
    Serial.begin(1200);

    if (proxy.connect())
    {
        logSerial.println("Connected to controller.");
    }
    else
    {
        logSerial.println("Failed to connect to controller.");
    }
}

void printEvent(const ComProxy::Event& evt)
{    
    ComProxy::printFormat(logSerial, evt);
    logSerial.println();
}

void setup()
{
    logSerial.begin(57600);

    pinMode(2, INPUT_PULLUP);

    if (digitalRead(2))
    {
        // Disable proxy, needed when reflashing controller to not interfere with communication
        pinMode(0, INPUT);
        pinMode(1, INPUT);
        pinMode(8, INPUT);
        pinMode(9, INPUT);

        logSerial.println("Logger Disabled.");
    }
    else
    {
        delay(100);   
        initProxy();
    } 
}

void loop()
{
    if (proxy.isConnected())
    {
        proxy.process();
     
        if (proxy.hasLogEvent())
        {
            ComProxy::Event evt;
            if (proxy.getLogEvent(evt))
            {
                printEvent(evt); 
            }          
        }
    }
}
