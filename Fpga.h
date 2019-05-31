//Fpga.h
#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>

#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"

#include <cerrno>
#include <cstring>

#include "registers_config.h"

#define HW_REGS_BASE ( 0xfc000000) //ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 ) //64 MB with 32 bit adress space this is 256 MB
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )


class Fpga {
    public:
        Fpga();
        virtual ~Fpga();
     
        void access_fpga();
        void read_fpga(uint32_t, uint32_t*);
	void read_fpga(uint32_t, uint64_t*);
        void write_fpga(uint32_t, uint32_t);
	void write_fpga(uint32_t, uint16_t);
	void read_fpga(uint32_t,uint8_t*);
        void write_fpga(uint32_t, uint8_t);

	void read_DAQ_READ0(uint32_t* data);
	
        void close_fpga();
    
    private:
        void find_reg_address(uint32_t);
        void read(uint32_t*);
	void read(uint64_t*);
        void write(uint32_t);
	void write(uint16_t);

	void read(uint8_t*);
	//		void read(uint32_t*);
        void write(uint8_t);
        
        void *map_virtual_addr;
        void *map_register_address;
	void *DAQ_READ0_register_address;
        
        int memFile;
    
};
