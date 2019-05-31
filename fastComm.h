#pragma once

#include "Fpga.h"

//#include <time.h> 
using namespace std;

class FastComm {

 public:
  FastComm(){};
  FastComm(Fpga*);
  ~FastComm(){};

  void write_tfc();
  void read_daq(uint32_t clock_delay, uint32_t length, int trigger, uint8_t (&packet)[5120]);
  void read_daq(uint8_t length, uint32_t (&packet)[5120], bool tfc_trig);
  void read_daq(uint8_t length, uint32_t *packet);
  string read_daq(uint8_t length);
  uint32_t arrange_Elinks(uint32_t data);
  void read_elinks(uint16_t length_read, uint32_t (&data)[5120]);
  string arrange_Elinks(uint64_t data);
  void Take_a_run(uint16_t length_read, uint32_t (&data)[5120], uint8_t length_write, uint8_t clock_delay, uint8_t command[], uint8_t period, bool singleShot, bool tfc_trig);
//	void Take_a_run(uint16_t length_read, uint32_t *packet, uint8_t length_write, uint8_t clock_delay, uint8_t command[], uint8_t period, bool singleShot, bool tfc_trig);
//  void Take_a_run(uint16_t length_read, uint64_t (&packet)[5120], uint8_t length_write, uint8_t clock_delay, uint8_t command[], uint8_t period, bool singleShot, bool tfc_trig);
	void Take_a_run(uint16_t length_read, string &data, uint8_t length_write, uint8_t clock_delay, uint8_t command[], uint8_t period, bool singleShot, bool tfc_trig);
  void Launch_ACQ(bool tfc_trig);  
  
  
   void config_daq(uint16_t length, uint8_t clock_delay, bool tfc_trig);
   
   void config_tfc(uint8_t length, uint8_t command[], uint8_t period, bool singleShot);

   void reset_DAQ();
	 void DecodeData( uint16_t *decoded_data, uint32_t *data, int L, int NLanes);
   unsigned read_twelveBits(string data, int i);
   unsigned read_twelveBits(uint32_t data, int i);
   void read_Header(unsigned twelveBits, int &bxid, int &parity, int &flag, int &length);
   void read_Normal_packet(string data, int startBit, int (&ADC)[128]);
   void read_Normal_packet(uint16_t length_read, unsigned (&data)[10240], int startBit, int (&ADC)[128]);
	 void read_Normal_packet(uint16_t *data_decoded, int length, int startBit, int *ADC); //NEW FORMAT
   void read_NZS_packet(string data, int startBit, int (&ADC)[128], int &bxid, int &parity, int &mcm_v, int &mcm_ch, int &mem_space);
   void read_NZS_packet(uint16_t length_read, unsigned (&twelveBits)[10240], int startBit, int (&ADC)[128], int &bxid, int &parity, int &mcm_v, int &mcm_ch, int &mem_space);
	 void read_NZS_packet(uint16_t *data_decoded, int length, int startBit, int *ADC, int &bxid, int &parity, int &mcm_v, int &mcm_ch, int &mem_space); //NEW FORMAT

   void trigger_DAQ(bool tfc_trig);
   
 private:
    Fpga *fpga_;

};
