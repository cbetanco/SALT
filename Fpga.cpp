//Fpga.cpp
#include "Fpga.h"

#include <cerrno>
#include <cstring>

Fpga::Fpga()
{
    this->access_fpga();
}


Fpga::~Fpga()
{
    this->close_fpga();
}

void Fpga::access_fpga()
{
    memFile = open( "/dev/mem", O_RDWR | O_SYNC );
    if( memFile < 0 )
    { 
        int errvsv = errno;
        printf("ERROR: I cannot open physical memory /dev/mem and return errno %s\n",strerror(errvsv));
        exit(1);
    }
  
      //lightweight HPS-to-FPGA bridge
    map_virtual_addr = mmap(0,HW_REGS_SPAN,PROT_READ | PROT_WRITE,MAP_SHARED,memFile,HW_REGS_BASE);
    DAQ_READ0_register_address = map_virtual_addr + ( ( uint32_t )( (unsigned long) (ALT_LWFPGASLVS_OFST + registers::DAQ_READ0 )) & ( uint32_t )( HW_REGS_MASK ) );
    if( map_virtual_addr == MAP_FAILED )
    {
        int errvsv = errno;
        printf("ERROR: I cannot map physical memory in the process virtual memory for lightweight HPS-to-FPGA and return errno %s\n",strerror(errvsv));
        close(memFile);
        exit(1);
    }
}   

void Fpga::find_reg_address(uint32_t register_address)
{
    map_register_address = map_virtual_addr + ( ( uint32_t )( (unsigned long) (ALT_LWFPGASLVS_OFST + register_address )) & ( uint32_t )( HW_REGS_MASK ) );
}
      
void Fpga::read(uint32_t* data)
{
    *data = *( ( uint32_t *) map_register_address );
    
}

void Fpga::read(uint64_t* data)
{
    *data = *( ( uint32_t *) map_register_address );
    
}

void Fpga::read(uint8_t* data)
{
    *data = *( ( uint32_t *) map_register_address );
} 

void Fpga::read_DAQ_READ0(uint32_t* data)
{
    *data = *( ( uint32_t *) DAQ_READ0_register_address );
}

void Fpga::read_fpga(uint32_t register_address,uint32_t* data)
{
    this->find_reg_address(register_address);
    //std::cout << "Reg address = " << std::hex << unsigned(register_address) << std::endl;
    this->read(data);
}

void Fpga::read_fpga(uint32_t register_address,uint64_t* data)
{
    this->find_reg_address(register_address);
    //std::cout << "Reg address = " << std::hex << unsigned(register_address) << std::endl;
    this->read(data);
}

void Fpga::read_fpga(uint32_t register_address,uint8_t* data)
{
    this->find_reg_address(register_address);
    this->read(data);
}

void Fpga::write(uint32_t data)
{
    *( ( uint32_t *) map_register_address ) = data;
}

void Fpga::write(uint8_t data)
{
    *( ( uint8_t *) map_register_address ) = data;
}

void Fpga::write(uint16_t data)
{
    *( ( uint8_t *) map_register_address ) = data;
}

void Fpga::write_fpga(uint32_t register_address,uint32_t data)
{
    this->find_reg_address(register_address);
    this->write(data);
}

void Fpga::write_fpga(uint32_t register_address,uint8_t data)
{
    this->find_reg_address(register_address);
    this->write(data);
}
void Fpga::write_fpga(uint32_t register_address,uint16_t data)
{
    this->find_reg_address(register_address);
    this->write(data);
}

void Fpga::close_fpga()
{
    if( munmap( map_virtual_addr, HW_REGS_SPAN ) )
    {
        int errvsv = errno;
        printf("ERROR:  I cannot unmap physical memory for lightweight HPS-to-FPGA and return errno %s\n",strerror(errvsv));
        close(memFile);
        exit(1);
    }
    close( memFile );

}
