#include "detect_accel.h"

//#include "../pipeline/proc/hist.h"
#include "../pipeline/proc/thresh.h"
#include "../pipeline/proc/preproc.h"
//#include "../pipeline/proc/pclines.h"
#include "../pipeline/proc/markerwarp.h"
#include "../tools.h"

#define PL_PROC(lvl) (pipelineProcessors[(int)(lvl)])

CvAccARDetectAccel::CvAccARDetectAccel(CvAccARCam *useCam)
	: CvAccARDetectBase(useCam)
{
	outputTexId = 0;

	for (int i = 0; i < CV_ACCAR_PIPELINE_LEVELS; i++) {
		pipelineProcessors[i] = NULL;
	}
}

CvAccARDetectAccel::~CvAccARDetectAccel() {
	for (int i = 0; i < CV_ACCAR_PIPELINE_LEVELS; i++) {
		if (pipelineProcessors[i]) {
			delete pipelineProcessors[i];
		}
	}
}

bool CvAccARDetectAccel::setPipelineProcWithShader(CvAccARPipelineProcLevel pipelineLvl, CvAccARShader *shader) {
	CvAccARPipelineProcFBO *pipelineProc = NULL;
	const int lvlNum = (int)pipelineLvl;

	if (pipelineLvl == PREPROC) {
		CvAccARPipelineProcPreproc *preprocPipeline = new CvAccARPipelineProcPreproc(pipelineLvl);
		preprocPipeline->bindShader(shader);

		pipelineProc = preprocPipeline;
		LOGINFO("CvAccARDetectAccel: Added new pipeline processor for level PREPROC (%d)", lvlNum);
	} else if (pipelineLvl == ATHRESH_1 || pipelineLvl == ATHRESH_2 /* || pipelineLvl == THRESH*/) {
		CvAccARPipelineProcThresh *threshPipeline = new CvAccARPipelineProcThresh(pipelineLvl);
		threshPipeline->bindShader(shader);

		pipelineProc = threshPipeline;
		LOGINFO("CvAccARDetectAccel: Added new pipeline processor for level THRESH (%d)", lvlNum);
	} else if (pipelineLvl == MARKERWARP) {
		CvAccARPipelineProcMarkerWarp *markerWarpPipeline = new CvAccARPipelineProcMarkerWarp();
		markerWarpPipeline->bindShader(shader);

		pipelineProc = markerWarpPipeline;
		LOGINFO("CvAccARDetectAccel: Added new pipeline processor for level MARKERWARP (%d)", lvlNum);
	} /*else if (pipelineLvl == HIST) {
		CvAccARPipelineProcHist *histPipeline = new CvAccARPipelineProcHist();
		histPipeline->bindShader(shader);

		pipelineProc = histPipeline;
		LOGINFO("CvAccARDetectAccel: Added new pipeline processor for level HIST (%d)", lvlNum);
	}*//* else if (pipelineLvl == PCLINES) {
		CvAccARPipelineProcPCLines *pclinesPipeline = new CvAccARPipelineProcPCLines();
		pclinesPipeline->bindShader(shader);

		pipelineProc = pclinesPipeline;
		LOGINFO("CvAccARDetectAccel: Added new pipeline processor for level PCLINES (%d)", lvlNum);
	}*/

	if (pipelineProc != NULL) {
		pipelineProcessors[lvlNum] = pipelineProc;

		LOGINFO("CvAccARDetectAccel: Added pipeline processor of level %d", lvlNum);

		return true;
	} else {
		return false;
	}
}

void CvAccARDetectAccel::initPipeline(CvAccARFBOMgr *fboMgr) {
	LOGINFO("CvAccARDetectAccel: Running pipeline initializations");

	// bind FBOs
	for (int i = 0; i < CV_ACCAR_PIPELINE_LEVELS; i++) {
		pipelineProcessors[i]->setFBOMgr(fboMgr);
		pipelineProcessors[i]->bindFBO(fboMgr->getFBOPtr((CvAccARPipelineProcLevel)i));
	}

	// init all processors with frame size
	const int pyrDown = (CvAccARConf::detectMakePyrDown == 1) ? 2 : 1;
	int prevOutW = 0;
	int prevOutH = 0;
	for (int i = 0; i < CV_ACCAR_PIPELINE_LEVELS; i++) {
		if (i == 0) {
			pipelineProcessors[i]->initWithFrameSize(origFrameSize.width, origFrameSize.height, pyrDown);
			prevOutW = pipelineProcessors[i]->getOutFrameWidth();
			prevOutH = pipelineProcessors[i]->getOutFrameHeight();
		} else {
			pipelineProcessors[i]->initWithFrameSize(prevOutW, prevOutH, 1);
		}
	}

	// init individual processors
	PL_PROC(ATHRESH_1)->useTexture(PL_PROC(PREPROC)->getFBOTexId());	// chain thresh pass 1 to preproc output
	PL_PROC(ATHRESH_2)->useTexture(PL_PROC(ATHRESH_1)->getFBOTexId());	// chain thresh pass 2 to thresh pass 1 output
//	PL_PROC(PCLINES)->useTexture(PL_PROC(ATHRESH_2)->getFBOTexId());

/*	CvAccARPipelineProcHist *histProc = (CvAccARPipelineProcHist *)PL_PROC(HIST);
	histProc->setAreaSize(CvAccARConf::detectMarkerPxSize, CvAccARConf::detectMarkerPxSize);
	histProc->setNumSamplesPerAreaRow(8);*/

	// init frame memory
	inpFrame.create(procFrameSize.height, procFrameSize.width, CV_8UC4);	// rgba
	curFrame.create(procFrameSize.height, procFrameSize.width, CV_8UC1);	// gray
}

