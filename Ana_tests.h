#include <stddef.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "Salt.h"
#include "Fpga.h"
#include "fastComm.h"
#include <fstream>
//#include <vector>

using namespace std;

class Ana_tests {

 public:

  Ana_tests(){};
  Ana_tests(Fpga*, Salt*, FastComm*);
  ~Ana_tests(){};

  void Get_run(string option, int runs, string outText);
  void output_file(int runs, float avg_ADC[], float avg_chip[], float avg_noise, float length_avg, string outText, string option);
  void Trim_dac_scan();
  bool Check_Gain();
  bool Get_noise(int runs, string data_type, string option);
  bool Check_NZS();
  bool Baseline_corr();
  float calculateSD(float data[], int runs);
  float calculateSD(int data[], int runs);
  void adc_output(int min, int bins);
  void xtalk_output();
  void xtalk_outFile();
  void histogram(int start, int bins, int data[], int size, int ch);
  bool set_calib_fifo();
  void gain_output();
  bool uniformity_test();
 bool xtalk_test();
  void enable_ch(int ch);
 void disable_ch(int ch);
void baseline_output();
  void noise_outFile();
  void baseline_outFile();
  void gain_outFile();
  void bad_ch_output();
  //vector<int> m_hist[128];
  int m_hmin;
  int m_bins;
  string m_health;

  float xtalkl[128];
  float xtalkr[128];
  float m_xtalk[128];
  float m_avg_adc[128];
  float m_std_dev[128];
  float m_gain[128];
  float m_gain_std[128];
  float m_gain_pts[3][8];
  float m_offset[128];
  float m_gain_chip;
  float m_offset_chip;
  float m_noise;
  int m_baseline[128];
  float m_baseline_slope[128];


  bool m_ch_pass[128] = {true};
  bool m_baseline_fail[128] = {false};
  bool m_pedestal_fail[128] = {false};
  bool m_noise_fail[128] = {false};
  bool m_gain_lin_fail[128] = {false};
  bool m_gain_range_fail[128] = {false};
  bool m_gain_offset_fail[128] = {false};
  bool m_xtalk_fail[128] = {false};



  float m_noise_rms;
  int m_mcm_ch;
  int m_mcm_v;
  float m_a;
  float m_b;
  float m_c;
  // int m_hist[128];
  int adc_hist[128][64];
  // vector<int> hist;
  bool Check_PedS();
  bool Check_MCMS();
  bool Check_MCMS(float ADC[128], int mcm1, int mcm2, int mcm_ch, int mcm_v);
  bool Check_linear(float x[], float y[], int size, float thresh);
  bool Get_Quad_Coef(float x[], float y[], int PointsNum, float &a, float &b, float &c);
  void Get_quadTerms(float x[], float y[], int npoints, float &a, float &b, float &c);

  private:
  Fpga* fpga_;
  Salt* salt_;
  FastComm* fastComm_;

};
