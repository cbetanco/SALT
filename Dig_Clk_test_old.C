#include <time.h>
#include "Dig_Clk_test.h"
#include "Salt.h"
#include <unistd.h>
#include "Fpga.h"
#include "registers_config.h"
#include "fastComm.h"
#include <iostream>
#include <ctime>
#include <fstream>
#include <iomanip>

using namespace std;

// CONSTRUCTOR
Dig_Clk_test::Dig_Clk_test(Fpga *fpga, Salt *salt, FastComm *fastComm) {
   fpga_=fpga;
  salt_=salt;
  fastComm_=fastComm;  
}

// Synch output of DAQ to clock
bool Dig_Clk_test::DAQ_Sync() {

  uint32_t data[5120];
  const int length=100;
  int e[8][8] = {0};
  int bs_p[2];
  bool found_opt = false;
  bool tfc_trig = false; // Do not set tfc trigger for pattern readout
  
  // DAQ Reset & Clear FIFO
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x11);
  usleep(100);
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);

  // configure DAQ  
  fastComm_->config_daq(length, 0, tfc_trig);
  
  // Set correct pattern
  salt_->write_salt(registers::pattern_cfg, (uint8_t) 0xAB); // Set pattern for synch, in this case hAB
  salt_->write_salt(registers::ser_source_cfg, (uint8_t) 0x22); // Reset ser_source_cfg (count up, pattern register output)
salt_->write_salt(registers::elinks_cfg, (uint8_t) 0x00);

  // Loop over phases
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 8; j++) {
      for(int k=0; k<10; k++) {
	fastComm_->read_daq(length,data,tfc_trig);
	e[i][j]+=Check_Ber(data,length,0xAB);   
	if(k==9 && (e[i][j]==0)) {
	  bs_p[0] = i;
	  bs_p[1] = j;
	  found_opt = true;
	  break;
	}
      }
      if(found_opt) break;
      FPGA_PLL_shift(1);
    }
    if(found_opt) break;
    //bit slip
    fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
    usleep(100);
    fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
    usleep(100);
    FPGA_PLL_shift(-8);
  }
  if(!found_opt) cout << "CLK synch failed: Could not find optimal bit slip/phase" << endl;
  else cout << "CLK synch finished. Optimal values = " <<bs_p[0]  << ", " << bs_p[1] << endl; 
  return found_opt;
}

void Dig_Clk_test::FPGA_PLL_shift(int16_t phase) {
  if(phase>0)  fpga_->write_fpga(registers::PLL_DFS_3,(uint8_t) 0x21);
  else fpga_->write_fpga(registers::PLL_DFS_3,(uint8_t) 0x01); //UP
  fpga_->write_fpga(registers::PLL_DFS_1,(uint8_t) abs(phase));
  fpga_->write_fpga(0x00040008,(uint8_t) 0x01);
}

// check bit error
int Dig_Clk_test::Check_Ber(uint32_t *packet, int length, uint8_t pattern) {
  int error =0;
  uint32_t RepeatedPacket = pattern << 16 | pattern << 8 | pattern;
  for (int k=0; k<length; k++) {
	  //cout << "packet[k] = " << hex << (unsigned) packet[k] << endl;
	  //cout << "pattern = " << hex << (unsigned) pattern << endl;
	  //cout << "Reapeat = " << hex << (unsigned) RepeatedPacket << endl;
		if ((packet[k] & 0x00FFFFFF)!=RepeatedPacket)
		{
			//cout << "error" << endl;
			error++;
			break; //Disable if really interested in BER
		}
  }
  return error;
}

