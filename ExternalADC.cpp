#include "ExternalADC.h"

ExternalADC::ExternalADC(int8_t device_address, int8_t bus_number) : I2C( bus_number, device_address)
{ 
	access_device(); //Initializes the I2C driver
	
	//Default configuration, can be overwritten at any time.
	m_gain = GAIN_FOUR;
	m_channelUsed = ADS1015_REG_CONFIG_MUX_SINGLE_0;
	m_singleShot = ADS1015_REG_CONFIG_MODE_SINGLE;
	m_SPS = ADS1115_REG_CONFIG_DR_860SPS;
} 

ExternalADC::~ExternalADC(){}

void ExternalADC::setGain(adsGain_t gain) 		 {m_gain = gain;} //Call ONLY WITH .h DEFINITIONS!
adsGain_t ExternalADC::getGain() 							 {return m_gain;}

void ExternalADC::setChannel(uint16_t channel) {m_channelUsed = channel;} //Call ONLY WITH .h DEFINITIONS!
uint16_t ExternalADC::getChannel() 						 {return m_channelUsed;}

void ExternalADC::setSingleShot(uint16_t	singleShot) {m_singleShot = singleShot;} //Call ONLY WITH .h DEFINITIONS!
uint16_t ExternalADC::getSingleShot() 								{return m_singleShot;}

void ExternalADC::setSPS(uint16_t	SPS)  				{m_SPS = SPS;} //Call ONLY WITH .h DEFINITIONS!
bool ExternalADC::getSPS() 											{return m_SPS;}

void ExternalADC::configADC() {
  // Start with default static values
  m_config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
             ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
             ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
             ADS1015_REG_CONFIG_CMODE_TRAD; 	 // Traditional comparator (default val)
             
  // Now the variable fields 
  m_config |= m_SPS;  			// Samples Per Second
	m_config |= m_singleShot; // shingleShot or continuous mode
	m_config |= m_channelUsed; // What channel to use
  m_config |= m_gain;   // Set PGA/voltage range
  
  // Write config register to the ADC
  //write_buffer(ADS1015_REG_POINTER_CONFIG, (uint8_t)((m_config&0xFF00)>>8),(uint8_t)(m_config&0x00FF));
	//std::cout << "Writing to ADC:" << std::hex << m_config << std::dec << std::endl;
}

void ExternalADC::readSingleShot(int16_t &adc_counts)
{
	uint16_t	tempConfig=m_config;
	uint16_t	temp;
	uint8_t 	tempConfigL, 	tempConfigH;
	
	tempConfig |= ADS1015_REG_CONFIG_OS_SINGLE; 	// Set 'start single-conversion' bit
	tempConfigL = (uint8_t)  (tempConfig&0x00FF);
	tempConfigH = (uint8_t) ((tempConfig&0xFF00)>>8);

	write_buffer(ADS1015_REG_POINTER_CONFIG, tempConfigH,tempConfigL); //Trigger acquisition
	usleep(ConversionDelay[m_SPS>>5]); 	// Wait for the conversion to complete (depends on the SPS)
  read_buffer(ADS1015_REG_POINTER_CONVERT,&temp); // Read the conversion results
  adc_counts = (temp <= 0x7FFF ? temp : 0xFFFF-temp); //Change sign (not really needed)
}

void ExternalADC::readSingleShotinMiliVolts(float &V)
{
		int16_t ADC_Counts;
		float VPS; //Vots per step;
		switch (m_config & ADS1015_REG_CONFIG_PGA_MASK)
		{
			case ADS1015_REG_CONFIG_PGA_6_144V:
				VPS =6144.0/32767.0;
				break;
			case ADS1015_REG_CONFIG_PGA_4_096V:
				VPS =4096.0/32767.0;
				break;
			case ADS1015_REG_CONFIG_PGA_2_048V:
				VPS =2048.0/32767.0;
				break;
			case ADS1015_REG_CONFIG_PGA_1_024V:
				VPS =1024.0/32767.0;
				break;
			case ADS1015_REG_CONFIG_PGA_0_512V:
				VPS =0512.0/32767.0;
				break;
			case ADS1015_REG_CONFIG_PGA_0_256V:
				VPS =0256.0/32767.0;
				break;
			default :
				VPS = 0;
				break;
		}
		readSingleShot(ADC_Counts);
		V=ADC_Counts*VPS;
}
