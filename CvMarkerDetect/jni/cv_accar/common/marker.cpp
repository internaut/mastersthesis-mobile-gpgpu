#include "marker.h"

CvAccARMarker::CvAccARMarker(PointVec &pts) {
	for (int i = 0; i < 4; i++) {
		points.push_back(cv::Point2f(pts[i].x, pts[i].y));
	}

	init();
}

CvAccARMarker::CvAccARMarker(Point2fVec &pts) {
	points.assign(pts.begin(), pts.begin() + 4);

	init();
}

void CvAccARMarker::updatePoseMat(const cv::Mat &r, const cv::Mat &t) {
//	printFloatMat("input", poseMat);
	cv::Mat cvPoseMat;
	cvPoseMat.create(4, 4, CV_32F);
	cvPoseMat.eye(4, 4, CV_32F);

	// r and t are double vectors from solvePnP
	// convert them!
	r.convertTo(rVec, CV_32F);
	t.convertTo(tVec, CV_32F);

	cv::Mat rotMat(3, 3, CV_32FC1);
	cv::Rodrigues(rVec, rotMat);

	/* BEGIN modified code from ArUco lib */
    float para[3][4];
    for (int i=0; i < 3; i++) {
    	float *rotMatRow = rotMat.ptr<float>(i);
        for (int j = 0; j < 3; j++) {
        	para[i][j] = rotMatRow[j];
        }
    }
    //now, add the translation
    float *tVecData = tVec.ptr<float>(0);
    para[0][3] = tVecData[0];
    para[1][3] = tVecData[1];
    para[2][3] = tVecData[2];

    // create and init modelview_matrix
    float modelview_matrix[16];
    memset(modelview_matrix, 0, 16 * sizeof(float));	// init with zeros

    for (int i = 0; i < 3; i++) {
    	float sign = (i != 2) ? 1.0f : -1.0f;
    	for (int j = 0; j < 4; j++) {
    		modelview_matrix[i + j * 4] = sign * para[i][j];
    	}
    }

    modelview_matrix[15] = 1.0f;
    /* END modified code from ArUco lib */

    poseMat = glm::make_mat4(modelview_matrix);
//    poseMat = glm::translate(poseMat, glm::vec3(markerSize / 2.0f, markerSize / 2.0f, 0.0f));

/*
//	printFloatMat("rVec", rVec);
//	printFloatMat("tVec", tVec);

	// calc. rotation matrix from rot. vector
	// and update rotation component of 4x4 matrix
	cv::Mat rotMat = cvPoseMat(cv::Rect(0, 0, 3, 3));
	cv::Rodrigues(rVec, rotMat);
//	printFloatMat("rodrigues", rotMat);

	// update translation component of 4x4 matrix as R|T
	cv::Mat transMat = cvPoseMat(cv::Rect(3, 0, 1, 3));
	tVec.copyTo(transMat);
	cvPoseMat.at<float>(3, 3) = 1.0f;
//	printFloatMat("trans", poseMat);

//	printFloatMat("not inverted", poseMat);

	// calculate inverse
	cvPoseMat = cvPoseMat.inv();

	// convert to glm::mat
	//float *matData = cvPoseMat.ptr<float>();
	poseMat = glm::make_mat4(cvPoseMat.ptr<float>());*/
}

void CvAccARMarker::sortPoints() {
	// Sort the points in anti-clockwise order
	cv::Point v1 = points[1] - points[0];
	cv::Point v2 = points[2] - points[0];

	// if the third point is in the left side,
	// sort in anti-clockwise order
	if ((v1.x * v2.y) - (v1.y * v2.x) < 0.0) {
		swap(points[1], points[3]);
	}
}

void CvAccARMarker::rotatePoints(int rot) {
	rotate(points.begin(), points.begin() + 4 - rot, points.end());
}

void CvAccARMarker::calcShapeProperties() {
	centroid = 0.25f * (points[0] + points[1] + points[2] + points[3]);
	float maxDist = numeric_limits<float>::min();
	for (Point2fVec::iterator it = points.begin();
		 it != points.end();
		 ++it)
	{
		float d = cv::norm(centroid - *it);
		maxDist = max(maxDist, d);
	}

	perimeterRad = maxDist;
}

void CvAccARMarker::init() {
	rVec.create(3, 1, CV_32F);
	tVec.create(3, 1, CV_32F);

	sortPoints();
	calcShapeProperties();
}
