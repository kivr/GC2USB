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

#define NORM(X) ({short val = 128 * (X - 128) / 90; abs(val) < 20 ? 128 : val >= 128 ? 255 : val < -128 ? 0 : val + 128; })

#ifdef DEBUG
#define SERIAL_BEGIN() Serial.begin(115200)
#define SERIAL_PRINT(X) Serial.print(X)
#define SERIAL_PRINTLN(X) Serial.println(X)
#else
#define SERIAL_BEGIN() do {} while(0)
#define SERIAL_PRINT(X) do {} while(0)
#define SERIAL_PRINTLN(X) do {} while(0)
#endif

static int port = CONTROLLER_ID_BASE;

void processControllerReport(ControllerReport *report, uchar controllerID)
{
    static uchar usbReport[USB_REPORT_BUFFER_SIZE] = {0};
    uchar buttons;
    uchar hatSwitch;
    int buttonNumber = 0;
    int offset = 0;
    

    //usbReport[offset++] = controllerID + 1;
    usbReport[offset++] = 1;

    //usbReport[offset++] = (hatSwitch << 4) | cHatSwitch;
    //usbReport[offset++] = hatSwitch;

    usbReport[offset++] = NORM(report->jx_axis);
    usbReport[offset++] = 255 - NORM(report->jy_axis);
    usbReport[offset++] = NORM(report->cx_axis);
    usbReport[offset++] = 255 - NORM(report->cy_axis);

    if (!report->start) {
        hatSwitch = report->up   && report->right ? 1 :
              report->up   && report->left  ? 7 :
              report->down && report->right ? 3 :
              report->down && report->left  ? 5 :
              report->up                    ? 0 :
              report->down                  ? 4 :
              report->left                  ? 6 :
              report->right                 ? 2 : 8;
       
        buttons = hatSwitch & 0x0f;
    
        buttonNumber = 4;
        buttons |= report->b ? 1 << buttonNumber : 0;
        buttonNumber++;
        buttons |= report->a ? 1 << buttonNumber : 0;
        buttonNumber++;
        buttons |= report->x ? 1 << buttonNumber : 0;
        buttonNumber++;
        buttons |= report->y ? 1 << buttonNumber : 0;
    }
    usbReport[offset++] = buttons;
    
    buttons = 0;
    buttonNumber = 0;
    buttons |= report->l ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->r ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->z ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->start && report->b ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->start && report->y ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->start && report->x ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->start && report->left ? 1 << buttonNumber : 0;
    buttonNumber++;
    buttons |= report->start && report->right ? 1 << buttonNumber : 0;
    usbReport[offset++] = buttons;

    buttons = 0;
    buttonNumber = 0;
    buttons |= report->start && report->a ? 1 << buttonNumber : 0;
    usbReport[offset++] = buttons;

    UsbGamePad.sendReport(usbReport);
}

void setup() {
    SERIAL_BEGIN();
    NIN_arduinoSetDataPort('C');
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
        
        SERIAL_PRINT("Unknown controller: ");
        for (j = 0; j <  ID_LENGTH; j++)
        {
            SERIAL_PRINT(id[j]);
        }
        SERIAL_PRINT("\r\n");
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
            SERIAL_PRINT("r");
            delay(10);      
        }
        else
        {
            SERIAL_PRINT("i");
        }
    }
}