void CvAccARDetectAccel::processFrame() {
	outputTexId = 0;

	// 1. preproc. on GPU

#ifdef BENCHMARK
	clock_t t1 = clock();
#endif

	preprocessAccel();

#ifdef BENCHMARK
	clock_t t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_PREPROC] += initialCPUCopyMs + CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 2. adapt. thresh. on GPU

#ifdef BENCHMARK
	t1 = clock();
#endif

	performThresholdAccel();

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_THRESH] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 3. find contours on CPU

#ifdef BENCHMARK
	t1 = clock();
#endif

	findContours();

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_CONTOURS] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 4. approx. contours, find marker candidates on CPU

#ifdef BENCHMARK
	t1 = clock();
#endif

	findMarkerCandidates();

#ifdef BENCHMARK
	t2 = clock();
	execTimesMsSum[CV_ACCAR_PROC_LEVEL_CANDIDATES] += CvAccARTools::getMsFromClocks(t1, t2);
#endif

	// 5. detect markers partly on GPU

#ifdef BENCHMARK
	t1 = clock();
#endif

	detectMarkersAccel();

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

void CvAccARDetectAccel::preprocessAccel() {
	// set input texture
	PL_PROC(PREPROC)->useTexture(inputTexId);

	// render
	PL_PROC(PREPROC)->render();


	if (procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_PREPROC) {		// display output?
		outputTexId = PL_PROC(PREPROC)->getFBOTexId();
	}
}

void CvAccARDetectAccel::performThresholdAccel() {
	// render
	PL_PROC(ATHRESH_1)->render();
	PL_PROC(ATHRESH_2)->render();

	PL_PROC(ATHRESH_2)->getResultData(inpFrame.data);
	cv::cvtColor(inpFrame, curFrame, CV_RGBA2GRAY);

	// display output?
	if (procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_THRESH) {
		checkAndSetOutputFrame(CV_ACCAR_PROC_LEVEL_THRESH);
	}
}

