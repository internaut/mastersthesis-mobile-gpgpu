#include "detect_base.h"

#include <time.h>
#include <set>

#include "../tools.h"

static unsigned char possibleBitcodes[4][5] = {
	{ 1,0,0,0,0 },
	{ 1,0,1,1,1 },
	{ 0,1,0,0,1 },
	{ 0,1,1,1,0 }
};

static void printBitMatrix(cv::Mat m) {
	for (int i = 0; i < CV_ACCAR_MARKERCODE_CELLS - 2; i++) {
		LOGINFO("%d %d %d %d %d", (int)m.at<uchar>(i, 0), (int)m.at<uchar>(i, 1),
								  (int)m.at<uchar>(i, 2), (int)m.at<uchar>(i, 3),
								  (int)m.at<uchar>(i, 4));

	}
}

CvAccARDetectBase::CvAccARDetectBase(CvAccARCam *useCam) {
#ifdef BENCHMARK
	resetBenchmarks();
#endif

	cam = useCam;
	outFrame = NULL;
	procFrameOutputLevel = CV_ACCAR_PROC_LEVEL_NONE;
	minContourPointsAllowed = 4;
	minContourLengthAllowed = CvAccARConf::detectMinCountourLength * CvAccARConf::detectMinCountourLength;	// needs to be squared!

	// normalized 2D marker coordinates
	// in anti-clockwise order:
	const int s = CvAccARConf::detectMarkerPxSize;
	markerCellSize = s / CV_ACCAR_MARKERCODE_CELLS; // 2 are for black border
	minSetMarkerPixels = markerCellSize * markerCellSize / 2;

	markerSizePx = cv::Size(s, s);

	LOGINFO("CvAccARDetectBase: Marker size in px is %d, %d", markerSizePx.width, markerSizePx.height);
	LOGINFO("CvAccARDetectBase: Marker code cell number: %d", CV_ACCAR_MARKERCODE_CELLS);
	LOGINFO("CvAccARDetectBase: Marker cell size is: %d", markerCellSize);
	LOGINFO("CvAccARDetectBase: Min. marker set pixels is: %d", minSetMarkerPixels);

	normalizedMarkerCoord.push_back(cv::Point2f(0, 0));
	normalizedMarkerCoord.push_back(cv::Point2f(s - 1, 0));
	normalizedMarkerCoord.push_back(cv::Point2f(s - 1, s - 1));
	normalizedMarkerCoord.push_back(cv::Point2f(0, s - 1));

	const float sm = CvAccARConf::markerSizeMeters;	// size in meters
	normalizedMarkerCoord3D.push_back(cv::Point3f(-0.5f * sm,  0.5f * sm, 0.0f));
	normalizedMarkerCoord3D.push_back(cv::Point3f( 0.5f * sm,  0.5f * sm, 0.0f));
	normalizedMarkerCoord3D.push_back(cv::Point3f( 0.5f * sm, -0.5f * sm, 0.0f));
	normalizedMarkerCoord3D.push_back(cv::Point3f(-0.5f * sm, -0.5f * sm, 0.0f));
}

CvAccARDetectBase::~CvAccARDetectBase() {
	if (outFrame) {
		delete outFrame;
	}
}

void CvAccARDetectBase::camIsInitialized() {
	// because of downsampling:
	origFrameSize = cam->getFrameSize();

	if (CvAccARConf::detectMakePyrDown > 0) {
		procFrameSize = cv::Size(origFrameSize.width / 2, origFrameSize.height / 2);
	} else {
		procFrameSize = origFrameSize;
	}

	LOGINFO("CvAccARDetectBase: Proc frame size is %d, %d", procFrameSize.width, procFrameSize.height);
}

