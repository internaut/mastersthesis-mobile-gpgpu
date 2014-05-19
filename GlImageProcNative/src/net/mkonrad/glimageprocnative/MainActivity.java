package net.mkonrad.glimageprocnative;


import java.util.ArrayList;

import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Scalar;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity
{
	final private static String TAG = "GlImageProcNative::MainActivity";
	private static final int OPERATION_TYPE_IMGCONV = 0;
	private static final int OPERATION_TYPE_HOUGH = 1;
	
	static {
	    if (!OpenCVLoader.initDebug()) {
	        Log.e(TAG, "Could not load OpenCV as static library");
	    } else {
	    	Log.i(TAG, "Loaded OpenCV as static library");
	    }
	}

	private static final int selectSingleImg = 2;					// select image resolution or use all image resolutions (set to < 0)
	private static final int operationType = OPERATION_TYPE_HOUGH;	// select image proc. operation
	private static final boolean loadBinImg = (operationType == OPERATION_TYPE_HOUGH);
	private static final int useKernel = 1;							// 0 is 3x3 gauss kernel, 1 is 5x5, 2 is 7x7
	
	private ArrayList<String> kernelNames = null;
	private String selectedKernelType;
	
	private ArrayList<Bitmap> bitmaps = null;
	private ArrayList<String> bitmapNames = null;
	
	private int curRunBmIndex = 0;
	
	private String benchmarkRes;
	
	private TextView infoTextView;
	private GLSurfaceView glView;
	private GLRenderer glRenderer;
	
	private Mat inputImgMat;
	private Mat outputImgMat;
	
	private ArrayList<Point> extractedLineParams = null;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        
        imgproc.init();
        		
		kernelNames = new ArrayList<String>();
		kernelNames.add("3x3");
		kernelNames.add("5x5");
		kernelNames.add("7x7");
		
		setupTestCases();
		
		resetBenchmark();
		
		initGLView();
		
		initMainView();
    }
    
	public void testRunFinished(float pushMsAvg, float execMsAvg, float pullMsAvg) {
		Log.i(TAG, "Test run finished");
		glView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		
		String bitmapName = (selectSingleImg < 0) ? bitmapNames.get(curRunBmIndex)
												  : bitmapNames.get(selectSingleImg);  
		
		if (operationType == OPERATION_TYPE_IMGCONV) {
			benchmarkRes += selectedKernelType + "," + bitmapName + "," +
					pushMsAvg + "," + execMsAvg + "," + pullMsAvg + "\n";
		} else if (operationType == OPERATION_TYPE_HOUGH) {
			benchmarkRes += bitmapName + "," + execMsAvg + "\n";
		}
		
		curRunBmIndex++;
		
		if (curRunBmIndex >= bitmaps.size()) {
			Log.i(TAG, "All test runs finished. Here are the results:");
			Log.i(TAG, benchmarkRes);
			resetBenchmark();
		} else {
			nextTestRun();
		}
	}
    
    @Override
    protected void onDestroy() {
    	imgproc.cleanup();
    	
    	super.onDestroy();
    }
    
    private void initMainView() {
    	setContentView(R.layout.main);
    	
		infoTextView = (TextView)findViewById(R.id.infotext);
		infoTextView.setText("Android UI - Input image");
		
		Bitmap inputBitmap = bitmaps.get(0);
 		initTextView(infoTextView, inputBitmap, "Android UI - Input image");
    }
    
    private void initGLView() {
    	glView = new GLSurfaceView(this);
		glView.setEGLContextClientVersion(2);
		glView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
		
		glRenderer = new GLRenderer(this);
		glView.setRenderer(glRenderer);
		
		glView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		glView.requestRender();
		
		initGLView(glView);
    }

	private void mainViewClickAction() {
		setContentView(glView);
		
		nextTestRun();
		
		initGLView(glView);
    }
    
    private void glViewClickAction() {
    	Log.i(TAG, "glView clicked");
//    	glView.requestRender();	// second requestRender needed because the first delivers empty buffer (GRALLOC hack issue)
    	
    	setContentView(R.layout.main);
    	
		infoTextView = (TextView)findViewById(R.id.infotext);
		Bitmap outputBm = null;
		if (extractedLineParams != null && extractedLineParams.size() > 0) {
			drawLinesToImg(extractedLineParams, inputImgMat);
			outputBm = Bitmap.createBitmap(inputImgMat.cols(),
						inputImgMat.rows(),
						Bitmap.Config.ARGB_8888);
			Utils.matToBitmap(inputImgMat, outputBm);
		} else {
			outputBm = Bitmap.createBitmap(outputImgMat.cols(),
					  outputImgMat.rows(),
					  Bitmap.Config.ARGB_8888);
			Utils.matToBitmap(outputImgMat, outputBm);
		}

		initTextView(infoTextView, outputBm, "Android UI - Output image");
    }

	private void initTextView(TextView t, Bitmap bm, CharSequence text) {
    	BitmapDrawable drawable = new BitmapDrawable(getResources(), bm);
		t.setCompoundDrawablesWithIntrinsicBounds(null, null, null, drawable);
		t.setText(text);
		
		t.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				mainViewClickAction();
			}
		});
    }
    
    private void initGLView(GLSurfaceView view) {
		view.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				glViewClickAction();
			}
		});
	}
    
    private void nextTestRun() {
    	Log.i(TAG, "Starting next test run with image#" + curRunBmIndex);
    	
    	inputImgMat = matFromBitmap(bitmaps.get(curRunBmIndex));
		outputImgMat = new Mat(inputImgMat.rows(), inputImgMat.height(), inputImgMat.type());
		glRenderer.setInputImageAddr(inputImgMat.getNativeObjAddr());
		glRenderer.setOutputImageAddr(outputImgMat.getNativeObjAddr());
		
		glView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
		glView.requestRender();
    }
    
    private void setupTestCases() {  
    	bitmapNames = new ArrayList<String>();
    	bitmaps = new ArrayList<Bitmap>();
    	int imgSet[];
    	
    	if (loadBinImg) {
			imgSet = new int[4];
			imgSet[0] = R.drawable.test_02_256_canny;
			bitmapNames.add("256x256");
			imgSet[1] = R.drawable.test_02_512_canny;
//			imgSet[1] = R.drawable.houghtest;
			bitmapNames.add("512x512");
			imgSet[2] = R.drawable.test_02_1024_canny;
			bitmapNames.add("1024x1024");
			imgSet[3] = R.drawable.test_02_2048_canny;
			bitmapNames.add("2048x2048");
    	} else {
			imgSet = new int[4];
			imgSet[0] = R.drawable.test_01_256;
			bitmapNames.add("256x256");
			imgSet[1] = R.drawable.test_01_512;
			bitmapNames.add("512x512");
			imgSet[2] = R.drawable.test_01_1024;
			bitmapNames.add("1024x1024");
			imgSet[3] = R.drawable.test_01_2048;
			bitmapNames.add("2048x2048");
    	}
		
		for (int i = 0; i < imgSet.length; i++) {
			int imgId = imgSet[i];
			if ((selectSingleImg >= 0 && selectSingleImg == i) || selectSingleImg < 0) {
				Bitmap bm = BitmapFactory.decodeResource(getResources(), imgId);
				bitmaps.add(bm);
			}
		}
		
		Log.i(TAG, "Loaded " + bitmaps.size() + " images for tests");
    }
    
    private void resetBenchmark() {
    	benchmarkRes = "";
    	curRunBmIndex = 0;
		selectedKernelType = kernelNames.get(useKernel);
    }
    
    private Mat matFromBitmap(Bitmap bm) {
    	Mat mat = new Mat();
    	Utils.bitmapToMat(bm, mat);
    	
    	return mat;
    }
    
    private Mat matFromBitmapResource(int resId) {
    	Bitmap bm = BitmapFactory.decodeResource(getResources(), resId);
    	return matFromBitmap(bm);
    }
    
    private void drawLinesToImg(ArrayList<Point> lineParams, Mat img) {
    	Log.i(TAG, "Extracted " + lineParams.size() + " lines");
    	
    	final double w = img.cols();
    	final double h = img.rows();
    	final Scalar lineColor = new Scalar(255, 255, 255);
    	
    	for (Point lineParam : lineParams) {
    		final double m = lineParam.x;
    		final double b = lineParam.y;
    		
    		//Log.i(TAG, "Line: m=" + m + "\tb=" + b);
    		
    		double x0 = 0.0f;
    		double y0 = b * h;
    		double x1 = w;
    		double y1 = (m + b) * h;

    		Point p0 = new Point(x0, y0);
    		Point p1 = new Point(x1, y1);

    		
    		Core.line(img, p0, p1, lineColor, 3);
    	}
	}

	public void setExtractedLineParams(ArrayList<Point> lineParams) {
		extractedLineParams = lineParams;
	}
}
