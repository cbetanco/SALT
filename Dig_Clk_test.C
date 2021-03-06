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
#include <string.h>

using namespace std;

// CONSTRUCTOR
Dig_Clk_test::Dig_Clk_test(Fpga *fpga, Salt *salt, FastComm *fastComm) {
  fpga_=fpga;
  salt_=salt;
  fastComm_=fastComm;  
}
/*
// Sync prcedure as outlines by carlos
bool Dig_Clk_test::FPGA_DAQ_Sync() {

 uint32_t data[5120];
  const int length=100;
  int e[16][16] = {0};
  int edeep[16][16]={0};
  int s[16][16] = {0}; //One sample from each...
  int bs_p[2];
  bool found_opt = false;
  bool tfc_trig = false; // Do not set tfc trigger for pattern readout
  int pattern = 0x0F;
  
    
  // DAQ Reset & Clear FIFO
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x11);
  usleep(100);
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
  
  // configure DAQ  
  fastComm_->config_daq(length, 0, tfc_trig);
  
  // Set correct pattern
  salt_->write_salt(registers::pattern_cfg, (uint8_t) pattern); // Set pattern for synch, in this case hAB
  salt_->write_salt(registers::ser_source_cfg, (uint8_t) 0x22); // Reset ser_source_cfg (count up, pattern register output)
  salt_->write_salt(registers::elinks_cfg, (uint8_t) 0x00);
}
*/
int Dig_Clk_test::Check_Seq() {
	int error =0;
	uint32_t data[5120];
	const int length = 100;
  uint8_t pattern[256];
	uint8_t value = 0;
	uint32_t mask = 0xFFFFFF; //In case we want to mask data
  salt_->write_salt(registers::ser_source_cfg, (uint8_t) 0x03);

	for(int j = 0; j < 100; j++) //Many PRBS runs
	{
		fastComm_->read_daq(length,data,false);
		value = data[0] & mask;
		pattern[0] = value;
		for(int i = 1; i < length; i++) //Loop over samples of a given run
		{
			data[i-1] &= mask;
			pattern[i]= NextValue(value);
			uint32_t repPattern= (value | value << 8 | value << 16);
			repPattern &= mask;
			if (data[i-1]!=repPattern)
			{
				//cout << "Check_Seq: RunN:"<< dec<< j <<" Sample:"<< dec << i << " Expected: 0x" << hex << repPattern << " but got: 0x" << hex << data[i-1] << endl;
				error++;
				break;
			}
			value = pattern[i];
		}
		if (error !=0) break;
	}
	return error;
}

uint8_t Dig_Clk_test::NextValue(uint8_t CVal) {
	bool ornot = 0;
	for(int mask = 1; mask != 0x80; mask <<=1)
		ornot |= (CVal & mask);
	ornot = not(ornot);
	bool xorornot = ((CVal & 0x80) >> 7)^ornot;
	bool feedback = xorornot ^ ((CVal & 0x08) >> 3) ^ ((CVal & 0x10) >> 4) ^ ((CVal & 0x20) >> 5);
	uint8_t NValue = (CVal << 1) | feedback;
	return NValue;
}