void CvAccARDetectAccel::detectMarkersAccel() {
	// clear list of detected markers
	detectedMarkers.clear();

	// get number of possible markers
	const int numMarkers = possibleMarkers.size();

	if (numMarkers <= 0) return;

	// --------------
	// 1. marker warp
	// --------------

	// set input texture
	CvAccARPipelineProcMarkerWarp *markerWarpProc = (CvAccARPipelineProcMarkerWarp *)PL_PROC(MARKERWARP);

	if (outFrame && procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_MARKERCODE) {
		outFrame->setTo(cv::Scalar(0, 0, 0, 255));	// clear: fill black
//		curFrame.copyTo(*outFrame);
	}

	// prepare the marker warp processor
	markerWarpProc->useTexture(PL_PROC(PREPROC)->getFBOTexId());
	markerWarpProc->prepareForMarkers(numMarkers);

	// add the coordinates to the marker warp processor
	for (vector<CvAccARMarker>::iterator it = possibleMarkers.begin();
		it != possibleMarkers.end();
		++it)
	{
		markerWarpProc->addMarkerOriginCoords(it->getPoints());
	}

	// warp the markers
	markerWarpProc->render();

	const int markerWarpOutputW = markerWarpProc->getOutFrameWidth();
	const int markerWarpOutputH = markerWarpProc->getOutFrameHeight();

	cv::Mat *warpedMarkers = NULL;

	if (CvAccARConf::gpuAccelFixedMarkerWarp <= 0
	|| (CvAccARConf::gpuAccelFixedMarkerWarp > 0
	 && (markerWarpOutputH > CvAccARConf::detectMarkerPxSize || numMarkers * CvAccARConf::detectMarkerPxSize == markerWarpOutputW)))
	{
		// get the full output frame
		//LOGINFO("CvAccARDetectAccel: Getting full marker warp result frame");
		warpedMarkers = new cv::Mat(markerWarpOutputH, markerWarpOutputW, CV_8UC4);
		markerWarpProc->getResultData(warpedMarkers->data);
	} else {	// copy only a partial result
		//LOGINFO("CvAccARDetectAccel: Getting partial marker warp result frame");
		int resRectW = numMarkers * CvAccARConf::detectMarkerPxSize;
		warpedMarkers = new cv::Mat(CvAccARConf::detectMarkerPxSize, resRectW, CV_8UC4);
		markerWarpProc->getResultDataRect(warpedMarkers, cv::Rect(0, 0, resRectW, CvAccARConf::detectMarkerPxSize));
	}

	cv::cvtColor(*warpedMarkers, *warpedMarkers, CV_RGBA2GRAY);

	if (procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_MARKERCODE) {
		if (outFrame) delete outFrame;

		outFrame = new cv::Mat(warpedMarkers->rows, warpedMarkers->cols, CV_8UC1);
		warpedMarkers->copyTo(*outFrame);

		if (warpedMarkers) {
			delete warpedMarkers;
			warpedMarkers = NULL;
		}

		return;
	}


	// ---------------------------
	// 2. warped markers histogram
	// ---------------------------

	// go through the areas of possible markers, which are all normalized now
	int markersPerRow = warpedMarkers->cols / CvAccARConf::detectMarkerPxSize;
	int rowsOfMarkers = warpedMarkers->rows / CvAccARConf::detectMarkerPxSize;

	if (markersPerRow > numMarkers) markersPerRow = numMarkers;

//	LOGINFO("CvAccARDetectAccel: Warped markers frame of size %dx%d - %d rows, %d markers per row",
//			markerWarpOutputW, markerWarpOutputH, rowsOfMarkers, markersPerRow);

	for (int markerAreaY = 0; markerAreaY < rowsOfMarkers; markerAreaY++) {
		for (int markerAreaX = 0; markerAreaX < markersPerRow; markerAreaX++) {
			// get the current possible marker number (equals index in possibleMarkers - NOT the id)
			const int curMarkerNum = markerAreaY * markersPerRow + markerAreaX;
			if (curMarkerNum >= numMarkers) break;

			// select the ROI
			cv::Rect roi(markerAreaX * CvAccARConf::detectMarkerPxSize,
						 markerAreaY * CvAccARConf::detectMarkerPxSize,
						 CvAccARConf::detectMarkerPxSize, CvAccARConf::detectMarkerPxSize);

/*			LOGINFO("CvAccARDetectAccel: checking possible marker %d (%d, %d) at ROI %d, %d",
					curMarkerNum, markerAreaX, markerAreaY,
					markerAreaX * CvAccARConf::detectMarkerPxSize,
					markerAreaY * CvAccARConf::detectMarkerPxSize);*/

			// get the area
			cv::Mat markerArea(*warpedMarkers, roi);

			// threshold it
			cv::threshold(markerArea, markerArea, 125, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

			// try to read the marker code
//			LOGINFO("read marker bits:");
			int rot;
			int readId = readMarkerCode(markerArea, &rot);

			if (readId < 0) {	// code could not be read. filter out this marker
				continue;
			}

			// set the id and add this marker to the detected markers
//			LOGINFO("CvAccARDetectAccel: marker id %d at rotation %d", readId, rot);

			CvAccARMarker &detectedMarker = possibleMarkers[curMarkerNum];

			detectedMarker.setId(readId);
			if (rot != 0) {
				detectedMarker.rotatePoints(rot);
			}

			detectedMarkers.insert(pair<int, CvAccARMarker>(readId, detectedMarker));

			// refine corners
			if (CvAccARConf::detectRefineCornersIter > 0) {
				cv::cornerSubPix(inpFrame, detectedMarker.getPoints(),
								 cv::Size(5, 5), cv::Size(-1,-1),
								 cv::TermCriteria(CV_TERMCRIT_ITER, CvAccARConf::detectRefineCornersIter, 0.1f));	// max. iterations, min. epsilon
			}
		}
	}

	// cleanup
	if (warpedMarkers) {
		delete warpedMarkers;
		warpedMarkers = NULL;
	}

	// filter out duplicate markers
	discardDuplicateMarkers();

/*	CvAccARPipelineProcHist *histProc = (CvAccARPipelineProcHist *)PL_PROC(HIST);

	// set input texture
	int markerWarpTexId = markerWarpProc->getFBOTexId(); // to try reading from input texture set "1"
	LOGINFO("CvAccARDetectAccel: HIST uses tex id %d now", markerWarpTexId);
	histProc->useTexture(markerWarpTexId);	// because CvAccARPipelineProcMarkerWarp's tex id can change

	// run init again - maybe size changed
	histProc->initWithFrameSize(markerWarpProc->getNonPOTOutFrameW(), markerWarpProc->getNonPOTOutFrameH());

	// prepare for markers
	histProc->prepareForAreas(numMarkers);

	// perform histogram calculation
	histProc->render();

	if (procFrameOutputLevel == CV_ACCAR_PROC_LEVEL_MARKERCODE) {
		if (outFrame) delete outFrame;

		outFrame = new cv::Mat(histProc->getOutFrameHeight(), histProc->getOutFrameWidth(), CV_8UC4);
		histProc->getResultData(outFrame->data);

		cv::cvtColor(*outFrame, *outFrame, CV_RGBA2GRAY);

		return;
	}*/
}
