//Salt.h
#pragma once
#ifndef SALT_H
#define SALT_H
#include "I2C.h"

class Salt : public I2C {
    public:
        Salt(int8_t, int8_t);
        ~Salt();
        void find_device_address(int8_t);
        void read_salt(int8_t, uint8_t, uint8_t*);
	void read_salt(int8_t, int8_t, uint16_t*);
        void read_salt(int16_t, uint16_t*);
        void read_salt(int16_t, uint8_t*);
        
        void write_salt(int8_t, int8_t, uint8_t);
	void write_salt(int8_t, int8_t, uint16_t);
        void write_salt(int16_t, uint16_t);
	 void write_salt(int16_t, uint8_t);
	void split_register(int16_t, int8_t*,int8_t*);
    private:
        int8_t chip_id =0b101;

    
};
#endif