// Synch output of DAQ to clock
bool Dig_Clk_test::DAQ_Sync() {
  
  uint32_t data[5120];
  const int length=100;
  int e[16][16] = {0};
  int edeep[16][16]={0};
  int s[16][16] = {0}; //One sample from each...
  int bs_p[2];
  bool found_opt = false;
  bool tfc_trig = false; // Do not set tfc trigger for pattern readout
  int pattern = 0x0F;
  
  // DAQ Reset & Clear FIFO
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x11);
  usleep(100);
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
  
  // configure DAQ  
  fastComm_->config_daq(length, 0, tfc_trig);
  
  // Set correct pattern
  salt_->write_salt(registers::pattern_cfg, (uint8_t) pattern); // Set pattern for synch, in this case hAB
  salt_->write_salt(registers::ser_source_cfg, (uint8_t) 0x22); // Reset ser_source_cfg (count up, pattern register output)
  salt_->write_salt(registers::elinks_cfg, (uint8_t) 0x00);
  
  // Loop over phases
  cout << "Preliminary BER table (5 iters), rows -> bit slip. Cols -> FPGA_PLL phase" << endl;
  cout << "     0     1     2     3     4     5     6     7" << endl;
  //       X: 12345 12345 12345 12345 12345 12345 12345 12345
  for(int i = 0; i < 8; i++) {
  cout << i << ": ";
    for(int j = 0; j < 16; j++) {
      for(int k=0; k<5; k++) {
		fastComm_->read_daq(length,data,tfc_trig);
		e[i][j]+=Check_Ber(data,length,pattern);   
	}
	if(e[i][j]==0){
        for(int l=0; l<1000; l++) {
		fastComm_->read_daq(length,data,tfc_trig);
	  	edeep[i][j]+=Check_Ber(data,length,pattern);   
   		}
      }
      s[i][j]=data[0];
      cout << setfill('0') << setw(5) << dec << e[i][j] << " ";
      FPGA_PLL_shift(1);
    }
    cout << endl;
    //bit slip
    fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
    fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
    FPGA_PLL_shift(-7); //Leave it where it was
  }
  //Leave the bit slip where it initially was...
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);

  //We have a preliminary table, now we try deeper in the interesting ones
  // Loop over phases
  cout << "In-depth BER table (1000 iters), rows -> bit slip. Cols -> FPGA_PLL phase" << endl;
  cout << "     0     1     2     3     4     5     6     7" << endl;
  //       X: 12345 12345 12345 12345 12345 12345 12345 12345
  for(int i = 0; i < 8; i++) {
  cout << i << ": ";
    for(int j = 0; j < 16; j++) {
      if(e[i][j]==0){
        cout << setfill('0') << setw(5) << dec << edeep[i][j] << " ";
      } else cout << "----- ";
    }
    cout << endl;
    //bit slip
  }  
 //With the definitive table we chose the first one that works
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 16; j++) {
      if(e[i][j]==0){
	  bs_p[0] = i;   bs_p[1] = j;   found_opt = true;
	  break;
      } else FPGA_PLL_shift(1);
    }
    if (!found_opt) {
	    //bit slip
	    fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
	    fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
	    FPGA_PLL_shift(-7); //Leave it where it was
    }
  }

  if(!found_opt){  //Leave the bit slip where it initially was...
     fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
     fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
     cout << "CLK synch failed: Could not find optimal bit slip/phase" << endl;
     cout << "Example of the data collected, rows -> bit slip. Cols -> FPGA_PLL phase" << endl;
     cout << "     0      1      2      3      4      5      6      7" << endl;
     //       X: 123456 123456 123456 123456 123456 123456 123456 12345
     for(int i = 0; i < 8; i++) {
       cout << i << ": ";
       for(int j = 0; j < 16; j++) cout << setfill('0') << setw(6) << hex << (0xFFFFFF & s[i][j]) << " ";
       cout << endl;
     }
  } else cout << "CLK synch finished. Optimal values = " <<bs_p[0]  << ", " << bs_p[1] << endl; 
  
  return found_opt;
}

bool Dig_Clk_test::DAQ_Sync_v3_1() {
  
  uint32_t data[5120];
  const int length=100;
  int e[8][8] = {0};
  int bs_p[2];
  bool found_opt = false;
  bool tfc_trig = false; // Do not set tfc trigger for pattern readout
  uint8_t pattern = 0xAB;
  int error = 0;
  uint8_t read;

  // DAQ Reset & Clear FIFO
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x11);
  usleep(100);
  fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
  
  // configure DAQ  
  fastComm_->config_daq(length, 0, tfc_trig);
  
  // Set correct pattern
 salt_->write_salt(registers::pattern_cfg, pattern); // Set pattern for synch, in this case hAB
  salt_->write_salt(registers::ser_source_cfg, (uint8_t) 0x22); // Reset ser_source_cfg (count up, pattern register output)
  salt_->write_salt(registers::elinks_cfg, (uint8_t) 0x00);
   //uint8_t buffer;
  //salt_->read_salt(registers::pattern_cfg,&buffer);
  //cout << "buffer is = " << (unsigned) buffer << endl;
  salt_->read_salt(registers::ser_g_cfg, &read);
cout << "read bit slip is " << hex << (unsigned) read << endl;
  //return true; 

  // Loop over phases
  for(int j = 0; j < 4; j++) {
	error = 0;
	// check that e-links read same value 1000 times
	for(int m = 0; m < 1000; m++) {
		uint8_t e0, e1, e2;
		fastComm_->read_daq(length,data,tfc_trig);
		for(int n =0; n < length; n++) {
		e0 = data[n] & 0x000000FF;
		e1 = (data[n] & 0x0000FF00) >> 8;
		e2 = (data[n] & 0x00FF0000) >> 16;
		
		if((e0!=e1) && (e0!=e2)) {
		cout << hex << (unsigned) e2 << (unsigned) e1 << (unsigned) e0 << endl;
			error++;
			//cout << "error = " << dec << error << endl;
		}
//cout << "error = " << error<< endl;
		}
 	}
	//return true;
	if(error ==0) {
		cout << "here" << endl;
		//return true;
		for(int i = 0; i < 8; i++) {
    			salt_->write_salt(registers::ser_g_cfg, (uint8_t) i);
			for(int k=0; k<1000; k++) {
				//if(!Check_Seq()) e[i][j]++;
				fastComm_->read_daq(length,data,tfc_trig);
				e[i][j]+=Check_Ber(data,length,pattern);
				//cout << "e[" << dec << i << "][" << j<<"] = " << e[i][j] << endl;
				//return true;
				if(k==999 && (e[i][j]==0)) {
	  			bs_p[0] = i;
	  			bs_p[1] = j;
				cout << "bs_p[0] = " << bs_p[0] << endl;
				cout << "bs_p[1] = " << bs_p[1] << endl;
	  			found_opt = true;
				//return true;
	  			break;
				}
			}
			if(found_opt) break;

		}
		
	}

	if(found_opt) break;
 	FPGA_PLL_shift(1);

  }
   if(!found_opt) cout << "CLK synch failed: Could not find optimal bit slip/phase" << endl;
  else cout << "CLK synch finished. Optimal values = " <<bs_p[0]  << ", " << bs_p[1] << endl; 
  return found_opt;
}

