package net.mkonrad.glimageprocnative;

import static android.opengl.GLES20.*;

import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import org.opencv.core.Mat;
import org.opencv.core.Point;

import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.util.Log;

public class GLRenderer implements Renderer {
	final private static String TAG = "GlImageProcNative::GLRenderer";
	
	private static final boolean extractPCLines = true;
	private static final int numTestRuns = 2;
	
	private long inputImgAddr = 0;
	private long outputImgAddr = 0;
	
	private MainActivity mainActivity;
	
	private int curTestRun;
	private float pushMsSum;
	private float execMsSum;
	private float pullMsSum;
	
	public GLRenderer(MainActivity main) {
		mainActivity = main;
		
		resetTestRuns();
	}
	
	@Override
	public void onDrawFrame(GL10 arg0) {
		if (inputImgAddr != 0) {
			imgproc.set_input_image(inputImgAddr);
		}
		
		imgproc.glview_draw();
		
		if (outputImgAddr != 0) {
			imgproc.get_output_image(outputImgAddr);
			if (extractPCLines && curTestRun == 1) {
				ArrayList<Point> extractedLineParams = extractLineParamsFromPCLinesImage(outputImgAddr);
				mainActivity.setExtractedLineParams(extractedLineParams);
			}
		}
		
		pushMsSum += imgproc.get_image_push_ms();
		execMsSum += imgproc.get_render_ms();
		pullMsSum += imgproc.get_image_pull_ms();
		
		curTestRun++;
		
		if (curTestRun > numTestRuns) {
			float pushMsAvg = pushMsSum / numTestRuns;
			float execMsAvg = execMsSum / numTestRuns;
			float pullMsAvg = pullMsSum / numTestRuns;
			
			mainActivity.testRunFinished(pushMsAvg, execMsAvg, pullMsAvg);
			resetTestRuns();
		}
	}

	@Override
	public void onSurfaceChanged(GL10 arg0, int w, int h) {
		imgproc.glview_resize(w, h);
	}

	@Override
	public void onSurfaceCreated(GL10 arg0, EGLConfig conf) {
		imgproc.glview_create();
	}

	public void setInputImageAddr(long nativeObjAddr) {
		inputImgAddr = nativeObjAddr;
	}

	public void setOutputImageAddr(long nativeObjAddr) {
		outputImgAddr = nativeObjAddr;
	}

	public void resetTestRuns() {
		curTestRun = 0;
		pushMsSum = 0.0f;
		execMsSum = 0.0f;
		pullMsSum = 0.0f;
	}
	
	private ArrayList<Point> extractLineParamsFromPCLinesImage(long imgAddr) {
		Mat img = new Mat(imgAddr);
		final float w = img.rows();
		final float h = img.cols();
		final float nearXThresh = 0.05f;
		Log.i(TAG, "Extracting lines from image of size " + (int)w + "x" + (int)h);
		
		ArrayList<Point> lineParams = new ArrayList<Point>();
		
		// This code follows the approach by Brad Larson
		// (see https://github.com/BradLarson/GPUImage/blob/master/framework/Source/GPUImageHoughTransformLineDetector.m)
		for (int y = 0; y < img.rows(); y++) {
			for (int x = 0; x < img.cols(); x++) {
				double px[] = img.get(y, x);
				if (px[0] > 0) {
					float normX = -1.0f + 2.0f * (float)x / w;
					float normY = -1.0f + 2.0f * (float)y / h;
					
					Point p = new Point();
					if (normX < 0.0f) {	// T space
						if (normX > -nearXThresh) {	// near X axis
							p.x = 100000.0;
							p.y = normY;
						} else {
							p.x = -1.0 - 1.0 / normX;
							p.y = normY / normX;
						}
					} else {			// S space
						if (normX < nearXThresh) {	// near X axis
							p.x = 100000.0;
							p.y = normY;
						} else {
							p.x = 1.0 - 1.0 / normX;
							p.y = normY / normX;
						}
					}
					
					lineParams.add(p);
				}
			}
		}
		
		return lineParams;
	}
}
