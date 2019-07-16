#include "DacCalib.h"

void dacCalib_Calibrate(Salt *salt)
{
	struct dacCalib_t 
	{
		std::stringstream Name;
		ExternalADC* ADC;
		uint16_t Channel;
		uint16_t Register;
		uint8_t OriginalRValue;
		uint8_t  Steps;
		float Outcome[256];
	};
	dacCalib_t dacCal[7];
	
	saltChipHandle2=salt;
	ExternalADC *ADC500 = new ExternalADC(0x48,0);
	ExternalADC *ADC510 = new ExternalADC(0x4b,2);
	ExternalADC *ADC520 = new ExternalADC(0x49,2);

	//We set the configuration for all calibrations
	//VSH_DAC
	dacCal[0].Name.str("SH       ");
	dacCal[0].ADC=ADC500; 
	dacCal[0].Channel=ADS1015_REG_CONFIG_MUX_SINGLE_0; 
	dacCal[0].Register=registers::shaper_cfg;
	dacCal[0].Steps=32;

	//V_PRE_DAC
	dacCal[1].Name.str("PRE      ");
	dacCal[1].ADC=ADC500;
	dacCal[1].Channel=ADS1015_REG_CONFIG_MUX_SINGLE_3;
	dacCal[1].Register=registers::preamp_cfg;
	dacCal[1].Steps=32;
	
	//V_SLVS_REF_DAC
	dacCal[2].Name.str("SLVS_REF ");
	dacCal[2].ADC=ADC510;
	dacCal[2].Channel=ADS1015_REG_CONFIG_MUX_SINGLE_3;
	dacCal[2].Register=registers::slvs_vcm_cfg;
	dacCal[2].Steps=32;

	//V_S2D_DAC
	dacCal[3].Name.str("S2D      ");
	dacCal[3].ADC=ADC520;
	dacCal[3].Channel=ADS1015_REG_CONFIG_MUX_SINGLE_1;
	dacCal[3].Register=registers::s2d_cfg;
	dacCal[3].Steps=32;

	//V_SLVS_BIAS_DAC
	dacCal[4].Name.str("SLVS_BIAS");
	dacCal[4].ADC=ADC520;
	dacCal[4].Channel=ADS1015_REG_CONFIG_MUX_SINGLE_2;
	dacCal[4].Register=registers::slvs_cur_cfg;
	dacCal[4].Steps=32;
	
	//V_PULSE_DAC
	dacCal[5].Name.str("PULSE    ");
	dacCal[5].ADC=ADC520;
	dacCal[5].Channel=ADS1015_REG_CONFIG_MUX_SINGLE_0;
	dacCal[5].Register=registers::calib_volt_cfg;
	dacCal[5].Steps=64;

	//V_VCM_DAC AKA V_KRUM_DAC
	dacCal[6].Name.str("VCM      ");
	dacCal[6].ADC=ADC510; 
	dacCal[6].Channel=ADS1015_REG_CONFIG_MUX_SINGLE_2; 
	dacCal[6].Register=registers::vcm_cur_cfg;
	dacCal[6].Steps=64;

	//ABOUT WAITING TIMES
/* 	* Configuring a SALT DAC takes 16b@400KHz >= 40us
		* Configuring all 7 would take 280us
		* Requesting an ADC conversion takes 24b@400KHz >= 60us
			//Total 340us
		* We need to wait ~500us for a DAC output to stabilize
			//We have to wait at least 160us between passes

		* Reading the ones with 64 steps would require waiting more than before
		* We can profit this wait time to read the others again and gain resolution
		* It also makes the procedure homogeneus and easier to code
*/

	for (uint8_t step=0; step<64; step++)
	{
		for(int signal=0; signal < 7; signal ++)
		{
			//Copy original value of the register to restore it later
			if (step==0) saltChipHandle2->read_salt(dacCal[signal].Register, &(dacCal[signal].OriginalRValue));
			saltChipHandle2->write_salt(dacCal[signal].Register, step); //Write DAC I2C	
		}
		usleep(160);
		for(int signal=0; signal < 7; signal ++)
		{
			dacCal[signal].ADC->configADC(); 													//Reset the ADC configuration
			dacCal[signal].ADC->setChannel(dacCal[signal].Channel); 		//Set the right channel
			dacCal[signal].ADC->readSingleShotinMiliVolts(dacCal[signal].Outcome[step]);//Read the ADC		
			if (step >= dacCal[signal].Steps) //We average the ones with 32 values
			{
				dacCal[signal].Outcome[step-32]=(dacCal[signal].Outcome[step-32]+dacCal[signal].Outcome[step])/2;
			}
		}
	}
	
	for (int i=0; i<7; i++) //We present the results
	{
		//Profit to restore the original register value
		saltChipHandle2->write_salt(dacCal[i].Register, dacCal[i].OriginalRValue);
		
		std::cout << dacCal[i].Name.str() <<"[mV]:";
		for (int j=0; j<dacCal[i].Steps; j++)
		{
			//std::cout << " "<< dacCal[i].Outcome[j];
			printf(" %4.0f", dacCal[i].Outcome[j]);
		}
		std::cout << ";" << std::endl << std::endl;
	}
}

/*
	//float Outcome[256];
	//V_SH_DAC in VCMA. -> ADC510, input 0, registers::shaper_cfg, 5b -> 32 values
	for (int a=0; a<10; a++)
	{
		dacCalib_PerformCalib(ADC510, ADS1015_REG_CONFIG_MUX_SINGLE_0, registers::shaper_cfg, 32, Outcome);
		//for (int i=0; i<32; i++) std::cout << "ADC[" << i <<"]= " << Outcome[i] << " mV" << std::endl;
	}
*/

void dacCalib_PerformCalib(ExternalADC *ADC, uint16_t ADCChannel, uint16_t DACRegisterAddress, uint8_t NumOfSteps, float *Outcome)
{
	ADC->setChannel(ADCChannel);
	ADC->configADC();
	for (uint8_t i=0; i< NumOfSteps; i++)
	{
		saltChipHandle2->write_salt(DACRegisterAddress, i); //Write DAC I2C
		usleep(500);
		ADC->readSingleShotinMiliVolts(Outcome[i]);	 //Read the ADC
	}
}