// Synch output of DAQ to clock
bool Dig_Clk_test::DAQ_Sync_v3() {
  
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
 salt_->write_salt(registers::pattern_cfg, (uint8_t) 0x0F); // Set pattern for synch, in this case hAB
  salt_->write_salt(registers::ser_source_cfg, (uint8_t) 0x22); // Reset ser_source_cfg (count up, pattern register output)
  salt_->write_salt(registers::elinks_cfg, (uint8_t) 0x00); 
  //uint8_t buffer;
  //salt_->read_salt(registers::pattern_cfg,&buffer);
  //cout << "buffer is = " << (unsigned) buffer << endl;
  
  // Loop over phases
  for(int i = 0; i < 8; i++) {
    salt_->write_salt(registers::ser_g_cfg, (uint8_t) i);

    for(int j = 0; j < 4; j++) {
      for(int k=0; k<1000; k++) {
	fastComm_->read_daq(length,data,tfc_trig);
	e[i][j]+=Check_Ber(data,length,0x0F);
	if(k==999 && (e[i][j]==0)) {
	  bs_p[0] = i;
	  bs_p[1] = j;
	  found_opt = true;
	  break;
	}
	//cout << "e[i][j] = " << dec << e[i][j] << endl;
      }
      if(found_opt) break;
      FPGA_PLL_shift(1);
    }
    if(found_opt) break;
    //bit slip

    //fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
    //usleep(100);
    //fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
    //usleep(100);
    // salt_->write(registers::ser_g_cfg, (unsigned) i+1);
    FPGA_PLL_shift(-7);
  }

  /*int err = 0;
if(found_opt) {
for(int i = 0; i < 1000; i++) {
    
    uint8_t k = randomPattern();
    //command[0]=k;
    //command[1]=command[0];
    //command[2]=command[0];
    //fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
    //if(i==0) continue;
fastComm_->read_daq(length,data,tfc_trig);

    if(Check_Ber(data,length,k) != 0) {
      
      found_opt = false;
      cout << "ERROR::Configuration not stable" << endl;
      cout << "Failed after " << i << " iterations" << endl;
      //cout << "pattern = " << hex << (unsigned) k << endl;
      //cout << "data = " << hex << (unsigned) data[i] << endl;
      break;
      
    }
  }


}
*/
  if(!found_opt) cout << "CLK synch failed: Could not find optimal bit slip/phase" << endl;
  else cout << "CLK synch finished. Optimal values = " <<bs_p[0]  << ", " << bs_p[1] << endl; 
  return found_opt;
}
void Dig_Clk_test::phase_find() {


	clock_t start;
	clock_t finish;
	bool pass = false;
	start = clock();
	finish = clock();
//salt_->write_salt(registers::n_zs_cfg, (uint8_t) 32);
//salt_->write_salt(registers::ana_g_cfg, (uint8_t) 0x92);
//salt_->write_salt(registers::baseline_g_cfg, (uint8_t) 255);


	while(!pass && ((float) (finish/CLOCKS_PER_SEC))<10) {
		if(DLL_Check_v3() && PLL_Check_v3() && DAQ_Sync_v3_2() && TFC_DSR_sync()) {
			TFC_Command_Check();
			//if(tot_fail <=1) {
				cout << "tot fail " << tot_fail << endl;
			pass = true;
			cout << "PASSED PART 1" << endl;
			//}
		}
		else FPGA_PLL_shift_Deser(1);

		finish = start - clock();
	}
	if(tot_fail<=1) pass = true;
	else pass = false;
	start = clock();
	while(!pass&& ((float) (finish/CLOCKS_PER_SEC))<10){
		cout << "IN PART 2" << endl;
		if(DLL_Check_v3() && PLL_Check_v3() && DAQ_Sync_v3_2() && TFC_DSR_sync())
		{
			TFC_Command_Check();
			if(tot_fail <=1) 
				pass = true;
			
		}

		if(pass) break;
			FPGA_PLL_shift_Deser(0);
			usleep(100);
	finish = clock();	
	}
//salt_->write_salt(registers::n_zs_cfg, (uint8_t) 0x20);
//salt_->write_salt(registers::ana_g_cfg, (uint8_t) 0x04);
//salt_->write_salt(registers::baseline_g_cfg, (uint8_t) 255);

}

