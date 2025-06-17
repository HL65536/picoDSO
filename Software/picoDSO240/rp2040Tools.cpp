#include "rp2040Tools.h"

#include "hardware/vreg.h"

void setCoreVoltageOffset(i8 offset)
{
    switch (offset)
    {
    case 1:
        vreg_set_voltage(VREG_VOLTAGE_1_15);
        break;
    case 2:
        vreg_set_voltage(VREG_VOLTAGE_1_20);
        break;
    case 3:
        vreg_set_voltage(VREG_VOLTAGE_1_25);
        break;
    case 4:
        vreg_set_voltage(VREG_VOLTAGE_1_30);
        break;
    case -1:
        vreg_set_voltage(VREG_VOLTAGE_1_05);
        break;
    case -2:
        vreg_set_voltage(VREG_VOLTAGE_1_00);
        break;
    case -3:
        vreg_set_voltage(VREG_VOLTAGE_0_95);
        break;
    case -4:
        vreg_set_voltage(VREG_VOLTAGE_0_90);
        break;
    case -5:
        vreg_set_voltage(VREG_VOLTAGE_0_85);
        break;

    default:
        vreg_set_voltage(VREG_VOLTAGE_1_10);
        break;
    }
}
