#include <iostream>
#include <stddef.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "Salt.h"
#include "Fpga.h"
#include "fastComm.h"

//#include <time.h> 
using namespace std;

class Dig_Clk_test {

 public:

  Dig_Clk_test(){};
  Dig_Clk_test(Fpga*, Salt*, FastComm*);
  ~Dig_Clk_test(){};

  //Salt *st = new Salt();

  void unmask_all();
  bool DAQ_Sync();
  //bool FPGA_DAQ_Sync();
  bool DAQ_Sync_v3_1();
  bool DAQ_Sync_v3_2();

  bool DAQ_Sync_v3();
  int Check_Ber(uint8_t *packet, int length);
  int Check_Ber(uint32_t * packet, int length, uint8_t pattern);
  int Check_Ber(uint32_t * packet, int length, uint8_t *pattern);
  void FPGA_PLL_shift(int16_t phase);
  void FPGA_PLL_shift_Deser(int16_t phase);
  void phase_find();
  bool DLL_Check();
  bool PLL_Check();
  bool DLL_Check_v3();
  bool PLL_Check_v3();
  bool I2C_check();
  bool TFC_Command_Check();
  bool TFC_DAQ_sync();
  bool TFC_DSR_sync();
  bool DAQ_Delay();
  void TFC_Reset();
  uint8_t randomPattern();
  uint8_t NextValue(uint8_t CVal);
	int Check_Seq();

int tot_fail;

 private:
  Fpga* fpga_;
  Salt* salt_;
  FastComm* fastComm_;
  int max_commands = 1000;
};