bool Dig_Clk_test::DAQ_Sync_v3_2() {

	bool found=false;
	cout << "Testing DAQ synchronization" << endl;
	for (int phase=0; phase < 64; phase++){
		int errors=0;
		for (int bitslip=0; bitslip<16; bitslip++){
		
			errors = Check_Seq();
       			if(errors ==0) break;
			else{
        			fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
				fpga_->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
				usleep(100);
			}
		}
		if(errors==0) {

		found = true;
		break;

		}
		else {
			FPGA_PLL_shift(1);
			usleep(100);
		}
	}
	return found;
}

void Dig_Clk_test::FPGA_PLL_shift_Deser(int16_t phase) {
if(phase == 0) 
  fpga_->write_fpga(0x00040018,(uint32_t) 0x00250001);
else fpga_->write_fpga(0x00040018,(uint32_t) 0x00240001);

  fpga_->write_fpga(0x00040008,(uint8_t) 0x01);  
}

void Dig_Clk_test::FPGA_PLL_shift(int16_t phase) {
/*
  if(phase>0)  fpga_->write_fpga(registers::PLL_DFS_3,(uint8_t) 0x21);
  else fpga_->write_fpga(registers::PLL_DFS_3,(uint8_t) 0x01); //UP
  fpga_->write_fpga(registers::PLL_DFS_1,(uint8_t) abs(phase));
  fpga_->write_fpga(0x00040008,(uint8_t) 0x01);
  */

  fpga_->write_fpga(0x00040018,(uint32_t) 0x00210001);
  fpga_->write_fpga(0x00040008,(uint8_t) 0x01);  
}
int Dig_Clk_test::Check_Ber(uint32_t *packet, int length, uint8_t *pattern) {
  int error =0;
  int mask=0xFFFFFF;
//  uint8_t read;
  uint32_t RepeatedPacket;// = pattern << 16 | pattern << 8 | pattern;
 // RepeatedPacket &= mask;
  for (int k=0; k<length; k+=3) {
	RepeatedPacket = pattern[k] << 16 | pattern[k+1] << 8 | pattern[k+2];
  	//RepeatedPacket &= mask;

    //salt_->read_salt(registers::pattern_cfg,&read);
  // 	cout << "packet[k] = " << hex << (unsigned) packet[k] << endl;
 //  	cout << "packet[k+1] = " << hex << (unsigned) packet[k+1] << endl;
//	cout << "packet[k+2] = " << hex << (unsigned) packet[k+2] << endl;

    //cout << "pattern = " << hex << (unsigned) pattern << endl;
    //cout << "read back pattern (i2c) = " << hex << (unsigned) read << endl;
  //cout << "Reapeat = " << hex << (unsigned) RepeatedPacket << endl;
    if ((packet[k] & mask)!=RepeatedPacket )
      {
	// cout << "packet[k] = " << hex << (unsigned) packet[k] << endl;
	//cout << "repeat = " << (unsigned) RepeatedPacket << endl;
	//cout << "error" << endl;
	error++;
	//break; //Disable if really interested in BER
      }
  }
  return error;
}

