#include "cam.h"

#include "tools.h"

/* BEGIN code from ArUco lib */

static float norm( float a, float b, float c )
{
    return( sqrt( a*a + b*b + c*c ) );
}

static float dot( float a1, float a2, float a3,
                              float b1, float b2, float b3 )
{
    return( a1 * b1 + a2 * b2 + a3 * b3 );
}

static int arParamDecompMat( float source[3][4], float cpara[3][4], float trans[3][4] )
{
    int       r, c;
    float    Cpara[3][4];
    float    rem1, rem2, rem3;

    if ( source[2][3] >= 0 )
    {
        for ( r = 0; r < 3; r++ )
        {
            for ( c = 0; c < 4; c++ )
            {
                Cpara[r][c] = source[r][c];
            }
        }
    }
    else
    {
        for ( r = 0; r < 3; r++ )
        {
            for ( c = 0; c < 4; c++ )
            {
                Cpara[r][c] = -(source[r][c]);
            }
        }
    }

    for ( r = 0; r < 3; r++ )
    {
        for ( c = 0; c < 4; c++ )
        {
            cpara[r][c] = 0.0;
        }
    }
    cpara[2][2] = norm( Cpara[2][0], Cpara[2][1], Cpara[2][2] );
    trans[2][0] = Cpara[2][0] / cpara[2][2];
    trans[2][1] = Cpara[2][1] / cpara[2][2];
    trans[2][2] = Cpara[2][2] / cpara[2][2];
    trans[2][3] = Cpara[2][3] / cpara[2][2];

    cpara[1][2] = dot( trans[2][0], trans[2][1], trans[2][2],
                       Cpara[1][0], Cpara[1][1], Cpara[1][2] );
    rem1 = Cpara[1][0] - cpara[1][2] * trans[2][0];
    rem2 = Cpara[1][1] - cpara[1][2] * trans[2][1];
    rem3 = Cpara[1][2] - cpara[1][2] * trans[2][2];
    cpara[1][1] = norm( rem1, rem2, rem3 );
    trans[1][0] = rem1 / cpara[1][1];
    trans[1][1] = rem2 / cpara[1][1];
    trans[1][2] = rem3 / cpara[1][1];

    cpara[0][2] = dot( trans[2][0], trans[2][1], trans[2][2],
                       Cpara[0][0], Cpara[0][1], Cpara[0][2] );
    cpara[0][1] = dot( trans[1][0], trans[1][1], trans[1][2],
                       Cpara[0][0], Cpara[0][1], Cpara[0][2] );
    rem1 = Cpara[0][0] - cpara[0][1]*trans[1][0] - cpara[0][2]*trans[2][0];
    rem2 = Cpara[0][1] - cpara[0][1]*trans[1][1] - cpara[0][2]*trans[2][1];
    rem3 = Cpara[0][2] - cpara[0][1]*trans[1][2] - cpara[0][2]*trans[2][2];
    cpara[0][0] = norm( rem1, rem2, rem3 );
    trans[0][0] = rem1 / cpara[0][0];
    trans[0][1] = rem2 / cpara[0][0];
    trans[0][2] = rem3 / cpara[0][0];

    trans[1][3] = (Cpara[1][3] - cpara[1][2]*trans[2][3]) / cpara[1][1];
    trans[0][3] = (Cpara[0][3] - cpara[0][1]*trans[1][3]
                   - cpara[0][2]*trans[2][3]) / cpara[0][0];

    for ( r = 0; r < 3; r++ )
    {
        for ( c = 0; c < 3; c++ )
        {
            cpara[r][c] /= cpara[2][2];
        }
    }

    return 0;
}
/* END code from ArUco lib */

CvAccARCam::CvAccARCam(int camId) {
	id = camId;
	curFrame = NULL;
	curFrameGray = NULL;
	cap = NULL;
	projNear = 0.01f;
	projFar = 10.0f;
}

CvAccARCam::~CvAccARCam() {
	stop();

	if (curFrame) {
		delete curFrame;
	}

	if (curFrameGray) {
		delete curFrameGray;
	}
}

bool CvAccARCam::start() {
	if (cap) return true;	// already started!

	// check if we have a "real" cam
	if (id >= 0) {
		return initCamCapture();	// then start it!
	}

	// no video capture but a still image for debugging
	LOGINFO("CvAccARCam: No VideoCapture created");
	cap = NULL;

	return true;	// this is ok, we have a still image for debugging
}

bool CvAccARCam::stop() {
	if (!cap) return true;	// already stopped

	cap->release();
	delete cap;
	cap = NULL;

	LOGINFO("CvAccARCam: VideoCapture stopped and released");

	return true;
}

void CvAccARCam::setDbgStillImage(const cv::Mat &img) {
	dbgStillImage = img.clone();
	setFrameSize(img.cols, img.rows);
}

void CvAccARCam::update() {
//	CvAccARTools::t1 = clock();
	if (id < 0 && dbgStillImage.rows > 0) {	// just set the still image as current frame
		curFrame = &dbgStillImage;
	} else if (cap && cap->grab()) {	// try to get a new frame from the camera
		cap->retrieve(*curFrame, CV_CAP_ANDROID_COLOR_FRAME_RGBA);

		if (CvAccARConf::camGrayscaleOutput) {
			cap->retrieve(*curFrameGray, CV_CAP_ANDROID_GREY_FRAME);
		}
	}
//	CvAccARTools::t2 = clock();
//	CvAccARTools::printExecTime("CvAccARCam: update");
}

