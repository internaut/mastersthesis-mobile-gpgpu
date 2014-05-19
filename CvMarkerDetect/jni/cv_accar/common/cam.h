#ifndef CV_ACCAR_CAM_H
#define CV_ACCAR_CAM_H

#include "../cv_accar.h"

class CvAccARCam {
public:
	CvAccARCam(int camId);
	~CvAccARCam();

	int getId() const { return id; };

	bool start();
	bool stop();

	void setIntrinsics(cv::Mat &intr) { intrinsics = intr.clone(); };
	const cv::Mat &getIntrinsics() const { return intrinsics; };

	void setDbgStillImage(const cv::Mat &img);

	cv::Size getFrameSize() const { return camFrameSize; };

	void update();
	cv::Mat *getCurFrame() const { return curFrame; };
	cv::Mat *getCurFrameGray() const { return curFrameGray; };

	const glm::mat4 &getProjMat() const { return pMat; };

	void calcProjMat(float viewW, float viewH, float procFrameW, float procFrameH);

private:
	bool initCamCapture();
	void setFrameSize(int w, int h) { camFrameSize = cv::Size(w, h); };

	int id;

	cv::Mat intrinsics;	// contains double values!

	float projNear;	// near clipping plane for projection matrix
	float projFar;	// far clipping plane for projection matrix
	glm::mat4 pMat;	// projection matrix

	cv::Size camFrameSize;
	cv::VideoCapture *cap;

	cv::Mat dbgStillImage;
	cv::Mat *curFrame;
	cv::Mat *curFrameGray;
};

#endif
