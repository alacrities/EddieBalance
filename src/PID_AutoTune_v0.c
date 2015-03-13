#include "PID_AutoTune_v0.h"
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
unsigned long millis() 
{
    struct timeval c_time; 
    gettimeofday(&c_time, NULL);
    unsigned long milliseconds = c_time.tv_sec*1000 + c_time.tv_usec/1000;
    return milliseconds;
}


void FinishUp();
int isMax, isMin;
float *input, *output;
float setpoint;
float noiseBand;
int controlType;
int running;
unsigned long peak1, peak2, lastTime;
int sampleTime = 0;
int nLookBack = 0;
int peakType = 0;
float lastInputs[101] = {0};
float peaks[10] = {0};
int peakCount = 0;
int justchanged = 0;
int justevaled = 0;
float absMax, absMin = 0;
float oStep = 0;
float outputStart = 0;
float Ku, Pu = 0;

void PID_ATune(float* Input, float* Output)
{
	input = Input;
	output = Output;
	controlType = 1; //default to PID
	noiseBand = 0.5;
	running = 0;
	oStep = 3; //30 Default provided
	SetLookbackSec(3); //10 Default provided
	lastTime = millis();
	
}



void Cancel()
{
	running = 0;
} 
 
int Runtime()
{
//printf("Runtime() called\r\n");
	justevaled=0;
	if(peakCount>9 && running)
	{
		running = 0;
		FinishUp();
		return 1;
	}
//printf("Runtime() millis\r\n");
	unsigned long now = millis();
	
	if((now-lastTime)<sampleTime) return 0;
	lastTime = now;
//printf("Runtime() setting refVal\r\n");
	float refVal = *input;
//printf("Runtime() set refVal\r\n" );
	justevaled=1;
	if(!running)
	{ //initialize working variables the first time around
printf("Runtime() detected first time\r\n");
		peakType = 0;
		peakCount=0;
		justchanged=0;
		absMax=refVal;
		absMin=refVal;
		setpoint = refVal;
		running = 1;
		outputStart = *output;
		*output = outputStart+oStep;
	}
	else
	{
		if(refVal>absMax)absMax=refVal;
		if(refVal<absMin)absMin=refVal;
	}
//printf("Runtime() 1\r\n");	
	//oscillate the output base on the input's relation to the setpoint
	
	if(refVal>setpoint+noiseBand) *output = outputStart-oStep;
	else if (refVal<setpoint-noiseBand) *output = outputStart+oStep;
	
//printf("Runtime() 2\r\n");	
  //bool isMax=1, isMin=1;
  isMax=1;isMin=1;
  //id peaks
  int i=0;
  for(i=nLookBack-1;i>=0;i--)
  {
    float val = lastInputs[i];
    if(isMax) isMax = refVal>val;
    if(isMin) isMin = refVal<val;
    lastInputs[i+1] = lastInputs[i];
  }
  lastInputs[0] = refVal;  
  if(nLookBack<9)
  {  //we don't want to trust the maxes or mins until the inputs array has been filled
	return 0;
	}
  
  if(isMax)
  {
    if(peakType==0)peakType=1;
    if(peakType==-1)
    {
      peakType = 1;
      justchanged=1;
      peak2 = peak1;
    }
    peak1 = now;
    peaks[peakCount] = refVal;
   
  }
  else if(isMin)
  {
    if(peakType==0)peakType=-1;
    if(peakType==1)
    {
      peakType=-1;
      peakCount++;
      justchanged=1;
    }
    
    if(peakCount<10)peaks[peakCount] = refVal;
  }
//printf("Runtime() 3\r\n");  
  if(justchanged && peakCount>2)
  { //we've transitioned.  check if we can autotune based on the last peaks
    float avgSeparation = (abs(peaks[peakCount-1]-peaks[peakCount-2])+abs(peaks[peakCount-2]-peaks[peakCount-3]))/2;
    if( avgSeparation < 0.05*(absMax-absMin))
    {
		FinishUp();
      running = 0;
	  return 1;
	 
    }
  }
   justchanged=0;
	return 0;
}
void FinishUp()
{
	  *output = outputStart;
      //we can generate tuning parameters!
      Ku = 4*(2*oStep)/((absMax-absMin)*3.14159);
      Pu = (float)(peak1-peak2) / 1000;
}

float GetKp()
{
	return controlType==1 ? 0.6 * Ku : 0.4 * Ku;
}

float GetKi()
{
	return controlType==1? 1.2*Ku / Pu : 0.48 * Ku / Pu;  // Ki = Kc/Ti
}

float GetKd()
{
	return controlType==1? 0.075 * Ku * Pu : 0;  //Kd = Kc * Td
}

void SetOutputStep(float Step)
{
	oStep = Step;
}

float GetOutputStep()
{
	return oStep;
}

void SetControlType(int Type) //0=PI, 1=PID
{
	controlType = Type;
}
int GetControlType()
{
	return controlType;
}
	
void SetNoiseBand(float Band)
{
	noiseBand = Band;
}

float GetNoiseBand()
{
	return noiseBand;
}

void SetLookbackSec(int value)
{
    if (value<1) value = 1;
	
	if(value<25)
	{
		nLookBack = value * 4;
		sampleTime = 250;
	}
	else
	{
		nLookBack = 100;
		sampleTime = value*10;
	}
}

int GetLookbackSec()
{
	return nLookBack * sampleTime / 1000;
}
