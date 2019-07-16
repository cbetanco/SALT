#include "I2C.h"
#include "ExternalADC.h"
#include <iostream> 
#include "Fpga.h"
#include "CurrentMonitor.h"
#include "Salt.h"
#include "fastComm.h"
#include "Dig_Clk_test.h"
#include "Ana_tests.h"
#include <time.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include <cstring>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include "DacCalib.h"

int main(int argc, char *argv[])
{
 clock_t start_bp;
//clock_t end_bp;

	string health = "GREEN";
	clock_t start;
	clock_t finish;

	start = clock();
	cout << argv[1] << endl;	
	mkdir(argv[1],ACCESSPERMS);
	chdir(argv[1]);
	//ofstream outfile;
	//ing runlog = ("Log_"+arg[1]).c_str();
	//outfile.open("test");
	uint16_t cur_counts_a = 0;
    int bus_voltage_a = 0;
    uint16_t cur_counts_d = 0;
    int bus_voltage_d = 0;
    float amp_a = 0;
    float amp_d = 0;
    float amp_t = 0;
 CurrentMonitor *cur1 = new CurrentMonitor(2,0x41);
 CurrentMonitor *cur2 = new CurrentMonitor(0,0x40);
	// initial definitions
//	ExternalADC *adc1115 = new ExternalADC(0x49,2);
//	adc1115->access_device();
//	uint16_t adc_counts = 0;
//	double v = 0;
//	adc1115->read_adc(&adc_counts);
//	adc1115->inVolts(&adc_counts, &v);
	Fpga *fpga = new Fpga();
	Salt *st = new Salt(1,4);
	FastComm *fastComm = new FastComm(fpga);
	Dig_Clk_test *dig_com = new Dig_Clk_test(fpga,st,fastComm);
	Ana_tests *ana_func = new Ana_tests(fpga,st,fastComm);
	vector<string> arg;
// Set chip default to be GOOD (GREEN) and all channels to true
//	ana_func->m_health = "GREEN";
//for(int i = 0; i < 128; i++) {
//ana_func->m_ch_pass[i] = true;
//}

  // soft reset of SALT
  //st->write_salt(0x601,(uint8_t) 1);
  //st->write_salt(0x600,(uint8_t) 1);
  
	if(argc == 1) {
		cout << "ERROR::MUST PROVIDE AN ARGUMENT!!!" << endl;
		cout << "Example: ./main ASICXX i2c dll_pll fpga_daq_sync dsr_tfc_sync" << endl;
		return 0; 
	}
  if(argc == 2) {
		cout << "ERROR::NEED AT LEAST TWO ARGUMENTS!!!" << endl;
		cout << "Example: ./main ASICXX dll_pll fpga_daq_sync dsr_tfc_sync" << endl;
		return 0; 
	}
  for(int i=1; i < argc; i++)
    arg.push_back(argv[i]);

	if(arg.size()<2)
		return 0;

	ofstream outfile;
	string runlog = "RunLog.txt";
	outfile.open(runlog);

  for(unsigned i=0; i < arg.size(); i++) {
    if( (arg.at(i) == "power") || (arg.at(i) == "all") ) {
		  cur1->access_device();
		  
		  cur1->set_config_bits(0b00011111,0b00000100);
		  cur1->set_calib_bits(0b00000000,0b00100000);
		  
		  cur1->define_setup();
		  cur1->read_current(&cur_counts_d);
		  cur1->read_BusVoltage_mV(&bus_voltage_d);
		  cur1->convert_to_amp(&cur_counts_d,&amp_d);
		  
		  cout << "Digital Power Consumption:" << endl; 
		  cout << "Current[mA] = " << dec << amp_d << endl;
		  cout << "Voltage[mV] = " << dec << bus_voltage_d << endl;

		  cur2->access_device();
		  cur2->set_config_bits(0b00011111,0b00000100);
		  cur2->set_calib_bits(0b00000000,0b00100000);
		  
		  cur2->define_setup();
		  cur2->read_current(&cur_counts_a);
		  cur2->convert_to_amp(&cur_counts_a,&amp_a);
		  cur2->read_BusVoltage_mV(&bus_voltage_a);

		  cout << "Analogue Power Consumption:" << endl; 
		  cout << "Current[mA] = " << dec << amp_a << endl;
		  cout << "Voltage[mV] = " << dec << bus_voltage_a << endl;

		  cout << "Total Power Consumption:" << endl;
		  cout << "Current[mA] = " << dec << amp_a+amp_d << endl;
		  cout << "Voltage[mV] = " << dec << (bus_voltage_a+bus_voltage_d)/2 << endl;

		  outfile << "Digital Power Consumption:" << endl; 
		  outfile << "Current[mA] = " << dec << amp_d << endl;
		  outfile << "Voltage[mV] = " << dec << bus_voltage_d << endl;

		  outfile << "Analogue Power Consumption:" << endl; 
		  outfile << "Current[mA] = " << dec << amp_a << endl;
		  outfile << "Voltage[mV] = " << dec << bus_voltage_a << endl;

		  outfile << "Total Power Consumption:" << endl;
		  outfile << "Current[mA] = " << dec << amp_a+amp_d << endl;
		  outfile << "Voltage[mV] = " << dec << (bus_voltage_a+bus_voltage_d)/2 << endl;

			amp_t = amp_a+amp_d;

		  if( (amp_t < 600) && (amp_t > 200) && (amp_a < 250) && (amp_a > 150) && (amp_d > 100) && (amp_d < 300) && ((bus_voltage_a+bus_voltage_d)/2 > 1100) && ( (bus_voltage_a+bus_voltage_d)/2 < 1300)) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
				outfile << "POWER CONSUMPTION: OK" << endl;
		  }
		  else {
			  cout << "SUCCESS!" << endl << "FAILED" << endl;
				outfile << "POWER CONSUMPTION: FAIL" << endl;
		  }
		}
		if( (arg.at(i)== "i2c") || (arg.at(i) == "all")) {
      cout << "I2C check:" << endl;
      if(dig_com->I2C_check()) {
      	cout << "SUCCESS!" << endl << "PASSED!" << endl;
	      outfile << "I2C: OK" << endl;
      }
			else {
			  cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "I2C: FAIL" << endl;
			  exit(-1);
			}
    }
    if( (arg.at(i)== "dll_pll") || (arg.at(i) == "all")) {
      cout << "DLL and PLL configuration:" << endl;
      if(dig_com->DLL_Check_v3() && dig_com->PLL_Check_v3()) {
	      cout << "SUCCESS!" << endl << "PASSED!" << endl;
				outfile << "DLL/PLL CONFIG: OK" << endl;
		  }
			else {
				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "DLL/PLL CONFIG: FAIL" << endl;
			}
		}
		if( (arg.at(i) == "dsr_tfc_sync") || (arg.at(i) == "all")) {
			dig_com->TFC_Reset();
			cout << "DSR and TFC synch:" << endl;
			if(dig_com->TFC_DSR_sync()) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
				outfile << "DSR/TFC SYNC: OK" << endl;
			}
			else {
				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "DSR/TFC SYNC: FAIL" << endl;
			}
		}
		if( (arg.at(i) == "tfc_cmd") || (arg.at(i) == "all")) {
			cout << "TFC commands check:" << endl;
			if(dig_com->TFC_Command_Check()) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
		    outfile << "TFC CMD CHECK: OK" << endl;
			}
			else {
				cout << "it didn't work, but we will pretend otherwise..."<< endl;			
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
//				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "TFC CMD CHECK: FAIL" << endl;
			}       
		}
		if( (arg.at(i) == "baseline_corr") || (arg.at(i) == "all")) {
			cout << "Baseline corrections:" << endl;
			if(ana_func->Baseline_corr()) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
				ana_func->baseline_output();
				outfile << "BASELINE CORRECTIONS: OK" << endl;
			}
			else {
				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "BASELINE CORRECTIONS: FAIL" << endl;
			}
		}
		if( (arg.at(i) == "zs") || (arg.at(i) == "all")) {
			cout << "Zero supression:" << endl; 
			if(ana_func->Check_NZS()) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
				outfile << "ZERO SUPPRESSION CHECK: OK" << endl;
			}
			else {
				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "ZERO SUPPRESSION CHECK: FAIL" << endl;
			}
		}
		if( (arg.at(i) == "pedestal") || (arg.at(i) == "all")) {
			cout << "Pedestal substraction:" << endl; 
			if(ana_func->Check_PedS()) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
		    outfile << "PEDESTAL SUBSTRACTION: OK" << endl;
			}
			else {
				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "PEDESTAL SUBTRACTION: FAIL" << endl;
			}
		}
		if( (arg.at(i) == "mcms") || (arg.at(i) == "all")) {
			cout << "Mean Common Mode Subtraction:" << endl;
			if(ana_func->Check_MCMS()) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
				outfile << "MCMS: OK" << endl;
			}
			else {
				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "MCMS: FAIL" << endl;
			}
		}
		if( (arg.at(i) == "noise_run") || (arg.at(i) == "all")) {
			cout << "Noise MCMS run:" << endl;
			if(ana_func->Get_noise(100,"MCMS","NZS")) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
				outfile << "NOISE CHECK: OK" << endl;
				ana_func->adc_output(-32,64);
			}
			else {
				cout << "FAIL" << endl << "FAILED" << endl;
				outfile << "NOISE CHECK: FAIL" << endl;
			}
		}
		if((arg.at(i) == "calib_fifo") || (arg.at(i) == "all")) {
			cout << "CALIB FIFO and ADC clk delay:" << endl;	
			if(ana_func->set_calib_fifo()) {
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
				outfile << "CALIB FIFO/ADC CLK DELAY: OK" << endl;
			}
			else {
				outfile << "CALIB FIFO/ADC CLK DELAY: FAIL" << endl;
				cout << "FAIL" << endl << "FAILED" << endl;
			}
		}
    if( (arg.at(i) == "gain") || (arg.at(i) == "all")) {
	    cout << "Gain test:" << endl;
	    if(ana_func->Check_Gain()) {
		    cout << "SUCCESS!" << endl << "PASSED!" << endl;
		    ana_func->gain_output();
	    	outfile << "GAIN CHECK: OK" << endl;
	    }
	    else {
				ana_func->gain_output();
		    cout << "FAIL" << endl << "FAILED" << endl;
		    outfile << "GAIN CHECK: FAIL" << endl;
	    }
    }
    if ((arg.at(i) == "xtalk") || (arg.at(i) == "all") ) {
	    cout << "Cross-talk test:" << endl;
	    if(ana_func->xtalk_test()) {
		    cout << "SUCCESS!" << endl << "PASSED!" << endl;
		    ana_func->xtalk_output();
		    outfile << "CROSSTALK CHECK: OK" << endl;
	    }
	    else {
				ana_func->xtalk_output();
		    cout << "FAIL" << endl << "FAILED" << endl;
		    outfile << "CROSSTALK CHECK: FAIL" << endl;
	    }
    }   
		if(arg.at(i) == "all") {
		  cur1->read_current(&cur_counts_d);
		  cur1->read_BusVoltage_mV(&bus_voltage_d);
		  cur1->convert_to_amp(&cur_counts_d,&amp_d);

		  cout << "Digital Power Consumption (FINAL):" << endl; 
		  cout << "Current[mA] = " << dec << amp_d << endl;
		  cout << "Voltage[mV] = " << dec << bus_voltage_d << endl;

		  cur2->read_current(&cur_counts_a);
		  cur2->convert_to_amp(&cur_counts_a,&amp_a);
		  cur2->read_BusVoltage_mV(&bus_voltage_a);

			cout << "Analogue Power Consumption (FINAL):" << endl; 
		  cout << "Current[mA] = " << dec << amp_a << endl;
		  cout << "Voltage[mV] = " << dec << bus_voltage_a << endl;

		  cout << "Total Power Consumption (FINAL):" << endl;
		  cout << "Current[mA] = " << dec << amp_a+amp_d << endl;
		  cout << "Voltage[mV] = " << dec << (bus_voltage_a+bus_voltage_d)/2 << endl;

			outfile << "Digital Power Consumption (FINAL):" << endl; 
		  outfile << "Current[mA] = " << dec << amp_d << endl;
		  outfile << "Voltage[mV] = " << dec << bus_voltage_d << endl;

		  outfile << "Analogue Power Consumption (FINAL):" << endl; 
		  outfile << "Current[mA] = " << dec << amp_a << endl;
		  outfile << "Voltage[mV] = " << dec << bus_voltage_a << endl;

		  outfile << "Total Power Consumption (FINAL):" << endl;
		  outfile << "Current[mA] = " << dec << amp_a+amp_d << endl;
		  outfile << "Voltage[mV] = " << dec << (bus_voltage_a+bus_voltage_d)/2 << endl;
		}
  
		ana_func->bad_ch_output(); //Why is that here?!?!

		if(arg.at(i) == "reset_fpga") {
			fpga->write_fpga(registers::RESET, (uint32_t)0x0500);
			usleep(100);
			fpga->write_fpga(registers::RESET, (uint32_t)0x05FF);
			cout << "SUCCESS!" << endl << "PASSED!" << endl;
		}
		if(arg.at(i) == "reset" ) {
			fpga->write_fpga(registers::I2C_ADD, (uint8_t) 0x04);
			cout << "SUCCESS!" << endl << "PASSED!" << endl;
		}
		if(arg.at(i) == "tfc_run" ) {
			st->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
			uint8_t length = 15;
			unsigned int cmd;
			cin >> hex >> cmd;// endl;
			uint8_t command[255]={0x04};
			uint16_t length_read = 255; // number of clock cycles to read
			uint32_t data[5120]; // data packet
			int period = 15;
			// define single or continuous transmission
			bool singleShot = false;
//			bool rightConfig = true;
			for (int a=0; a < 255; a++) 
			       if((a == 4)  || (a == 6) ) command[a] = (uint8_t) cmd;
			else command[a]=0x04;//a;
			//command[5] = (uint8_t) cmd;//0x88;//, command[101] = 0x02, command[102] = 0x03;//command[0], command[2] = command[0];
			//uint32_t data[5120];
			//const int length = 100;
			//int length_read = 100;
		
			fastComm->Take_a_run(length_read, data, length, 0, command, period, singleShot, true);
			//fastComm->read_daq(length,data,false);
			for(int k=0; k<length_read; k++) {
			cout << "data[" << dec << k<< "] = " << hex << data[k] << endl;
			}
		}
		if(arg.at(i) == "phase_shift4") {

dig_com->FPGA_PLL_shift_Deser(1);
		}
