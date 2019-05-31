// I2C.h
#pragma once


#ifndef I2C_H
#define I2C_H

#include <fcntl.h>  // for open with option  O_RDWR
#include <sys/ioctl.h> // for ioctl
#include <linux/i2c-dev.h>  // for I2C_SLAVE in ioctl
#include <unistd.h> // for read/write I2C - needed for c++
#include <linux/i2c.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h> //for error and exit
#include <inttypes.h>

class I2C {
    public:
        I2C();
        I2C(int8_t, int8_t);
        virtual ~I2C();
        void set_bus_number(int8_t);
        void set_device_address(int8_t);
        void write_buffer(uint8_t, uint16_t);
        void write_buffer(uint8_t, uint8_t, uint8_t);
        void write_buffer(uint8_t, uint8_t);
        
        void read_buffer(uint8_t, uint16_t*);
        void read_buffer(uint8_t, uint8_t, uint8_t);
        void read_buffer(uint8_t, uint8_t*);
        
        void access_device();
        
        void open_bus();
        void get_bus_access();
        const char* make_bus_name();
    private:
        int bus;
        int bus_number ;
        int device_address;    
};

#endif// I2C_H