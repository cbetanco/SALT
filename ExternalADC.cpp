//ExternalADC.h
#include "ExternalADC.h"

ExternalADC::ExternalADC(int8_t device_address, int8_t bus_number) : I2C( bus_number, device_address) {} 

ExternalADC::~ExternalADC(){}

void ExternalADC::read_adc(uint16_t *adc_counts)
{   
    bit_15_8 = 0b10010011;
    bit_7_0 = 0b10000101;
    config_register = 1;
    convers_register = 0;
    write_buffer(config_register, bit_15_8, bit_7_0);
    if(isConversed())
    {
       read_buffer(convers_register, adc_counts); 
    }
 //   printf("Values: HEX 0x%02x\n  or in volts: %4.3f \n",*adc_counts, (*adc_counts)*4.096/32768.0);
    std::cout << "Power consumption in volts: " << std::showpos << std::fixed << std::dec <<std::setprecision(4) << (*adc_counts)*4.096/32768.0 << std::endl;
}

bool ExternalADC::isConversed()
{
    uint16_t data = 0;
    do{
        read_buffer(config_register,&data);
    } while((data & 0x8000) == 0);
    return true;
}

void ExternalADC::inVolts(uint16_t *data,double *data_in_volts)
{
    const double VPS = 4.096 / 32768.0; //volts per step
    *data_in_volts = (*data)*VPS;
}