bool CvAccARCam::initCamCapture() {
	cap = new cv::VideoCapture(id);

	// fails:
//	int w = (int)cap->get(CV_CAP_PROP_FRAME_WIDTH);
//	int h = (int)cap->get(CV_CAP_PROP_FRAME_HEIGHT);

//	curFrame = new cv::Mat(h, w, CV_32F);
	curFrame = new cv::Mat();

	if (CvAccARConf::camGrayscaleOutput) {
		curFrameGray = new cv::Mat();
	}

	if (CvAccARConf::requestedCamFrameW > 0 && CvAccARConf::requestedCamFrameH > 0) {
		cap->set(CV_CAP_PROP_FRAME_WIDTH, CvAccARConf::requestedCamFrameW);
		cap->set(CV_CAP_PROP_FRAME_HEIGHT, CvAccARConf::requestedCamFrameH);
	}

	if (!cap->grab()) {
		LOGERR("CvAccARCam: Could not grab first frame.");
		return false;
	}

	if (!cap->retrieve(*curFrame, CV_CAP_ANDROID_COLOR_FRAME_RGBA)) {
		LOGERR("CvAccARCam: Could not retrieve first frame.");
		return false;
	}

	// set the camera input frame size
	int w = curFrame->cols;
	int h = curFrame->rows;
	setFrameSize(w, h);

	LOGINFO("CvAccARCam: Created VideoCapture with cam id %d and frame size %dx%d", id, w, h);

	return true;
}

void CvAccARCam::calcProjMat(float viewW, float viewH, float procFrameW, float procFrameH) {
	bool invert = false;

	// intrinsics mat contains doubles. we need floats
	cv::Mat intrFloats(3, 3, CV_32F);
	intrinsics.convertTo(intrFloats, CV_32F);

	// get cam parameters
	/* BEGIN modified code from ArUco lib */
    const float Ax = viewW / (float)procFrameW;
    const float Ay = viewH / (float)procFrameH;
	const float f_x = intrFloats.at<float>(0, 0) * Ax;	// Focal length in x axis
	const float f_y = intrFloats.at<float>(1, 1) * Ay;	// Focal length in y axis
	const float c_x = intrFloats.at<float>(0, 2) * Ax; 	// Camera primary point x
	const float c_y = intrFloats.at<float>(1, 2) * Ay;	// Camera primary point y

    float cparam[3][4] =
    {
        {
        		f_x,  0,  c_x,  0
        },
        {0,          f_y,  c_y, 0},
        {0,      0,      1,      0}
    };

	// log parameters
	LOGINFO("CvAccARCam: Calculating projection matrix with parameters:");
	LOGINFO("> f = %f,%f, c = %f,%f, view size = %fx%f, frame size = %fx%f, near/far = %f/%f",
			f_x, f_y,
			c_x, c_y,
			viewW, viewH,
			procFrameW, procFrameH,
			projNear, projFar);

	// calculate the projection matrix
	float *m = glm::value_ptr(pMat);
	memset(m, 0, sizeof(float) * 16);	// init with zero

    cparam[0][2] *= -1.0;
    cparam[1][2] *= -1.0;
    cparam[2][2] *= -1.0;

    float   icpara[3][4];
    float   trans[3][4];
    float   p[3][3], q[4][4];

    arParamDecompMat(cparam, icpara, trans);

    for (int i = 0; i < 3; i++ )
    {
        for (int j = 0; j < 3; j++ )
        {
            p[i][j] = icpara[i][j] / icpara[2][2];
        }
    }

    q[0][0] = (2.0 * p[0][0] / viewW);
    q[0][1] = (2.0 * p[0][1] / viewW);
    q[0][2] = ((2.0 * p[0][2] / viewW)  - 1.0);
    q[0][3] = 0.0;

    q[1][0] = 0.0;
    q[1][1] = (2.0 * p[1][1] / viewH);
    q[1][2] = ((2.0 * p[1][2] / viewH) - 1.0);
    q[1][3] = 0.0;

    q[2][0] = 0.0;
    q[2][1] = 0.0;
    q[2][2] = (projFar + projNear)/(projFar - projNear);
    q[2][3] = -2.0 * projFar * projNear / (projFar - projNear);

    q[3][0] = 0.0;
    q[3][1] = 0.0;
    q[3][2] = 1.0;
    q[3][3] = 0.0;

    for (int i = 0; i < 4; i++ )
    {
        for (int j = 0; j < 3; j++ )
        {
            m[i+j*4] = q[i][0] * trans[0][j]
                       + q[i][1] * trans[1][j]
                       + q[i][2] * trans[2][j];
        }
        m[i+3*4] = q[i][0] * trans[0][3]
                   + q[i][1] * trans[1][3]
                   + q[i][2] * trans[2][3]
                   + q[i][3];
    }

    if (!invert)
    {
        m[13]=-m[13] ;
        m[1]=-m[1];
        m[5]=-m[5];
        m[9]=-m[9];
    }

    /* END modified code from ArUco lib */

/*	p[0]  = -2.0f * f_x / viewW;
	p[5]  =  2.0f * f_y / viewH;
	p[2]  = -2.0f * c_x / viewW - 1.0f;
	p[6]  =  2.0f * c_y / viewH - 1.0f;
	p[10] = -(projFar + projNear) / (projFar - projNear);
	p[11] = -2.0f * projFar * projNear / (projFar - projNear);
	p[14] = -1.0f;*/


	CvAccARTools::printFloatMat("> projection matrix:", m, 4, 4);
}
