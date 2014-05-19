#ifndef CV_ACCAR_DETECT_BASE_H
#define CV_ACCAR_DETECT_BASE_H

#include <vector>
#include <map>

#include "../../cv_accar.h"
#include "../marker.h"
#include "../cam.h"

using namespace std;

#define CV_ACCAR_MARKERCODE_CELLS	7

#define CV_ACCAR_PROC_LEVEL_NONE 		0
#define CV_ACCAR_PROC_LEVEL_PREPROC 	1
#define CV_ACCAR_PROC_LEVEL_THRESH 		2
#define CV_ACCAR_PROC_LEVEL_THRESH_POST	3
#define CV_ACCAR_PROC_LEVEL_CONTOURS 	4
#define CV_ACCAR_PROC_LEVEL_CANDIDATES 	5
#define CV_ACCAR_PROC_LEVEL_MARKERCODE 	6

typedef vector<cv::Point> PointVec;
typedef vector<cv::Point2f> Point2fVec;
typedef vector<cv::Point3f> Point3fVec;
typedef vector<PointVec> ContourVec;

class CvAccARMarker;
class CvAccARCam;

class CvAccARDetectBase {
public:
	explicit CvAccARDetectBase(CvAccARCam *useCam);

	virtual ~CvAccARDetectBase();

	void camIsInitialized();

	virtual void setInputFrame(cv::Mat *frame);
	virtual void setInputFrame(unsigned int id) { /* not implemented */ };
	virtual unsigned int getOutputFrameHandle() { return 0; };

	virtual void processFrame();

	virtual cv::Mat *getProcFrameOutput() const { return outFrame; };

	void setProcFrameOutputLevel(int level);
	int getProcFrameOutputLevel() const { return procFrameOutputLevel; };

	cv::Size getProcFrameSize() const { return procFrameSize; };

	const map<int, CvAccARMarker> &getDetectedMarkers() const { return detectedMarkers; };

#ifdef BENCHMARK
	void setInitialGPUCopyMsForBenchmark(float ms) { initialCPUCopyMs = ms; };
#endif

protected:
	void preprocess();

	void performThreshold();

	void threshPostProc();

	void findContours();

	void findMarkerCandidates();

	void detectMarkers(const cv::Mat &srcImg);

	void discardDuplicateMarkers();

	void estimatePositions();

	void checkAndSetOutputFrame(int reqLevel, cv::Mat *srcFrame = NULL);

	int readMarkerCode(cv::Mat &img, int *validRot);

	bool checkMarkerCode(const cv::Mat &m, int dir) const;
	int markerCodeToId(const cv::Mat &m, int dir) const;

	void drawMarker(cv::Mat &img, const CvAccARMarker &m);

#ifdef BENCHMARK
	void printBenchmarkResults();
	void resetBenchmarks();
#endif

	CvAccARCam *cam;
	cv::Mat *inpFrameRef;	// weak ref.
	cv::Mat inpFrame;
	cv::Mat curFrame;
	cv::Mat *outFrame;	// pointer to output frame for debugging with procFrameOutputLevel
	ContourVec curContours;
	vector<CvAccARMarker> possibleMarkers;
	map<int, CvAccARMarker> detectedMarkers;	// maps marker id -> marker object

	Point2fVec normalizedMarkerCoord;	// standard coordinates for a normalized rectangular marker in 2D
	Point3fVec normalizedMarkerCoord3D;	// standard coordinates for a normalized rectangular marker in 3D

	cv::Size origFrameSize;	// size of original cam frame
	cv::Size procFrameSize;	// size of frame that is processed (usually downsampled version of input frame)

	int procFrameOutputLevel;
	int minContourPointsAllowed;
	float minContourLengthAllowed;
	float realWorldMarkerSize;	// marker size in meters
	cv::Size markerSizePx;
	int markerCellSize;
	int minSetMarkerPixels;

#ifdef BENCHMARK
	int benchmarkRun;
	int numContourPoints;
	int numPossMarkers;
	int numDetectedMarkers;
	float initialCPUCopyMs;
	float execTimesMsSum[7];
#endif
};

#endif
