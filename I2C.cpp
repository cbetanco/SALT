// I2C.cpp
#include "I2C.h"

#include <cerrno>
#include <cstring>
#include <sstream>
#include <typeinfo>
#include <iostream>

I2C::I2C(){}
I2C::I2C(int8_t bus_number, int8_t device_address)
{
    I2C::bus_number = bus_number;
    I2C::device_address = device_address;
}
I2C::~I2C(){ close(bus); }

void I2C::open_bus()
{   
    const char* bus_dafault = "/dev/i2c-";
    std::ostringstream bus_n;
    bus_n << bus_dafault<<bus_number;
    const char *bus_name = bus_n.str().c_str();
    
  //  const char *bus_name = make_bus_name(); //= "/dev/i2c-1";// bus_n.c_str();
    
if ((bus = open(bus_name, O_RDWR)) < 0) {
        printf("file %s and bus_number  %i\n",bus_name,bus_number);
        int errvsv = errno;
        printf("Failed to open the i2c bus of ADC and return errno %s\n",strerror(errvsv));
        exit(1);
    }
}

const char* I2C::make_bus_name()
{   
    const char* bus_dafault = "/dev/i2c-";
    std::ostringstream bus_n;
    bus_n << bus_dafault<<bus_number;
    printf("inside bus name %s\n", bus_n.str().c_str());
    return bus_n.str().c_str();
   
}
void I2C::set_bus_number( int8_t bus_number ) { I2C::bus_number = bus_number;}

void I2C::get_bus_access()
{
    if (ioctl(bus, I2C_SLAVE, device_address) < 0) {
        int errvsv = errno;
        printf("Failed to acquire bus access and/or talk to slave and ioctl returned errno %s .\n",strerror(errvsv));
        exit(1);
    }
}

void I2C::set_device_address(int8_t device_address) {
    I2C::device_address = device_address;
    //printf("Device address is :  0x%08x\n",device_address);
}

void I2C::access_device()
{
    this->open_bus();
    this->get_bus_access();
}

void I2C::write_buffer(uint8_t register_address, uint16_t data){}
void I2C::write_buffer(uint8_t register_address, uint8_t bit_15_8, uint8_t bit_7_0)
{
    uint8_t buffer[3] = {register_address, bit_15_8, bit_7_0};
    if (write(bus, buffer, 3) != 3) {
        perror("Write to register");
        exit(-1);
    }
}

void I2C::write_buffer(uint8_t register_address, uint8_t bit_7_0)
{
    uint8_t buffer[2] = {register_address, bit_7_0};
    if (write(bus, buffer, 2) != 2) {
        perror("Write to register");
        exit(-1);
    }
}



void I2C::read_buffer(uint8_t register_address, uint16_t* data)
{
    //printf("in I2C:  register_address  -  0x%02x\n",register_address);
    uint8_t buffer[3] = {register_address, 0, 0};
  //  uint8_t buffer[3] = {0x01, 0, 0};
    
    
    
    if (write(bus, buffer, 1) != 1) {
        perror("Write register select");
        exit(-1);
    }
    
    if (read(bus, buffer, 2) != 2) {
        perror("Read conversion");
        exit(-1);
    }
    
    *data = buffer[0] << 8 | buffer[1];
    if( *data<0 ) data = 0;  
    
}

void I2C::read_buffer(uint8_t register_address, uint8_t* data)
{

  //printf("setting buffer register address\n");
    uint8_t buffer[2] = {register_address, 0};
    if (write(bus, buffer, 1) != 1) {
        perror("Write register select");
        exit(-1);
    }
    
    if (read(bus, buffer, 1) != 1) {
        perror("Read conversion");
        exit(-1);
    }
    
    *data = buffer[0];
    if( *data<0 ) data = 0;  
    
}