// DLL configuration as outlined in the SALT manual
bool Dig_Clk_test::DLL_Check() {
  uint8_t read=0;
  uint8_t vcdl_value;

  salt_->write_salt(registers::others_g_cfg,(uint8_t) 0x00);


  for (vcdl_value=0x60; vcdl_value < 0x7F; vcdl_value ++)  
  { //Only 16 phases to check!
  		salt_->write_salt(registers::dll_vcdl_cfg, vcdl_value);
  	usleep(1);
  	salt_->read_salt(registers::dll_vcdl_mon, &read);
  	//cout << "vcdl_value=" << hex << (int)vcdl_value << " dll_cur_OK="<< (bool)(read & 0x80) << endl;
  	if ((read & 0x80) == 0x00) break;
  }
  if (vcdl_value == 0x7F) //If previous failed we exit
  {
    cout << "ERROR: DLL_Check: Didn't manage to make dll_cur_OK go low"<< endl;
    return false;
  }
  for(vcdl_value--; vcdl_value >0; vcdl_value --)
  { //Only 127 values to check
  	salt_->write_salt(registers::dll_vcdl_cfg, vcdl_value);
  	usleep(1);
		salt_->read_salt(registers::dll_vcdl_mon, &read);
		//cout << "vcdl_value=" << hex << (int)vcdl_value << " dll_cur_OK="<< (bool)(read & 0x80) << endl;
		if ((read & 0x80) != 0x00) break;
  }
  if (vcdl_value == 0x00) //If previous failed we exit
  {
		cout << "ERROR: DLL_Check: Didn't manage to make dll_cur_OK go high"<< endl;
		return false;
  }
  salt_->write_salt(registers::others_g_cfg,(uint8_t) 0xC0);
  cout << "DLL calibration optimal Value: 0x" << hex << (int)vcdl_value << endl;
  return true;
}

// PLL configuration as outlined in SALT manual
bool Dig_Clk_test::PLL_Check() {
  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0x8C);
  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0xCC);
  cout << "PLL initialized" << endl;
  return true;
}

bool Dig_Clk_test::I2C_check() {
  uint8_t data = 0;
  // Configure PLL
  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0x8C );
  usleep(10);
  salt_->write_salt(registers::pll_vco_cfg, (uint8_t) 0x12 );
  usleep(10);
  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0xCC );
  usleep(10);
  salt_->write_salt(registers::ser_source_cfg, (uint8_t) 0x22 );

  for(int i=0; i<255; i++) {
    salt_->write_salt(registers::pattern_cfg, (uint8_t) i);
    salt_->read_salt(registers::pattern_cfg, &data);
    if(data!=i) return false;
  }
  //Leave it with the default value
  salt_->write_salt(registers::pattern_cfg, (uint8_t) 0xAB );
  return true;
}