// check bit error
int Dig_Clk_test::Check_Ber(uint32_t *packet, int length, uint8_t pattern) {
  int error =0;
  int mask=0x00FFFFFF;
//  uint8_t read;
  uint32_t RepeatedPacket = pattern << 16 | pattern << 8 | pattern;
  RepeatedPacket &= mask;
  for (int k=0; k<length; k++) {
    //salt_->read_salt(registers::pattern_cfg,&read);
    //cout << "packet[k] = " << hex << (unsigned) packet[k] << endl;
    //cout << "pattern = " << hex << (unsigned) pattern << endl;
    //cout << "read back pattern (i2c) = " << hex << (unsigned) read << endl;
    //cout << "Reapeat = " << hex << (unsigned) RepeatedPacket << endl;
    if ((packet[k] & mask)!=RepeatedPacket )
      {
	 //cout << "packet[k] = " << hex << (unsigned) packet[k] << endl;
	//salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0x8D);
//cout << "repeat = " << (unsigned) RepeatedPacket << endl;
	//cout << "error" << endl;
	error++;
	//break; //Disable if really interested in BER
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

// DLL configuration as outlined in the SALT manual
bool Dig_Clk_test::DLL_Check_v3() {
  uint8_t read=0;
  uint8_t vcdl_value;
bool pass = false;
  salt_->write_salt(registers::others_g_cfg,(uint8_t) 0x00);
  //salt_->write_salt(registers::dll_vcdl_cfg, 0xF3);

//for (vcdl_value=0xF3; vcdl_value > 0; vcdl_value--)  { 
  for (vcdl_value=127; vcdl_value > 0; vcdl_value--)  { //Only 16 phases to check!
      salt_->write_salt(registers::dll_vcdl_cfg, vcdl_value);
      usleep(1);
      salt_->read_salt(registers::dll_vcdl_mon, &read);
      //cout << "vcdl_value=" << hex << (int)vcdl_value << " dll_cur_OK="<< (bool)(read & 0x80) << endl;
      if ((read & 0x80) == 0x80) {
		pass = true;
	      break;
      }
      usleep(1);
  }
  
  salt_->write_salt(registers::others_g_cfg,(uint8_t) 0x40);
  usleep(1000);
  salt_->write_salt(registers::others_g_cfg,(uint8_t) 0x60);
  cout << "DLL calibration optimal Value: 0x" << hex << (int)vcdl_value << endl;
  return pass;
}


// PLL configuration as outlined in SALT manual
bool Dig_Clk_test::PLL_Check() {
  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0x8C);
  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0xCC);
  cout << "PLL initialized" << endl;
  return true;
}

bool Dig_Clk_test::PLL_Check_v3() {
bool pass = true;
 	salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0x8D);
  //	salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0xCD);
  	for(int m = 0; m < 3; m++) { 
 	salt_->write_salt(registers::mon_cfg, (uint8_t) m);

	uint8_t read;

	for(int i = 0; i < 1000; i++) {
		usleep(1);
		salt_->read_salt(registers::pll_vco_mon,&read); // read pll_vco_mon
		if( read !=0) { // if pll_vco_mon is <32 and not 0 then assume positive and increment pll_vco_cfg by one
			salt_->read_salt(registers::pll_vco_cfg,&read);
			//cout << "pll_vco_cfg = " << hex << (unsigned) read << endl;
			salt_->write_salt(registers::pll_vco_cfg, (uint8_t)(read + 1) );
			salt_->read_salt(registers::pll_vco_cfg,&read);

			//cout << "SUCCESS!" << endl << "PASSED!" << endl;
//out << "pll_vco_cfg + 1 = " << hex << (unsigned) read << endl;

		}
		else if(read >=32){ // if pll_vco_mon >32 then assume negative value and decrease pll_vco_cfg by one
			salt_->read_salt(registers::pll_vco_cfg,&read);
			//cout << "pll_vco_cfg = " << hex << (unsigned) read << endl;
			salt_->write_salt(registers::pll_vco_cfg, (uint8_t)(read - 1) );
			salt_->read_salt(registers::pll_vco_cfg,&read);

			//cout << "pll_vco_cfg - 1 = " << hex << (unsigned) read << endl;

		}
		else {
			
			break; // else break the loop and check pll_vco_mon (should be 0) 
		}
	}
	salt_->read_salt(registers::pll_vco_mon,&read);
if(abs(read) > 2) pass = false;
	cout << "pll_vco_mon = " << hex << (unsigned) read << endl;
	salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0xCD);

    cout << "PLL initialized" << endl;
    //if(read != 0
	}
  return pass;
}

bool Dig_Clk_test::I2C_check() {
  uint8_t data = 0;
  // Configure PLL
  //salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0x8C );
  //usleep(10);
  //salt_->write_salt(registers::pll_vco_cfg, (uint8_t) 0x12 );
  //usleep(10);
  //salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0xCC );
  

  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0x80);
  salt_->write_salt(registers::pll_main_cfg, (uint8_t) 0xC0);

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

