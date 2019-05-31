//Salt.cpp
#include "Salt.h"
#include <iostream>

Salt::Salt(int8_t bus_number,int8_t device_address) : I2C( bus_number, device_address)
{
  //  std::cout <<  "SALT open bus" << std::endl;
    this->open_bus();
}
Salt::~Salt(){}

void Salt::find_device_address(int8_t high_reg_bits)
{
  //std::cout << "SALLLT find address" << std::endl;
  chip_id = 0b101;
  this -> set_device_address(chip_id << 4 | high_reg_bits);
  int8_t test = chip_id << 4 | high_reg_bits;
}

void Salt::read_salt(int8_t high_reg_bits, uint8_t low_reg_bits, uint8_t* command)
{
  //std::cout << "Salt::read_salt(int8_t high_reg_bits, uint8_t low_reg_bits, uint8_t* command)" << std::endl;
  this -> find_device_address(high_reg_bits);
  //std::cout << "about to get bus address" << std::endl;
  this -> get_bus_access();
  //std::cout << "about to read buffer" << std::endl;
  this -> read_buffer(low_reg_bits,command);
  //std::cout <<"read buffer" << std::endl;
}

void Salt::read_salt(int8_t high_reg_bits, int8_t low_reg_bits, uint16_t* command)
{
  //std::cout << " Salt::read_salt(int8_t high_reg_bits, int8_t low_reg_bits, uint16_t* command)" << std::endl;
  this -> find_device_address(high_reg_bits);
  this -> get_bus_access();
  this -> read_buffer(low_reg_bits,command);
}
void Salt::write_salt(int8_t high_reg_bits, int8_t low_reg_bits, uint16_t command) {

  //std::cout << "Salt::write_salt(int8_t high_reg_bits, int8_t low_reg_bits, uint16_t command)" << std::endl;
  this -> find_device_address(high_reg_bits);
  //std::cout << "found address" << std::endl;
  this -> get_bus_access();
  //std::cout << "got bus access" << std::endl;
  this -> write_buffer(low_reg_bits,command);
  //std::cout << "wrote buffer" << std::endl;
}




void Salt::read_salt(int16_t full_reg_bits, uint8_t* command)
{
   	int8_t high_reg_bits,low_reg_bits;
	//	std::cout << "about to split registers" << std::endl;
	this->split_register(full_reg_bits, &high_reg_bits, &low_reg_bits);
	//	std::cout << "just split registers, about to read salt" << std::endl;
	
	this->read_salt(high_reg_bits,low_reg_bits,command); 
}

void Salt::read_salt(int16_t full_reg_bits, uint16_t* command)
{       
	int8_t high_reg_bits,low_reg_bits;
	this->split_register(full_reg_bits, &high_reg_bits, &low_reg_bits);
        printf("high_reg_bits :  0x%08x\n",high_reg_bits);
        printf("low_reg_bits :  0x%08x\n",low_reg_bits);
        
	this->read_salt(high_reg_bits,low_reg_bits,command);
}



void Salt::split_register(int16_t full_reg_bits,int8_t* high_reg_bits,int8_t* low_reg_bits)
{
	*high_reg_bits = (full_reg_bits >> 8);
        *low_reg_bits = full_reg_bits & 0xFF;
}
void Salt::write_salt(int16_t full_reg_bits, uint16_t command) 
{
  //std::cout << "Salt::write_salt(int16_t full_reg_bits, uint16_t command)" << std::endl;

  int8_t high_reg_bits, low_reg_bits;
  
  this->split_register(full_reg_bits, &high_reg_bits, &low_reg_bits);
  this->write_salt(high_reg_bits, low_reg_bits, command);
  // this -> find_device_address(high_reg_bits);
  // this -> get_bus_access();
  // this -> write_buffer(low_reg_bits,command);
}

void Salt::write_salt(int16_t full_reg_bits, uint8_t command) 
{
  //std::cout << "Salt::write_salt(int16_t full_reg_bits, uint8_t command)" << std::endl;

  int8_t high_reg_bits, low_reg_bits;
  
  this->split_register(full_reg_bits, &high_reg_bits, &low_reg_bits);
  this->write_salt(high_reg_bits, low_reg_bits, command);
  // this -> find_device_address(high_reg_bits);
  // this -> get_bus_access();
  // this -> write_buffer(low_reg_bits,command);
}

void Salt::write_salt(int8_t high_reg_bits, int8_t low_reg_bits, uint8_t command)
{
  //  std::cout << "Salt::write_salt(int8_t high_reg_bits, int8_t low_reg_bits, uint8_t command)" << std::endl;
    this -> find_device_address(high_reg_bits);
    this -> get_bus_access();
    this -> write_buffer(low_reg_bits,command);
}
