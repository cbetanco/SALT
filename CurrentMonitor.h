//CurrentMonitor.h
#pragma once

#include "I2C.h"

class CurrentMonitor : public I2C { 
    public:
      //  CurrentMonitor(){};
        CurrentMonitor(int8_t, int8_t);
        ~CurrentMonitor();
        
        
        void define_setup(); 
        void configure();
        void calibrate();
        void read_current(uint16_t *);
        void set_config_bits(uint16_t);
        void set_config_bits(uint8_t,uint8_t);
        void set_calib_bits(uint16_t );
        void set_calib_bits(uint8_t,uint8_t);
        void convert_to_amp(uint16_t*, float*);
 void read_BusVoltage_mV(int *BusVoltage_mV);

       

    private:
        uint8_t bit_15_8_conf;
        uint8_t bit_7_0_conf;
        
        uint8_t bit_15_8_calib;
        uint8_t bit_7_0_calib;
        
        uint8_t config_register = 0;
        uint8_t calib_register = 0b101;
        
    
};
