#include "tools.h"

clock_t CvAccARTools::t1 = 0;
clock_t CvAccARTools::t2 = 0;
double CvAccARTools::dTsum = 0.0;

void CvAccARTools::checkGLError(const char *msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		LOGERR("%s: GL error %d occurred", msg, err);
	}
}

void CvAccARTools::printFloatMat(const char *msg, const cv::Mat &m) {
	LOGINFO("Matrix %s:", msg);
	for (int r = 0; r < m.rows; r++) {
		char matRow[1024] = { '\0' };

		for (int c = 0; c < m.cols; c++) {
			char matCell[256];
			sprintf(matCell, "%.4e ", m.at<float>(r, c));
			strncat(matRow, matCell, 32);
		}

		LOGINFO("%s", matRow);
	}
}

void CvAccARTools::printFloatMat(const char *msg, const float *m, const int rows, const int cols) {
	LOGINFO("Matrix %s:", msg);
	for (int r = 0; r < rows; r++) {
		char matRow[1024] = { '\0' };

		for (int c = 0; c < cols; c++) {
			char matCell[256];
			sprintf(matCell, "%.4e ", m[r * rows + c]);
			strncat(matRow, matCell, 32);
		}

		LOGINFO("%s", matRow);
	}
}

void CvAccARTools::printDoubleMat(const char *msg, const cv::Mat &m) {
	LOGINFO("Matrix %s:", msg);
	for (int r = 0; r < m.rows; r++) {
		char matRow[1024] = { '\0' };

		for (int c = 0; c < m.cols; c++) {
			char matCell[256];
			sprintf(matCell, "%.4e ", m.at<double>(r, c));
			strncat(matRow, matCell, 32);
		}

		LOGINFO("%s", matRow);
	}
}

void CvAccARTools::printDoubleMat(const char *msg, const double *m, const int rows, const int cols) {
	LOGINFO("Matrix %s:", msg);
	for (int r = 0; r < rows; r++) {
		char matRow[1024] = { '\0' };

		for (int c = 0; c < cols; c++) {
			char matCell[256];
			sprintf(matCell, "%.4e ", m[r * rows + c]);
			strncat(matRow, matCell, 32);
		}

		LOGINFO("%s", matRow);
	}
}

void CvAccARTools::printExecTime(const char *msg) {
	double dt = ((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000.0;
	LOGINFO("%s - time: %.4f ms\n", msg, dt);
	dTsum += dt;
}

void CvAccARTools::printExecTimeSum(const char *msg) {
	LOGINFO("%s - time sum: %.4f ms\n", msg, dTsum);
	dTsum = 0.0;
}

float CvAccARTools::getMsFromClocks(clock_t t1, clock_t t2) {
	return (float)(((double)(t2 - t1) / CLOCKS_PER_SEC) * 1000.0);
}

int CvAccARTools::getNextPOT(int s, int minExp, int maxExp) {
	for (int exp = minExp; exp <= maxExp; exp++) {
		int pot = pow(2, exp);
		if (pot >= s) {
			return pot;
		}
	}

	// this should not happen:
	return -1;
}

float CvAccARTools::distSquared(cv::Point2f p1, cv::Point2f p2) {
	const float dX = p1.x - p2.x;
	const float dY = p1.y - p2.y;

	return dX * dX + dY * dY;
}
