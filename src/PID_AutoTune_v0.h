#ifndef PID_AutoTune_v0
#define PID_AutoTune_v0

/* 	Code derived from: http://playground.arduino.cc/Code/PIDAutotuneLibrary
	Which was ported from: http://www.mathworks.com/matlabcentral/fileexchange/4652-autotunerpid-toolkit
*/

void PID_ATune(float*, float*);                       	// * Constructor.  links the Autotune to a given PID
int Runtime();						   			   	// * Similar to the PID Compue function, returns non 0 when done
void Cancel();									   	// * Stops the AutoTune	

void SetOutputStep(float);						   	// * how far above and below the starting value will the output step?	
float GetOutputStep();							   	// 

void SetControlType(int); 						   	// * Determies if the tuning parameters returned will be PI (D=0)
int GetControlType();							   	//   or PID.  (0=PI, 1=PID)			

void SetLookbackSec(int);							// * how far back are we looking to identify peaks
int GetLookbackSec();								//

void SetNoiseBand(float);							// * the autotune will ignore signal chatter smaller than this value
float GetNoiseBand();								//   this should be acurately set

float GetKp();										// * once autotune is complete, these functions contain the
float GetKi();										//   computed tuning parameters.  
float GetKd();										//

#endif

