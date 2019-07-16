#include "ExternalADC.h"
#include "Salt.h"
#include "registers_config.h"
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream, std::stringbuf

Salt* saltChipHandle2; //Handle for the ASIC
void dacCalib_Calibrate(Salt *);
void dacCalib_PerformCalib(ExternalADC *, uint16_t, uint16_t, uint8_t, float *Outcome);