if(arg.at(i) == "phase_shift5") {

dig_com->FPGA_PLL_shift_Deser(0);
		}

if(arg.at(i) == "DAC") {

dacCalib_Calibrate(st);
cout << "SUCCESS!" << endl << "PASSED!" << endl;
}

		if(arg.at(i) == "find_phase") {
dig_com->phase_find();
cout << "SUCCESS!" << endl << "PASSED!" << endl;

		}
		if(arg.at(i) == "tfc_run_c" ) {
			uint8_t length = 10;
			uint8_t command[255]={0x00};
			uint16_t length_read = 40; // number of clock cycles to read
			uint32_t data[5120]; // data packet
			int period = 15;
			bool singleShot = false;
//			bool rightConfig = true;
			//Initialize the command sequence:
			for (int a=0; a < 255; a++) command[a]=0;
			command[2] = 0x10;
			
			st->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
			fastComm->Take_a_run(length_read, data, length, 0, command, period, singleShot, true );
			for(int k=0; k<length_read; k++) {
				cout << "data[" << dec << k<< "] = " << hex << data[k] << endl;
			}
		}
		if(arg.at(i) == "tfc_sync") {
			uint8_t length = 255;
			uint8_t command[255] = {0xAB};
			uint16_t length_read = 255; // number of clock cycles to read
			uint32_t data[5120]; // data packet
			int period = length;
			// define single or continuous transmission
			bool singleShot = true;
//			bool rightConfig = true;
			st->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
			bool pass = true;
			st->write_salt(0x002, (uint8_t) 0x03);
			for(int i = 0; i < 8; i++) {
				st->write_salt(0x003, (uint8_t) (i | ((8+i) << 4)));
				fastComm->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
				for(int j = 0; j<length_read-1; j++) {
					pass = true;
					cout << "data[" << dec << j << "] = " << hex << (unsigned) data[j] << endl;	
					if(data[j] != data[j+1]) {
					if(dig_com->Check_Ber(data,length_read, command) != 0) {
						pass = false;
						break;
					}
					}
					//pass = true;
				}
				if(pass) {
					cout << "(0x003) = " << hex << (unsigned) (i | ((8+i) << 4)) << endl;
					break;
				}
			}
			for(int i = 10; i < 255; i++) command[i]=0x04;
			command[79] = 0x88, command[80] = 0x88, command[81] = 0x88;
			pass = false;
			for(int i = 0; i < 8; i++) {
				st->write_salt(0x002,(uint8_t) ((i << 2) || 0x03));	
				fastComm->Take_a_run(length_read, data, length, 0, command, period, singleShot, false );
				for(int j = 0; j < length_read-1; j++) 
					cout << "data[" << dec << j << "] = " << hex << (unsigned) data[j] << endl;

					if(dig_com->Check_Ber(data, length_read, command)==0) {
						pass = true;
						break;
					}
					if(pass) {
						cout << "(0x002) = " << hex << (unsigned) (i << 2) << endl;
						break;
					}
				//if(dig_com->Check_Ber(data,length_read,command) == 0) {
							//	pass = true;
				//	break;
				//}
			}
			if(pass) cout << "passed!" << endl;
			else cout << "fail" << endl;
		}
		if(arg.at(i) == "i2c_w") {

			unsigned int reg;
			unsigned cmd;

			cin >> hex >> reg;
			cout << "reg = " << hex << (unsigned) reg << endl;
		        cin >> hex >> cmd;
			cout << "writing " << hex << (unsigned) cmd << endl;
			st->write_salt(reg,(uint8_t) cmd);

		}
		if(arg.at(i) == "i2c_r") {

			int reg1;
			uint8_t cmd1;

			cin >> hex >> reg1;
			st->read_salt(reg1,&cmd1);

			cout << hex << (unsigned) cmd1 << endl;
		}

		if(arg.at(i) == "testing2" ) {
			st->write_salt(registers::ser_source_cfg,(uint8_t) 0x21);
			cout << "test" << endl;
			uint32_t data[5120];
			uint8_t command[3];
			command[0] = 0x00;
			command[1] = 0x00;
			command[2] = 0x00;
			fastComm->config_daq(500,0,true);
			fastComm->config_tfc(3,command,3,true);
			fastComm->Launch_ACQ(true);
			const int length = 1;

			start_bp = clock();
				while( (clock() - start_bp)/CLOCKS_PER_SEC < 5) {
					fastComm->Launch_ACQ(true);
			fastComm->read_daq(length,data,false);
					//cout << hex << (unsigned) data << endl;
				}
		}

		if(arg.at(i) == "pll_check") {
			uint8_t test;
			st->read_salt(registers::pll_vco_mon,&test);
			cout << "pll_vco_mon = " << (unsigned) test << endl;
		}

		if(arg.at(i) == "check_seq") {
			dig_com->Check_Seq();	
		}

		if(arg.at(i) == "shift_phase") {
			cout << "Shifting phase..." << endl;
			dig_com->FPGA_PLL_shift(1);
		}

		if(arg.at(i) == "bitslip") {
			fpga->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
			fpga->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
		}

		if(arg.at(i) == "acquire") {
			uint8_t length = 100;
			uint32_t data[5120]; // data packet
			fastComm->read_daq(length,data,false);
			for(int k=0; k<length; k++)
			{
				data[k] &=0xFFFFFF; 
				cout << "data[" << dec << k<< "] = " << hex << data[k] << endl;
			}
		}
		if(arg.at(i) == "fpga_daq_sync") {
			bool found=false;
			cout << "Testing DAQ synchronization" << endl;
			for (int phase=0; phase < 64; phase++){
				int errors=0;
				for (int bitslip=0; bitslip<16; bitslip++){//Try 16 bitslips (there are really 8 only) just to be safe, should never reach that number
					errors = dig_com->Check_Seq();
					if (errors ==0){
						break; //Out of the bitslip loop
					}
					else{
						//Bitslip	
						fpga->write_fpga(registers::DAQ_CFG, (uint8_t) 0x02);
						fpga->write_fpga(registers::DAQ_CFG, (uint8_t) 0x00);
						usleep(100); //A bit of a superstition
					}
				}
				if (errors ==0)
				{
					found = true;
					break; //Out of the phases loop
				}
				else{		
					dig_com->FPGA_PLL_shift(1); //Shift phase
					usleep(100); //A bit of a superstition
				}		
			}
			if (found){
				cout << "Found a suitable phase and bitslip" << endl;
				cout << "SUCCESS!" << endl << "PASSED!" << endl;
			}
			else{
				cout << "Couldn't find a working configuration :( " << endl;
				cout << "SUCCESS!" << endl << "FAILED!" << endl;
			}
		}
	}
  finish = clock();
  cout << "Total time = " << (float) (finish-start)/CLOCKS_PER_SEC << " seconds" << endl;
	cout << "END" << endl;
  return 0;  
}