bool Dig_Clk_test::TFC_DSR_sync() {
  uint8_t length = 3;
  uint8_t command[max_commands]={0};
  uint16_t length_read = 100; // number of clock cycles to read
  uint32_t data[5120]; // data packet
  int period = 3;
  // define single or continuous transmission
  bool singleShot = false;
  bool rightConfig = true;
  int best_j=0, best_i1=0, best_i2=0, sum1=100, sum2=100, sum=100;
  int e[256][32] = {0};
  int s1[32] = {0};
  int s2[32] = {0};
  // Set DAQ delay to 0
  fpga_->write_fpga(registers::DAQ_DELAY, (uint8_t) 0x00);
  // set DAQ to DSP out data
  salt_->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
  for(int k=1; k<=128; k+=5) {
	  // loop over pattern k
	  //k = 0xAA;
    command[0]=k; command[1]=command[0]; command[2]=command[0];

    for(int j=0; j<32; j++) { // loop over deser_cfg
      salt_->write_salt(registers::deser_cfg, (uint8_t) j);
      
      if((k&0xAA) != 0) { //If it contains interesting data for this case
	for(int i = 0; i < 16; i++) { // loop over pll_clk_sel[0]
	  salt_->write_salt(registers::pll_clk_cfg, (uint8_t) i);
	  if(e[i][j]==0) {
	    fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
	    for(int m=0; m < 260; m++){ //260 because we use 255, but we are safe...
	      data[m] &= 0xAAAAAAAA;
	    }
	    e[i][j]+=Check_Ber(data,length_read,(command[0] & 0xAA));
	  } 
	}
      }
      if((k&0x55)!=0) {
	for(int i = 0; i < 16; i++) {
	  salt_->write_salt(registers::pll_clk_cfg, (uint8_t) (i << 4)); //Always valid
	  if(e[i+16][j]==0) {
	    fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
	    for(int m=0; m < 260; m++) { //260 because we use 255, but we are safe...
	      data[m] &= 0x55555555;
	    }
	    e[i+16][j]+=Check_Ber(data,length_read,(command[0] & 0x55));
	  }
	}
      }	
    }
  }
  for(int j = 0; j < 32; j++) { //Loop over rows
    for(int i = 0; i < 32; i++) { //Over columns
      if(i<16) {
	      if(e[i][j]!=0)
				s1[j]++;
      }	      
      else if(e[i][j]!=0)
		s2[j]++;      
    }
    if( (s1[j] <16) && (s2[j] <16) ) { // If there is at least one zero in every zone
      if( (s1[j] + s2[j]) < sum) { //If we got a better solution than the existing
	sum = s1[j] + s2[j];
	best_j = j;
      }
    }   
  }
  sum = 100;
  for(int i = 0; i < 16; i++) {
    if(e[i][best_j] != 0)
      continue;
    if(i == 0)
      sum1=10;// = e[1][best_j] + e[2][best_j];
    else if(i == 15)
      sum1 = e[13][best_j] + e[14][best_j];
    else if(i==1)
      sum1 = e[0][best_j] + e[2][best_j] + e[3][best_j];
    else if(i==14)
      sum1 = e[12][best_j] + e[13][best_j] + e[15][best_j]; 
    else
      sum1 = e[i-1][best_j] + e[i+1][best_j] + e[i-2][best_j] + e[i+2][best_j];
    if(sum1 < sum ) {
      sum = sum1;
      best_i1 = i;
    }
  }
  sum = 100;
   cout << "best sel1 (as it goes)= ";
  for(int i = 16; i < 31; i++) {
    if(e[i][best_j] != 0)
      continue;
    if(i == 16)
      sum2=10;//sum2 = e[17][best_j] + e[18][best_j]; 
    else if(i == 31)
      sum2 = e[29][best_j] + e[30][best_j];
    else if(i==17)
      sum2 = e[16][best_j] + e[18][best_j] + e[19][best_j];
    else if(i==30)
      sum2 = e[28][best_j] + e[29][best_j] + e[31][best_j]; 
    else
      sum2 = e[i-1][best_j] + e[i+1][best_j]+ e[i-2][best_j] + e[i+2][best_j];
    if(sum2 < sum ) {
      sum = sum2;
      best_i2 = i;
      cout << dec<< best_i2 << " ";
    }
  }
  cout << endl;
  cout << "best deser_cfg = " << best_j << endl;
  cout << "best sel0 = " << best_i1 << endl;
  cout << "best sel1 = " << dec << best_i2 << endl;
  cout << "columns sel0,1, rows deser_cfg" << endl;

  for(int j = 0; j < 32; j++){
    cout <<  setfill('0') << setw(2) << j << ". ";
    for(int i = 0; i < 32; i++){
      if (i == 16) cout << "| " ;
      if(e[i][j]!=0) cout << 1 << " ";
  	else cout << 0 << " ";
      }
    cout << endl;
  }

  cout <<  "pll_clk_cfg = " << hex << (best_i1 + ((best_i2-16) << 4)) << endl;
 // uint8_t pll_clk_cfg = best_i1 | ((best_i2-16) << 4);
 // salt_->write_salt(registers::pll_clk_cfg, pll_clk_cfg);
 salt_->write_salt(registers::pll_clk_cfg, (uint8_t) (best_i1 + ((best_i2-16) << 4)));
  salt_->write_salt(registers::deser_cfg, (uint8_t) (best_j));

  uint8_t test;
  salt_->read_salt(registers::pll_clk_cfg, &test);
  cout << "pll_clk_cfg_r = " << (unsigned) test << endl;
salt_->read_salt(registers::deser_cfg, &test);

  cout << "deser_cfg_r = " << dec << (unsigned) test << endl;

  for(int i = 0; i < 1000; i++) {
    
    uint8_t k = randomPattern();
    command[0]=k;
    command[1]=command[0];
    command[2]=command[0];
    fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
    //if(i==0) continue;
    if(Check_Ber(data,length_read,command[0]) != 0) {
      
      rightConfig = false;
      cout << "ERROR::Configuration not stable" << endl;
      cout << "Failed after " << i << " iterations" << endl;
      cout << "pattern = " << hex << (unsigned) k << endl;
      cout << "data = " << hex << (unsigned) data[i] << endl;
      break;
      
    }
  }

  return rightConfig;
  
}