void CvAccARDetectBase::setProcFrameOutputLevel(int level) {
	if (procFrameOutputLevel == level) return;	// nothing changed

	LOGINFO("CvAccARDetectBase: proc frame output level set to %d", procFrameOutputLevel);
	procFrameOutputLevel = level;

	if (outFrame) {
		delete outFrame;
		outFrame = NULL;
	}

	// create a new processing output frame if necessary
	if (procFrameOutputLevel > CV_ACCAR_PROC_LEVEL_NONE && !outFrame) {
		outFrame = new cv::Mat(inpFrame.rows, inpFrame.cols, CV_8UC1);
		LOGINFO("CvAccARDetectBase: created new proc output frame of size %dx%d and %d channels",
				outFrame->cols, outFrame->rows, outFrame->channels());
	}
}

void CvAccARDetectBase::setInputFrame(cv::Mat *frame) {
	inpFrameRef = frame;
}

void CvAccARDetectBase::processFrame() {
//	LOGINFO("CvAccARDetect: processing frame without GPU accel.");

	// 1. preproc.

#ifdef BENCHMARK
	clock_t t1 = clock();
#endif

	// make a copy of the frame because we'll modify it
	curFrame = inpFrameRef->clone();
	// preprocess
	preprocess();

#ifdef BENCHMARK
	clock_t t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_PREPROC] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 2. adapt. thresh.

#ifdef BENCHMARK
	t1 = clock();
#endif

	performThreshold();

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_THRESH] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 3. find contours

#ifdef BENCHMARK
	t1 = clock();
#endif

	findContours();

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_CONTOURS] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 4. approx. contours, find marker candidates

#ifdef BENCHMARK
	t1 = clock();
#endif

	findMarkerCandidates();

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_CANDIDATES] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 5. detect markers

#ifdef BENCHMARK
	t1 = clock();
#endif

	detectMarkers(inpFrame);

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_MARKERCODE] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 6. estimate positions

#ifdef BENCHMARK
	t1 = clock();
#endif

	estimatePositions();

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_THRESH_POST] += CvAccARTools::getMsFromClocks(t1, t2);	// because CV_ACCAR_PROC_LEVEL_THRESH_POST is unused
#endif

	// benchmarking stuff

#ifdef BENCHMARK
	benchmarkRun++;
	if (benchmarkRun >= CvAccARConf::benchmarkNumRuns) {
		printBenchmarkResults();
		resetBenchmarks();
	}
#endif
}

void CvAccARDetectBase::preprocess() {
	// convert to grayscale
	if (curFrame.channels() > 1) {
		cv::cvtColor(curFrame, curFrame, CV_RGBA2GRAY);
	}

	// downsample to half size
	if (CvAccARConf::detectMakePyrDown > 0) {
		cv::pyrDown(curFrame, inpFrame);
	} else {
		curFrame.copyTo(inpFrame);	// do not downsample!
	}

	checkAndSetOutputFrame(CV_ACCAR_PROC_LEVEL_PREPROC);
}

void CvAccARDetectBase::performThreshold() {
	cv::adaptiveThreshold(inpFrame,
			curFrame,
			255,
			cv::ADAPTIVE_THRESH_MEAN_C, // ADAPTIVE_THRESH_GAUSSIAN_C
			cv::THRESH_BINARY_INV,
			CvAccARConf::detectThreshBlockSize,
			CvAccARConf::detectThreshC);
	checkAndSetOutputFrame(CV_ACCAR_PROC_LEVEL_THRESH);
}

void CvAccARDetectBase::threshPostProc() {
	// unused
	// non max-suppression necessary?
	checkAndSetOutputFrame(CV_ACCAR_PROC_LEVEL_THRESH_POST);
}

