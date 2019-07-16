//ExternalADC.h
#pragma once

#include "I2C.h"
#include <fstream>
#include <cstring>
#include <iomanip>

class ExternalADC :  public I2C {
    public:
        ExternalADC(int8_t, int8_t);
        ~ExternalADC();
        
        void set_preamplifier();
        void read_adc(uint16_t*);
        bool isConversed();
        void inVolts(uint16_t*,double*);
    private:
        int device_address;
        float preamplifier;
        uint8_t bit_15_8;
        uint8_t bit_7_0;
        uint8_t config_register;
        uint8_t convers_register;
};