/*
// sync between TFC and chip
bool Dig_Clk_test::TFC_DAQ_sync() {
  int xmax = 15, ymax = 31;
  uint8_t length = 3;
  uint8_t command[max_commands]={0};
  uint16_t length_read = 255; // number of clock cycles to read
  uint32_t data[5120]; // data packet
  int period = 3;
  // define single or continuous transmission
  bool singleShot = false;
  bool rightConfig = false;
  //bool i_value_bad[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
  bool found_a_good_one = false;

  int best_x=0, best_y=0, sum=100;
  int e[256][32] = {0};
  int s[256][32] = {0};
  // Set DAQ delay to 0
  fpga_->write_fpga(registers::DAQ_DELAY, (uint8_t) 0x00);
  // set DAQ to DSP out data
  salt_->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
  for(int k=1; k<=128; k*=2) {// k*=2) {
    command[0]=k;
    //command[0]=randomPattern();

    command[1]=command[0];
    command[2]=command[0];
    //    cout << "k = " << dec << k << endl;
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
	      //cout << "data = " << hex << (unsigned) data[0] << endl;
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

		  for (int x=0; x<16; x++) {
		    for (int y=0; y<32; y++) {
		      cout << e[x][y] << " ";
		      //cout << "\n";
		      if(e[x][y] != 0)
			continue;
		      if( (x == 0) && (y == 0) ) 
			s[x][y] = e[xmax][ymax] + e[x][ymax] + e[x+1][ymax] + e[xmax][y] + e[x+1][y] + e[xmax][y+1] + e[x][y+1] + e[x+1][y+1];
		      else if ( (x == 0) && (y == ymax) ) 
			s[x][y] = e[xmax][y-1] + e[x][y-1] + e[x+1][y-1] + e[xmax][y] + e[x+1][y] + e[xmax][0] + e[x][0] + e[x+1][0];
		      else if ( (x == xmax) && (y == 0) ) 
			s[x][y] = e[x-1][ymax] + e[x-1][y] + e[x-1][y+1] + e[x][ymax] + e[x][y+1] + e[0][ymax] + e[0][y] + e[0][y+1];
		      else if( (x==xmax) && (y == ymax) )
			s[x][y] = e[x-1][y-1] + e[x][y-1] + e[0][y-1] + e[x-1][y] + e[x][y] + e[0][y] + e[x-1][0] + e[x][0] + e[0][0];
		      else if( (x==0) && ((y != 0) || (y != ymax)) ) 
			s[x][y] = e[xmax][y-1] + e[x][y-1] + e[x+1][y-1] + e[xmax][y] + e[x+1][y] + e[xmax][y+1] + e[x][y+1] + e[x+1][y+1];
		      else if( (y==0) && ((x != 0) || (x != xmax)) ) 
			s[x][y] = e[x-1][ymax] + e[x-1][y] + e[x-1][y+1] + e[x][ymax] + e[x][y+1] + e[x+1][ymax] + e[x+1][y] + e[x+1][y+1];
		      else if( (x == xmax) && ( (y != 0) || (y!=ymax)) )
			s[x][y] = e[x-1][y-1] + e[x][y-1] + e[0][y-1] + e[x-1][y] + e[x+1][y] + e[x-1][y+1] + e[x][y+1] + e[x+1][y+1];
		      else if( (y == ymax) && ( (x != 0) || (x!=xmax)) )
			s[x][y] = e[x-1][y-1] + e[x][y-1] + e[x+1][y-1] + e[x-1][y] + e[x+1][y] + e[x-1][0] + e[x][0] + e[x+1][0];
		      else 
			s[x][y] = e[x-1][y-1] + e[x][y-1] + e[x+1][y-1] + e[x-1][y] + e[x+1][y] + e[x-1][y+1] + e[x][y+1] + e[x+1][y+1];
		
		      if(s[x][y] < sum) { 
			sum = s[x][y];
			best_x = x;
			best_y = y;
		      }
		    }
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

  if(rightConfig) {
    salt_->write_salt(registers::pll_clk_cfg, (uint8_t) (16*best_x+((best_x+7)&0x0F))); //Always valid
    salt_->write_salt(registers::deser_cfg, (uint8_t) best_y);
    
    cout << "best x = " << best_x << endl;
    cout << "best y = " << best_y << endl;
    cout << "sum = " << sum << endl;
  

    for(int i = 0; i < 1000; i++) {
      
      uint8_t k = randomPattern();
      command[0]=k;
      command[1]=command[0];
      command[2]=command[0];
      fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
      if(Check_Ber(data,length_read,command[0]) != 0) {
	
	rightConfig = false;
	cout << "ERROR::Configuration not stable" << endl;
	break;
	
      }
    }
  }
  fastComm_->reset_DAQ(); //Leave it clean for the next user, just in case
  return rightConfig;   // if cann't get right pll config, fail
}
*/
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
  //for(int i=0; i<256; i++) {
  //  salt_->write_salt(registers::calib_fifo_cfg, (uint8_t) i);
 // }
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
  uint16_t length_read = 255;
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
  uint8_t cnt0;
  salt_->read_salt(registers::nzs_cnt0_snap_reg, &cnt0);