void CvAccARDetectBase::findContours() {
	// find contours
	ContourVec allContours;
	cv::findContours(curFrame, allContours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE); // CV_RETR_LIST or CV_RETR_EXTERNAL

#ifdef BENCHMARK
	numContourPoints = allContours.size();
#endif

	// filter out contours consisting of
	// less than <minContourPointsAllowed> points
	curContours.clear();
	for (ContourVec::iterator it = allContours.begin();
		it != allContours.end();
		++it)
	{
		if (it->size() >= minContourPointsAllowed) {
			curContours.push_back(*it);
		}
	}

	// draw contours if necessary
	if (outFrame && procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_CONTOURS) {
//		LOGINFO("Num. contours: %d", curContours.size());
		outFrame->setTo(cv::Scalar(0, 0, 0, 255));	// clear: fill black
		cv::drawContours(*outFrame, curContours, -1, cv::Scalar(255, 255, 255, 255));
	}
}

void CvAccARDetectBase::findMarkerCandidates() {
	possibleMarkers.clear();
	PointVec  approxCurve;

	for (ContourVec::iterator it = curContours.begin();
		it != curContours.end();
		++it)
	{
		PointVec contour = *it;
		// Approximate to a polygon
		float eps = contour.size() * 0.05f;
		cv::approxPolyDP(contour, approxCurve, eps, true);

		// We interested only in polygons that contains only four points
		if (approxCurve.size() != 4 || !cv::isContourConvex(approxCurve)) continue;

		// Ensure that the distance between consecutive points is large enough
		float minDist = numeric_limits<float>::max();
		for (int i = 0; i < 4; i++) {
			cv::Point side = approxCurve[i] - approxCurve[(i+1)%4];
			float squaredSideLength = side.dot(side);
			minDist = min(minDist, squaredSideLength);
		}

		if (minDist < minContourLengthAllowed) continue;

		// Create new marker candidate
		// Fill it with the points of the curve
		// CvAccARMarker will also sort the points in anti-clockwise order
		CvAccARMarker markerCand(approxCurve);

		// Add the marker candidate
		possibleMarkers.push_back(markerCand);
	}

#ifdef BENCHMARK
	numPossMarkers = possibleMarkers.size();
#endif

//	LOGINFO("CvAccARDetectBase: Num. of possible markers: %d", possibleMarkers.size());

	// TODO: do we have to filter out some markers?

	// set as current markers
//	curMarkers.clear();
//	curMarkers.assign(possibleMarkers.begin(), possibleMarkers.end());

	// draw markers if necessary
	if (outFrame && procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_CANDIDATES) {
		inpFrame.copyTo(*outFrame);
//		outFrame->setTo(cv::Scalar(0, 0, 0, 255));	// clear: fill black

		// draw each marker candidate
		for (vector<CvAccARMarker>::iterator it = possibleMarkers.begin();
			it != possibleMarkers.end();
			++it)
		{
			drawMarker(*outFrame, *it);
		}
	}
}