// sync between TFC and chip
bool Dig_Clk_test::TFC_DAQ_sync() {
  uint8_t length = 3;
  uint8_t command[max_commands]={0};
  uint16_t length_read = 255; // number of clock cycles to read
  uint32_t data[5120]; // data packet
  int period = 3;
   // define single or continuous transmission
  bool singleShot = false;
  bool rightConfig = false;
  bool i_value_bad[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
  bool found_a_good_one = false;

  int e[256][32] = {0};
  // Set DAQ delay to 0
  fpga_->write_fpga(registers::DAQ_DELAY, (uint8_t) 0x00);
  // set DAQ to DSP out data
  salt_->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
  for(int k=1; k<=128; k*=2) {
    command[0]=k;
    command[1]=command[0];
    command[2]=command[0];
    cout << "k = " << dec << k << endl;
    //cout << "command = " << dec << (unsigned) command[0] << endl;
    // loop over pll_clk_cfg values
    for(int i=0; i<16; i++) {
      //if (i_value_bad[i]) {break;} //Avoid testing if no valid pll_clk_cfg was found
      if(rightConfig) break;     //Do not keep on testing once we've decided the good one

      salt_->write_salt(registers::pll_clk_cfg, (uint8_t) (16*i+((i+7)&0x0F))); //Always valid
      found_a_good_one = false;
      for(int j=0; j<32; j++) 
      
      {
	     // cout << e[i][j] << " ";
	      //cout << endl;
	if(e[i][j]==0) //If not already bad from previous iteration
	{
	  salt_->write_salt(registers::deser_cfg, (uint8_t) j);
	  fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
	  e[i][j]+=Check_Ber(data,length_read,command[0]);
	  //cout << "e[" << dec << i << "][" << dec << j << "] = " << dec << e[i][j] << endl;
	  if (e[i][j]==0) found_a_good_one = true;
	  if(k==128  && e[i][j]==0) 
	  {
	    cout << "pll_clk_cfg = " << hex << (unsigned) 16*i << endl;
	    cout << "deser_cfg = " << hex << (unsigned) j << endl;
	    rightConfig=true;
            salt_->write_salt(registers::pll_clk_cfg, (uint8_t) (16*i+((i+7)&0x0F))); //Always valid
	    salt_->write_salt(registers::deser_cfg, (uint8_t) j);
	    cout << "Error Table: [row-> pll_clk_cfg, column -> deser_cfg]"<< endl;
	    for (int x=0; x<16; x++)
              {
               for (int y=0; y<32; y++)
        	cout << e[x][y] << " ";
                cout << "\n";

		
	      }
	    break;
	  }
	//  e[i][j] = 0;
	}
      }
     //  cout << endl;
if (!found_a_good_one) i_value_bad[i]=true;
    }
  }
  fastComm_->reset_DAQ(); //Leave it clean for the next user, just in case
  return rightConfig;   // if cann't get right pll config, fail
}

bool Dig_Clk_test::DAQ_Delay() {
  uint8_t length = 3;
  // Define command list (BXID and Sync)
  uint8_t command[max_commands]={0};
  // Read out data packet
  uint8_t length_read = 100; // number of clock cycles to read
  uint32_t data[5120]; // data packet
  uint8_t data8=0;
  int period = 100;
  // define single continuous transmission
  bool singleShot = true;
  bool rightConfig = false;
  
  command[0]=0x00;
  command[1]=0xAB;
  command[2]=0x00;
  salt_->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
  usleep(1000);
  // for(int i=0; i<256; i++) {
  //  for(int k=0; k<10; k++) {
  salt_->write_salt(registers::tfc_fifo_cfg, (uint8_t) 20);
  fastComm_->config_daq(length_read, (uint8_t) 0, true);
  fastComm_->config_tfc(length, command, period, singleShot);
  fastComm_->write_tfc();
  fastComm_->read_daq(length_read,data,true);
  salt_->read_salt(registers::pll_clk_cfg, &data8);
  cout << "pll_clk_cfg = " << hex<< (unsigned) data8 << endl;
  salt_->read_salt(registers::deser_cfg, &data8);
  cout << "deser_cfg = " << hex << (unsigned) data8 << endl;
  if(Check_Ber(data,length_read,0xAB)==0) {
    cout << "DAQ_DELAY = " << dec<< (unsigned) 0 << endl;
    fpga_->write_fpga(registers::DAQ_DELAY, (uint8_t) 0);
    return true;
  }
  command[0]=0x80;
  command[1]=command[0];
  command[2]=command[0];
  for(int i=0; i<256; i++) {
    salt_->write_salt(registers::calib_fifo_cfg, (uint8_t) i);
  }
  return rightConfig;
}
  
uint8_t Dig_Clk_test::randomPattern() {

 uint8_t x;
  srand(time(NULL)); // seed random number generator

  x = rand() & 0xFF;
  x |= (rand() & 0xFF) << 8;
  x |= (rand() & 0xFF) << 16;
  x |= (rand() & 0xFF) << 24;

    return x;
  
}

void Dig_Clk_test::unmask_all() {

  salt_->write_salt(registers::mask0_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask1_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask2_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask3_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask4_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask5_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask6_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask7_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask8_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask9_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask10_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask11_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask12_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask13_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask14_cfg,(uint8_t) 0x00);
  salt_->write_salt(registers::mask15_cfg,(uint8_t) 0x00);
}

bool Dig_Clk_test::TFC_Command_Check() {
  uint8_t command[max_commands]={0};
  uint8_t length = 90;
  uint16_t length_read = 500;
  string data_string;
  uint32_t data[5120];
  bool singleShot = true;
  int period = length;
  int flag = 0;
  int length1 = 0;
  int bxid = 0;
  int parity = 0;
  uint8_t buffer;
  unsigned twelveBits[10240];
  int counter = 0;
  unmask_all();

  TFC_Reset();
  // DSP output
  salt_->write_salt(registers::ser_source_cfg,(uint8_t) 0x20);

  //salt_->read_salt(registers::pack_adc_sync_cfg, &buffer);
  //cout << "BUFFER = " << hex << (unsigned) buffer << endl;
  // set synch pattern registers
  salt_->write_salt(registers::sync1_cfg,(uint8_t) 0x8C);
  salt_->write_salt(registers::sync0_cfg,(uint8_t) 0xAB);

  salt_->write_salt(registers::idle_cfg, (uint8_t) 4);
  // fereset counter for snapshot command check
  uint8_t counter_fere_snap = 0;
 
  // check TFC commands
  string option[8];
  bool pass[8] = {false};
  option[0] = "Normal";
  option[2] = "BXReset";
  option[1] = "Header";
  option[3] = "NZS";
  option[4] = "BxVeto";
  option[5] = "Synch";
  option[7] = "Snapshot";
  option[6] = "FEReset";
  
  for(int i=0; i<79; i++)
    command[i]=0x04;
  
  for(int i=0; i < 8; i++) {
	  //cout << endl;
    if(option[i] == "Normal")        command[79] = 0x00;
    else if(option[i] == "BXReset")  command[79] = 0x01;
    else if(option[i] == "FEReset")  command[79] = 0x02;
    else if(option[i] == "Header")   (command[79] = 0x04, command[80] = 0x04, command[81] = 0x04);
    else if(option[i] == "NZS")      command[79] = 0x08;
    else if(option[i] == "BxVeto")   command[79] = 0x10;
    else if(option[i] == "Snapshot") (command[79] = 0x08, command[80] = 0x08, command[81] = 0x20);
    else if(option[i] == "Synch")    {command[79] = 0x40; for(int k=80;k<89;k+=2) {command[k] = 0x40, command[k+1] = 0x10;};};

TFC_Reset(); 
    //cout << "option[" << i << "] = " << option[i] << endl;
    //cout << "command[79]" << hex << (unsigned) command[79] << endl;

    fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, true );

    for(int j=0; j< length_read; j++) {
     

         twelveBits[2*j] = fastComm_->read_twelveBits(data[j], 0);
	 twelveBits[2*j+1] = fastComm_->read_twelveBits(data[j], 1);
    	//cout << "option[" << i << "] = " << option[i] << endl;
	//cout << hex << setfill('0') << setw(3) << twelveBits[2*j] << " " << setw(3) << twelveBits[2*j+1];
    }
//cout << endl;
    for(int j=0; j<2*length_read; j++) {
    // cout << hex <<twelveBits[j] << " ";
      if(twelveBits[j] == 0x0F0) { continue;}
      fastComm_->read_Header(twelveBits[j], bxid, parity, flag, length1);
      
      if(flag == 0) {
	if(option[i] == "Normal") pass[i] = true;
	if(option[i] == "BXReset" && bxid == 0) pass[i] = true; 
	
      }
      if(flag == 1) {
//cout << "option is " << option[i] << endl;
//if(option[i] == "BxVeto")

//{cout << "twelve bits = " << hex << (unsigned)twelveBits[j] << endl;
//	      cout << "length = " << hex << (unsigned) length1 << endl;
//}

	if(((length1 == 0x11) || (length1 == 0x21)) && option[i] == "BxVeto") pass[i] = true;
	if(length1 == 0x12 && option[i] == "Header") pass[i] = true;
	if(length1 == 0x06 && option[i] == "NZS") pass[i] = true;
	
      }
      if(option[i] == "FEReset") {
 	
	salt_->read_salt(registers::fereset_cnt0_snap_reg, &counter_fere_snap);


	salt_->read_salt(registers::calib_cnt0_reg, &buffer);
	if(buffer == 0x00) pass[i] = true;
	
      }
      if(option[i] == "Synch") {
       
	//salt_->read_salt(registers::sync1_cfg, &buffer);
	salt_->read_salt(registers::sync0_cfg, &buffer);
	
	//twelveBits[j] = fastComm_->read_twelveBits(data_string, j+3);
	

	if((buffer & 0xFF) == (twelveBits[j] & 0xFF))
	  pass[i] = true;
	
      }
      if(option[i] == "Snapshot") {

	salt_->read_salt(registers::nzs_cnt0_snap_reg, &buffer);
	//salt_->read_salt(registers::bxid_cnt0_sn
//cout << hex << (unsigned) buffer << endl;	
	if((buffer) == 2)
	  pass[i] = true;
      }
    }

     //cout << endl; 
    if(!pass[i]) {
      cout << "ERROR::TFC " << option[i] << " fails" << endl;
      counter++;
    }
    else cout << option[i] << " passed" << endl;
    
  }
  
  if(counter>0) return false;
  
  return true;
  
}

void Dig_Clk_test::TFC_Reset() {

  // Reset TFC state machine and set all related registers to default values
  fpga_->write_fpga(registers::TFC_CFG,(uint8_t) 0x01);
  usleep(1000);
  fpga_->write_fpga(registers::TFC_CFG,(uint8_t) 0x00);
  
}
