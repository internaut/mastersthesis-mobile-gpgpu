#ifndef CV_ACCAR_CONF_H
#define CV_ACCAR_CONF_H

class CvAccARConf {
public:
	static bool useGPUAccel;
	static int gpuAccelFixedMarkerWarp;

	static int benchmarkNumRuns;

	static int requestedCamFrameW;
	static int requestedCamFrameH;
	static bool camGrayscaleOutput;

	static float markerSizeMeters;

	static float detectMinCountourLength;
	static int detectMarkerPxSize;
	static int detectMakePyrDown;	// only "1" or "0" are valid values!
	static int detectThreshBlockSize;
	static double detectThreshC;
	static int detectRefineCornersIter;
};

#endif
