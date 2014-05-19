#ifndef CV_ACCAR_MARKER_H
#define CV_ACCAR_MARKER_H

#include <vector>

#include "../cv_accar.h"

using namespace std;

typedef vector<cv::Point> PointVec;
typedef vector<cv::Point2f> Point2fVec;

class CvAccARMarker {
public:
	CvAccARMarker(PointVec &pts);
	CvAccARMarker(Point2fVec &pts);

	void setId(int newId) { id = newId; };
	int getId() const { return id; };

	Point2fVec getPoints() const { return points; };
	void addPoint(cv::Point2f p) { points.push_back(p); };
	void clearPoints() { points.clear(); };
	void rotatePoints(int rot);

	cv::Point2f getCentroid() const { return centroid; };
	float getPerimeterRadius() const { return perimeterRad; };

	const cv::Mat &getRVec() const { return rVec; };
	const cv::Mat &getTVec() const { return tVec; };

	void updatePoseMat(const cv::Mat &r, const cv::Mat &t);

	const glm::mat4 &getPoseMat() const { return poseMat; };
//	const float *getPoseMatPtr() const { return poseMat.ptr<float>(); };

	void sortPoints();
	void calcShapeProperties();

private:
	void init();

	int id;

	cv::Mat rVec;
	cv::Mat tVec;
	glm::mat4 poseMat;	// 4x4 matrix with model-view-transformation

	Point2fVec points;
	cv::Point2f centroid;
	float perimeterRad; // circumcircle radius
};

#endif
