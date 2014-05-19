#include "conf.h"

bool CvAccARConf::useGPUAccel 				= true;
int CvAccARConf::gpuAccelFixedMarkerWarp	= 1024;	// set to "0" to disable

int CvAccARConf::benchmarkNumRuns = 100;			// needs preproc. define "BENCHMARK"

int CvAccARConf::requestedCamFrameW 		= 1280;
int CvAccARConf::requestedCamFrameH 		= 720;
bool CvAccARConf::camGrayscaleOutput 		= false;

float CvAccARConf::markerSizeMeters 		= 0.04;

float CvAccARConf::detectMinCountourLength 	= 30.0f;
int CvAccARConf::detectMarkerPxSize 		= 64;	// must be power-of-two
//int CvAccARConf::detectMarkerPxSize 		= 56;	// must be power-of-two
int CvAccARConf::detectMakePyrDown			= 1;
int CvAccARConf::detectThreshBlockSize		= 5;
double CvAccARConf::detectThreshC			= 9.0;
int CvAccARConf::detectRefineCornersIter	= 0;