void CvAccARDetectBase::detectMarkers(const cv::Mat &srcImg) {
	if (outFrame && procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_MARKERCODE) {
//		outFrame->setTo(cv::Scalar(0, 0, 0, 255));	// clear: fill black
		srcImg.copyTo(*outFrame);
	}

	detectedMarkers.clear();
	for (vector<CvAccARMarker>::iterator it = possibleMarkers.begin();
		it != possibleMarkers.end();
		++it)
	{
		cv::Mat normMarkerImg(CvAccARConf::detectMarkerPxSize, CvAccARConf::detectMarkerPxSize, CV_8UC1);

		// Find the perspective transformation that brings current marker to
		// rectangular form
		const cv::Mat perspMat = cv::getPerspectiveTransform(it->getPoints(), normalizedMarkerCoord);
		cv::warpPerspective(srcImg, normMarkerImg,  perspMat, markerSizePx, cv::INTER_NEAREST);
		cv::threshold(normMarkerImg, normMarkerImg, 125, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		// try to read the marker code
		int rot;
		int readId = readMarkerCode(normMarkerImg, &rot);

//		LOGINFO("CvAccARDetectBase: read marker id: %d", readId);

		if (readId < 0) {	// code could not be read. filter out this marker
			continue;
		}

		// set the id and add this marker to the detected markers
//		LOGINFO("CvAccARDetectBase: marker id %d at rotation %d", readId, rot);
		it->setId(readId);
		if (rot != 0) {
			it->rotatePoints(rot);
		}
		detectedMarkers.insert(pair<int, CvAccARMarker>(readId, *it));

		// refine corners
		if (CvAccARConf::detectRefineCornersIter > 0) {
			cv::cornerSubPix(srcImg, it->getPoints(),
							 cv::Size(5, 5), cv::Size(-1,-1),
							 cv::TermCriteria(CV_TERMCRIT_ITER, CvAccARConf::detectRefineCornersIter, 0.1f));	// max. iterations, min. epsilon
		}

		if (outFrame && procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_MARKERCODE) {
			float r = it->getPerimeterRadius();
			cv::Point o = it->getCentroid() - (0.5f * cv::Point2f(r, r));
			LOGINFO("CvAccARDetectBase: drawing marker with id %d at pos %d, %d", it->getId(), o.x, o.y);
			cv::Rect roi(o, normMarkerImg.size());
			cv::rectangle(*outFrame, roi, cv::Scalar(255,255,255,255));
			cv::Mat dstMat = (*outFrame)(roi);
			normMarkerImg.copyTo(dstMat);

			drawMarker(*outFrame, *it);

//			LOGINFO("pers. mat.:");
//			for (int i = 0; i < perspMat.rows; i++) {
//				LOGINFO("%f\t%f\t%f", perspMat.at<float>(i, 0), perspMat.at<float>(i, 1), perspMat.at<float>(i, 2));
//			}
		}
	}

	// filter out duplicates
	discardDuplicateMarkers();
}

void CvAccARDetectBase::discardDuplicateMarkers() {
	set<int> idsToRemove;
	for (map<int, CvAccARMarker>::iterator it = detectedMarkers.begin();
		it != detectedMarkers.end();
		++it)
	{
		if (idsToRemove.find(it->first) != idsToRemove.end()) continue;

		for (map<int, CvAccARMarker>::iterator other = detectedMarkers.begin();
			other != detectedMarkers.end();
			++other)
		{
			if (it->first == other->first) continue;

			const float dist = CvAccARTools::distSquared(it->second.getCentroid(), other->second.getCentroid());

//			LOGINFO("CvAccARDetectBase: dist between marker %d and marker %d: %f", it->second.getId(), other->second.getId(), dist);

			if (dist < 25.0f
			 && it->second.getPerimeterRadius() >= other->second.getPerimeterRadius()) {
//				LOGINFO("CvAccARDetectBase: will remove marker %d", other->second.getId());
				idsToRemove.insert(other->second.getId());
			}
		}
	}

	for (set<int>::iterator it = idsToRemove.begin();
		 it != idsToRemove.end();
		 ++it)
	{
		detectedMarkers.erase(*it);
	}
}

void CvAccARDetectBase::estimatePositions() {
#ifdef BENCHMARK
	numDetectedMarkers = detectedMarkers.size();
#endif
//	LOGINFO("CvAccARDetectBase: num. detected markers: %d", numDetectedMarkers);

	const cv::Mat &intr = cam->getIntrinsics();
	Point2fVec dist;	// empty. no distortion
	for (map<int, CvAccARMarker>::iterator it = detectedMarkers.begin();
		it != detectedMarkers.end();
		++it)
	{
		CvAccARMarker &marker = it->second;

		// rVec and tVec are needed for solvePnP temporary result.
		// it will be saved as vector of double values.
		// these values will be converted to float type in CvAccARMarker::updatePoseMat
		cv::Mat rVec;
		cv::Mat tVec;
		cv::solvePnP(normalizedMarkerCoord3D, marker.getPoints(),
					 intr, dist,
					 rVec, tVec,
					 false);	// TODO: Test true?
//		LOGINFO("rVec: %.4e %.4e %.4e", rVec.at<double>(0), rVec.at<double>(1), rVec.at<double>(2));
//		LOGINFO("tVec: %.4e %.4e %.4e", tVec.at<double>(0), tVec.at<double>(1), tVec.at<double>(2));
//		printFloatMat("rVec", it->getRVec());
//		printFloatMat("tVec", it->getRVec());
		marker.updatePoseMat(rVec, tVec);
//		printFloatMat("poseMat", it->getPoseMat());
	}
}

int CvAccARDetectBase::readMarkerCode(cv::Mat &img, int *validRot) {
    cv::Mat bitMatrix = cv::Mat::zeros(CV_ACCAR_MARKERCODE_CELLS - 2, CV_ACCAR_MARKERCODE_CELLS - 2,CV_8UC1);

    for (int y = 0; y < CV_ACCAR_MARKERCODE_CELLS; y++) {
    	for (int x = 0; x < CV_ACCAR_MARKERCODE_CELLS; x++) {
    		int cellX = x * markerCellSize;
    		int cellY = y * markerCellSize;
    		cv::Mat cell = img(cv::Rect(cellX, cellY, markerCellSize, markerCellSize));

    		int nonZ = cv::countNonZero(cell);

    		// Check if we are in a border cell
    		bool isBorder = (y == 0 || y == CV_ACCAR_MARKERCODE_CELLS - 1
    		              || x == 0 || x == CV_ACCAR_MARKERCODE_CELLS - 1);

//    		LOGINFO("Cell %d,%d (border: %d) non-0 pixels: %d", cellX, cellY, isBorder, nonZ);

    		// Check number of non zero pixels
    		if (nonZ > minSetMarkerPixels) {
    			if (isBorder) {	// border must be black!
    				return -1;
    			} else {	// set to "1" for this cell
    				bitMatrix.at<uchar>(x - 1, y - 1) = 1;
    			}
    		}
    	}
    }

//    printBitMatrix(bitMatrix);
    cv::Mat bitMatrixT;
    cv::transpose(bitMatrix, bitMatrixT);
//    LOGINFO("transposed:");
//    printBitMatrix(bitMatrixT);

    cv::Mat *curBitMat;

    for (int rot = 0; rot < 4; rot++) {
    	int dir = -2 * (rot % 2) + 1; // dir is -1 or 1
    	if (rot < 2) {
//    		LOGINFO("Using normal matrix with direction %d", dir);
    		curBitMat = &bitMatrix;
    	} else {
//    		LOGINFO("Using transposed matrix with direction %d", dir);
    		curBitMat = &bitMatrixT;
    	}

    	if (checkMarkerCode(*curBitMat, dir)) {
//    		LOGINFO("CvAccARDetectBase: Found correct marker code!");
    		*validRot = rot;
    		return markerCodeToId(*curBitMat, dir);
    	}
    }

//    LOGINFO("CvAccARDetectBase: Incorrect marker code!");

    return -1;
}

bool CvAccARDetectBase::checkMarkerCode(const cv::Mat &m, int dir) const {
	// set start index depending on reading direction
	int start = (dir > 0) ? 0 : m.cols - 1;

	// go through all bitcode rows in the read matrix
	for (int r = 0; r < m.rows; r++) {
		// select read code row
		const unsigned char *readCode = m.ptr<unsigned char>(r);

		bool foundCode = false;

		// go through all possible bitcodes
		for (int p = 0; p < 4; p++) {
			// select possible code row
			const unsigned char *testCode = possibleBitcodes[p];

			// go through all bits in the row depending on direction
			bool nextBit = true;
			bool invalidBit = false;
			int i = start;
			int j = 0;
			while (nextBit) {
				if (readCode[i] != testCode[j]) {	// invalid bit found!
					invalidBit = true;
					break;
				}

				i += dir;
				if (dir > 0) {
					j = i;
					nextBit = (i < m.cols);
				} else {
					j = start - i;
					nextBit = (i > 0);
				}
			}

			if (!invalidBit) {	// this bitcode row is valid!
				foundCode = true;	// so check the next row
				break;
			}
		}

		if (!foundCode) {
			return false;
		}
	}

	return true;
}

int CvAccARDetectBase::markerCodeToId(const cv::Mat &m, int dir) const {
	int id = 0;

	// set start index depending on reading direction
	int start = (dir > 0) ? 0 : m.cols - 1;

	// go through all bitcode rows in the read matrix
	int r = start;
	unsigned char u, v;
	bool nextRow = true;
	while (nextRow) {
//		cv::Mat row = m.row(r);
//		if (dir < 0) {
//			cv::flip(row, row, 1);	// flip around because we read it in reverse order
//		}
		const unsigned char *row =  m.ptr<unsigned char>(r);

		if (dir > 0) {
			u = row[1];
			v = row[3];
		} else {	// reverse order
			u = row[3];
			v = row[1];
		}

		// code from ARuCo:
		id <<= 1;
        if (u) id |= 1;
		id <<= 1;
        if (v) id |= 1;

        // next row
		r += dir;
		nextRow = (dir > 0) ? (r < m.cols) : (r > 0);
	}

	return id;
}

void CvAccARDetectBase::checkAndSetOutputFrame(int reqLevel, cv::Mat *srcFrame) {
	if (outFrame && procFrameOutputLevel == reqLevel) {
		if (srcFrame == NULL) srcFrame = &curFrame;

		srcFrame->copyTo(*outFrame);
	}
}

void CvAccARDetectBase::drawMarker(cv::Mat &img, const CvAccARMarker &m) {
	// draw outline
	Point2fVec markerPts = m.getPoints();
	const int numPts = markerPts.size();
	cv::Scalar white(255, 255, 255, 255);
	for (int i = 0; i < numPts; i++) {
		cv::line(img, markerPts[i], markerPts[(i + 1) % numPts], white);
	}

	// draw centroid
	cv::Scalar green(0, 255, 0, 255);
	cv::Point cross1(2, 2);
	cv::Point cross2(2, -2);
	cv::Point c = m.getCentroid();
	cv::line(img, c - cross1, c + cross1, green);
	cv::line(img, c + cross2, c - cross2, green);

	// draw perimeter
	cv::Scalar blue(0, 0, 255, 255);
	cv::circle(img, c, m.getPerimeterRadius(), blue);
}

#ifdef BENCHMARK

void CvAccARDetectBase::resetBenchmarks() {
	benchmarkRun = 0;
	numContourPoints = 0;
	numPossMarkers = 0;
	numDetectedMarkers = 0;
	memset(execTimesMsSum, 0, sizeof(float) * 7);	// init with zero
}

void CvAccARDetectBase::printBenchmarkResults() {
	const float numTests = CvAccARConf::benchmarkNumRuns;

	float preProcMs 		= execTimesMsSum[CV_ACCAR_PROC_LEVEL_PREPROC] / numTests;
	float threshMs 			= execTimesMsSum[CV_ACCAR_PROC_LEVEL_THRESH] / numTests;
	float contoursMs 		= execTimesMsSum[CV_ACCAR_PROC_LEVEL_CONTOURS] / numTests;
	float approxAndFilterMs = execTimesMsSum[CV_ACCAR_PROC_LEVEL_CANDIDATES] / numTests;
	float identMarkersMs 	= execTimesMsSum[CV_ACCAR_PROC_LEVEL_MARKERCODE] / numTests;
	float estPoseMs 		= execTimesMsSum[CV_ACCAR_PROC_LEVEL_THRESH_POST] / numTests;

	LOGINFO("CvAccARDetectBase: benchmark result: %d, %d, %d, %f, %f, %f, %f, %f, %f",
			numDetectedMarkers, numContourPoints, numPossMarkers,
			preProcMs, threshMs, contoursMs,
			approxAndFilterMs, identMarkersMs, estPoseMs);
}

#endif
