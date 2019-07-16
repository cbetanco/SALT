//ExternalADC.h
#pragma once

#include "I2C.h"
#include <fstream>
#include <cstring>
#include <iomanip>

/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
    #define ADS1015_REG_POINTER_MASK        (0x03)
    #define ADS1015_REG_POINTER_CONVERT     (0x00)
    #define ADS1015_REG_POINTER_CONFIG      (0x01)
    #define ADS1015_REG_POINTER_LOWTHRESH   (0x02)
    #define ADS1015_REG_POINTER_HITHRESH    (0x03)
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
    #define ADS1015_REG_CONFIG_OS_MASK      (0x8000)
    #define ADS1015_REG_CONFIG_OS_SINGLE    (0x8000)  // Write: Set to start a single-conversion
    #define ADS1015_REG_CONFIG_OS_BUSY      (0x0000)  // Read: Bit = 0 when conversion is in progress
    #define ADS1015_REG_CONFIG_OS_NOTBUSY   (0x8000)  // Read: Bit = 1 when device is not performing a conversion

    #define ADS1015_REG_CONFIG_MUX_MASK     (0x7000)
    #define ADS1015_REG_CONFIG_MUX_DIFF_0_1 (0x0000)  // Differential P = AIN0, N = AIN1 (default)
    #define ADS1015_REG_CONFIG_MUX_DIFF_0_3 (0x1000)  // Differential P = AIN0, N = AIN3
    #define ADS1015_REG_CONFIG_MUX_DIFF_1_3 (0x2000)  // Differential P = AIN1, N = AIN3
    #define ADS1015_REG_CONFIG_MUX_DIFF_2_3 (0x3000)  // Differential P = AIN2, N = AIN3
    #define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000)  // Single-ended AIN0
    #define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000)  // Single-ended AIN1
    #define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000)  // Single-ended AIN2
    #define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000)  // Single-ended AIN3

    #define ADS1015_REG_CONFIG_PGA_MASK     (0x0E00)
    #define ADS1015_REG_CONFIG_PGA_6_144V   (0x0000)  // +/-6.144V range = Gain 2/3
    #define ADS1015_REG_CONFIG_PGA_4_096V   (0x0200)  // +/-4.096V range = Gain 1
    #define ADS1015_REG_CONFIG_PGA_2_048V   (0x0400)  // +/-2.048V range = Gain 2 (default)
    #define ADS1015_REG_CONFIG_PGA_1_024V   (0x0600)  // +/-1.024V range = Gain 4
    #define ADS1015_REG_CONFIG_PGA_0_512V   (0x0800)  // +/-0.512V range = Gain 8
    #define ADS1015_REG_CONFIG_PGA_0_256V   (0x0A00)  // +/-0.256V range = Gain 16

    #define ADS1015_REG_CONFIG_MODE_MASK    (0x0100)
    #define ADS1015_REG_CONFIG_MODE_CONTIN  (0x0000)  // Continuous conversion mode
    #define ADS1015_REG_CONFIG_MODE_SINGLE  (0x0100)  // Power-down single-shot mode (default)

    #define ADS1015_REG_CONFIG_DR_MASK      (0x00E0)  
    #define ADS1115_REG_CONFIG_DR_8SPS	    (0x0000)  // 8 samples per second
    #define ADS1115_REG_CONFIG_DR_16SPS     (0x0020)  // 16 samples per second
    #define ADS1115_REG_CONFIG_DR_32SPS     (0x0040)  // 32 samples per second
    #define ADS1115_REG_CONFIG_DR_64SPS     (0x0060)  // 64 samples per second
    #define ADS1115_REG_CONFIG_DR_128SPS    (0x0080)  // 128 samples per second (default)
    #define ADS1115_REG_CONFIG_DR_250SPS    (0x00A0)  // 250 samples per second
    #define ADS1115_REG_CONFIG_DR_475SPS    (0x00C0)  // 475 samples per second
    #define ADS1115_REG_CONFIG_DR_860SPS    (0x00E0)  // 860 samples per second

    #define ADS1015_REG_CONFIG_CMODE_MASK   (0x0010)
    #define ADS1015_REG_CONFIG_CMODE_TRAD   (0x0000)  // Traditional comparator with hysteresis (default)

    #define ADS1015_REG_CONFIG_CPOL_MASK    (0x0008)
    #define ADS1015_REG_CONFIG_CPOL_ACTVLOW (0x0000)  // ALERT/RDY pin is low when active (default)

    #define ADS1015_REG_CONFIG_CLAT_MASK    (0x0004)  // Determines if ALERT/RDY pin latches once asserted
    #define ADS1015_REG_CONFIG_CLAT_NONLAT  (0x0000)  // Non-latching comparator (default)

    #define ADS1015_REG_CONFIG_CQUE_MASK    (0x0003)
    #define ADS1015_REG_CONFIG_CQUE_NONE    (0x0003)  // Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

typedef enum
{
  GAIN_TWOTHIRDS    = ADS1015_REG_CONFIG_PGA_6_144V,
  GAIN_ONE          = ADS1015_REG_CONFIG_PGA_4_096V,
  GAIN_TWO          = ADS1015_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR         = ADS1015_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT        = ADS1015_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN      = ADS1015_REG_CONFIG_PGA_0_256V
} adsGain_t;

class ExternalADC :  public I2C {
    public:
    		ExternalADC(int8_t, int8_t);
        ~ExternalADC();
        
				void setGain(adsGain_t gain); 			//Use only with the constants above!
				adsGain_t getGain(); 								//Answers with the constants above
				
				void setChannel(uint16_t channel); 	//Use only with the constants above!
				uint16_t getChannel();							//Answers with the constants above
				
				void setSingleShot(uint16_t	singleShot); //Use only with the constants above!
				uint16_t getSingleShot();						//Answers with the constants above
						//Does not capture any data. It only reads the status of "Single shot" config
						
				void setSPS(uint16_t	SPS); 				//Use only with the constants above!
				bool getSPS(); 											//Answers with the constants above
				
				void configADC(); 									//Writes the config, none of the above functions does it on its own...
				
				void readSingleShot(int16_t &adc_counts); //Configure ONCE before using it
				void readSingleShotinMiliVolts(float &V); //Configure ONCE before using it
				
    private:
				adsGain_t m_gain;
				uint16_t	m_channelUsed;
				uint16_t	m_singleShot;
				uint16_t	m_SPS;
				uint16_t	m_config;
				
				const unsigned int ConversionDelay[8]={125000, 62500, 31250, 15625, 7813, 4000, 2106, 1163};
					//In microseconds. Usage example: usleep(ConversionDelay[m_SPS>>5]);
};