tot_fail=0;
  TFC_Reset();
  // DSP output
  salt_->write_salt(registers::ser_source_cfg,(uint8_t) 0x20);
  salt_->write_salt(registers::n_zs_cfg,(uint8_t) 0x21);

  //salt_->read_salt(registers::pack_adc_sync_cfg, &buffer);
  //cout << "BUFFER = " << hex << (unsigned) buffer << endl;
  // set synch pattern registers
  salt_->write_salt(registers::sync1_cfg,(uint8_t) 0x8C);
  salt_->write_salt(registers::sync0_cfg,(uint8_t) 0xAB);

  salt_->write_salt(registers::idle_cfg, (uint8_t) 4);
  salt_->write_salt(registers::pack_adc_sync_cfg, (uint8_t) 0x00);
  // fereset counter for snapshot command check
  uint8_t counter_fere_snap = 0;

salt_->write_salt(registers::n_zs_cfg, (uint8_t) 32);
salt_->write_salt(registers::ana_g_cfg, (uint8_t) 0x92);
salt_->write_salt(registers::baseline_g_cfg, (uint8_t) 255);


  // check TFC commands
  string option[8];
  bool pass[8] = {false};
  option[0] = "FEReset";
  option[2] = "BXReset";
  option[1] = "Header";
  option[3] = "NZS";
  option[4] = "BxVeto";
  option[5] = "Synch";
  option[6] = "Snapshot";
  option[7] = "Normal";
  
  for(int i=0; i<79; i++)
    command[i]=0x04;
  
  for(int i=0; i < 8; i++) {
    //cout << endl;
    if(option[i] == "Normal")        command[79] = 0x00;
    else if(option[i] == "BXReset")  command[79] = 0x01, command[80] = 0x00;
    else if(option[i] == "FEReset")  command[79] = 0x02;
    else if(option[i] == "Header")   (command[79] = 0x04, command[80] = 0x04, command[81] = 0x04);
    else if(option[i] == "NZS")      command[79] = 0x08;
    else if(option[i] == "BxVeto")   command[79] = 0x10;
    else if(option[i] == "Snapshot") (command[79] = 0x02, command[80] = 0x08, command[81] = 0x08, command[82] = 0x20);
    else if(option[i] == "Synch")    {command[79] = 0x40; for(int k=80;k<89;k+=2) {command[k] = 0x40, command[k+1] = 0x10;};};

    //	if(option[i] != "Normal") continue;
    TFC_Reset(); 
    //cout << "option[" << i << "] = " << option[i] << endl;
    //cout << "command[79]" << hex << (unsigned) command[79] << endl;

    fastComm_->Take_a_run(length_read, data, length, 0, command, period, singleShot, true );

    for(int j=0; j< length_read; j++) {
     

      twelveBits[2*j] = fastComm_->read_twelveBits(data[j], 0);
      twelveBits[2*j+1] = fastComm_->read_twelveBits(data[j], 1);
      //if(option[i] == "NZS"){
     // cout << "option[" << i << "] = " << option[i] << endl;
      //cout << hex << setfill('0') << setw(3) << twelveBits[2*j] << endl << setw(3) << twelveBits[2*j+1] << endl;;
     // }
    }
    //cout << endl;
    for(int j=0; j<2*length_read; j++) {
      // cout << hex <<twelveBits[j] << " ";
      if(twelveBits[j] == 0x0F0) { continue;}
      fastComm_->read_Header(twelveBits[j], bxid, parity, flag, length1);
     
	//if(option[i] == "Normal") {
	//		cout << "header =" << hex << (unsigned) twelveBits[j] << endl;
	//		cout << "flag = " << (unsigned) flag << endl;
	//}
		//if(option[i] == "BXReset") cout << "bxid =  " << bxid << endl << "flag = " << flag << endl;

      if(flag == 0) {
	if(option[i] == "Normal") pass[i] = true;
	if(option[i] == "BXReset" && bxid == 0) pass[i] = true; 
	
      }
      if(flag == 1) {
	//cout << "option is " << option[i] << endl;
//	if(option[i] == "NZS")
//	{
//		cout << "Command is " << option[i] << endl;
//		cout << "twelve bits = " << hex << (unsigned)twelveBits[j] << endl;
//	
  //		cout << "length = " << hex << (unsigned) length1 << endl;
//	}
	//if(option[i] == "BxVeto")
	 //     cout << "length = " << hex << length1 << endl;
	if((length1 == 0x11) && (option[i] == "BxVeto")) pass[i] = true;
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
	//cout << dec << (unsigned) buffer << endl;	
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
  tot_fail=counter;
salt_->write_salt(registers::n_zs_cfg, (uint8_t) 0x20);
salt_->write_salt(registers::ana_g_cfg, (uint8_t) 0x04);

  if(counter>0) return false;
  
  return true;
  
}

void Dig_Clk_test::TFC_Reset() {

  // Reset TFC state machine and set all related registers to default values
  fpga_->write_fpga(registers::TFC_CFG,(uint8_t) 0x01);
  usleep(1000);
  fpga_->write_fpga(registers::TFC_CFG,(uint8_t) 0x00);
  
}
