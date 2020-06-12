#include <nin_comm.h>
#include "UsbGamePad.h"

#define MAX_CONTROLLERS 4
#define NUM_RETRIES 3

#define CONTROLLER_ID_BASE 2
#define MESSAGE_SIZE 38

#define C_BORDER 50

#define C_UP(R) ((R)->cy_axis > 128 + C_BORDER)
#define C_DOWN(R) ((R)->cy_axis < 128 - C_BORDER)
#define C_LEFT(R) ((R)->cx_axis < 128 - C_BORDER)
#define C_RIGHT(R) ((R)->cx_axis > 128 + C_BORDER)

#define delay(X) delayMicroseconds(X * 1000)

static int port = CONTROLLER_ID_BASE;

void processControllerReport(ControllerReport *report, uchar controllerID)
{
    uchar usbReport[USB_REPORT_BUFFER_SIZE] = {0};
    uchar buttons = 0;
    uchar hatSwitch = 0;
    //uchar cHatSwitch = 0;
    int buttonNumber = 0;
    int offset = 0;

    if (report->left)
    {
        if (report->up)
        {
            hatSwitch = 8;
        }
        else if (report->down)
        {
            hatSwitch = 6;
        }
        else
        {
            hatSwitch = 7;
        }
    }
    else if (report->right)
    {
        if (report->up)
        {
            hatSwitch = 2;
        }
        else if (report->down)
        {
            hatSwitch = 4;
        }
        else
        {
            hatSwitch = 3;
        }
    }
    else if (report->up)
    {
        hatSwitch = 1;
    }
    else if (report->down)
    {
        hatSwitch = 5;
    }

    /*if (C_LEFT(report))
    {
        if (C_UP(report))
        {
            cHatSwitch = 8;
        }
        else if (C_DOWN(report))
        {
            cHatSwitch = 6;
        }
        else
        {
            cHatSwitch = 7;
        }
    }
    else if (C_RIGHT(report))
    {
        if (C_UP(report))
        {
            cHatSwitch = 2;
        }
        else if (C_DOWN(report))
        {
            cHatSwitch = 4;
        }
        else
        {
            cHatSwitch = 3;
        }
    }
    else if (C_UP(report))
    {
        cHatSwitch = 1;
    }
    else if (C_DOWN(report))
    {
        cHatSwitch = 5;
    }*/

    usbReport[offset++] = controllerID + 1;

    //usbReport[offset++] = (hatSwitch << 4) | cHatSwitch;
    usbReport[offset++] = hatSwitch;

    usbReport[offset++] = report->jx_axis;
    usbReport[offset++] = report->jy_axis;
    usbReport[offset++] = report->cx_axis;
    usbReport[offset++] = report->cy_axis;
    usbReport[offset++] = report->l_axis;
    usbReport[offset++] = report->r_axis;

    buttons |= report->start ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->a ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->b ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->x ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->y ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->z ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->l ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->r ? 1 << buttonNumber : 0;

    usbReport[offset++] = buttons;

    UsbGamePad.sendReport(usbReport);
}

void setup() {
    NIN_arduinoSetDataPort('B');
}

void loop()
{
    int i;
    ControllerType type = INVALID_TYPE;
    ControllerReport report = {0};
    bool id[ID_LENGTH];
    
    UsbGamePad.update();
    digitalWrite(13, !digitalRead(13));

    // Toggle controller port
    port = (port == CONTROLLER_ID_BASE + MAX_CONTROLLERS - 1) ? CONTROLLER_ID_BASE : port + 1;

    NIN_setDataPortMask(1 << port);

    for (i = 0; type == INVALID_TYPE && i < NUM_RETRIES; i++)
    {
        type = NIN_identifyController(id);
    }

    if (type == UNKNOWN_TYPE)
    {
        int j;
    
        /*Serial.begin(115200);
        Serial.print("Unknown controller: ");
        for (j = 0; j <  ID_LENGTH; j++)
        {
            Serial.print(id[j]);
        }
        Serial.print("\n");
        Serial.end();*/
    }
    else if (type != INVALID_TYPE)
    {
        bool valid = false;
        
        for (i = 0; !valid && i < NUM_RETRIES; i++)
        {
            valid = NIN_requestControllerReport(&report, type);
        }
        
        if (valid)
        {
            processControllerReport(&report, (uchar)(port - CONTROLLER_ID_BASE));
            delay(10);
        }
    }
}
