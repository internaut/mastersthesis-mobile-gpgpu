package net.mkonrad.climageproc;

import java.nio.ByteBuffer;
import java.util.ArrayList;

import net.mkonrad.climageproc.R;
import net.mkonrad.kernelgen.KernelGenerator;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

public class MainActivity extends Activity
{
	private static final String TAG = "ClImageProc::MainActivity";
	
	private static boolean useTestCases = false;	// if true, <numTestRuns> are run and statistics are provided
	private static int numTestRuns = 10;
	private static int useKernel = -1;				// "-1" is hough, 0 is 3x3 gauss kernel, 1 is 5x5, 2 is 7x7 
	
	private ImageView imgView;
	
	private ArrayList<float[]> kernels;
	private ArrayList<String> kernelNames;
	private String selectedKernelType;
	
	private ArrayList<Bitmap> bitmaps;
	private ArrayList<String> bitmapNames;
	
	private KernelGenerator kernelGen;
	private String kernelSrcFile = null;			// auto-generate CL kernel source if this is null
	private int clKernelProgType;
	private String clKernelProgName;
	private String clKernelProgSrc;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
                
        imgView = (ImageView)findViewById(R.id.main_image_view);
        
		bitmaps = new ArrayList<Bitmap>();
		bitmapNames = new ArrayList<String>();
		
		if (useKernel >= 0) {
			setupImgConv();
		} else if (useKernel == -1) {
			kernelSrcFile = "hough.cl";
			clKernelProgName = "cl_hough";
			clKernelProgType = 1;
		}
		
		
		if (useTestCases) {
			setupTestCases();
		} else {
			numTestRuns = 1;
			BitmapDrawable imgSrcDrawable = (BitmapDrawable)imgView.getDrawable();
			bitmaps.add(imgSrcDrawable.getBitmap());
		}
        
        if (!setupCL(kernelSrcFile)) {
        	return;
        }
        
		imgView.setOnClickListener(new ImageViewClickListener());
    }
    
    private void setupImgConv() {
    	clKernelProgType = 0;
    	
        kernelGen = new KernelGenerator();
		
		setupKernels();
		
		selectedKernelType = kernelNames.get(useKernel);
				
		if (kernelSrcFile == null) {
			float[] kernelVals = kernels.get(useKernel);
			int kernelSize = (int) Math.sqrt(kernelVals.length);
			
			clKernelProgName = "gauss" + selectedKernelType;
			kernelGen.setKernel(clKernelProgName, kernelSize, kernelSize, kernelVals);
			Log.i(TAG, "Kernel '" + clKernelProgName + "' set to: " + useKernel + " - " + selectedKernelType + " (" + kernelSize + "x" + kernelSize + ")");
		
			String[] kernelSrcCode = kernelGen.getKernelCodeStrings(KernelGenerator.KernelLang.CL);
			clKernelProgSrc = kernelSrcCode[0];
			Log.i(TAG, "Generated CL kernel source code:\n" + clKernelProgSrc);
		} else {
			clKernelProgName = "cl_filter";
		}
	}

	private void setupKernels() {
		kernelNames = new ArrayList<String>();
		
		kernelNames.add("3x3");
		final float[] kernelVals3x3 = {
				1f, 2f, 1f,
				2f, 4f, 2f,
				1f, 2f, 1f
			};
		
		kernelNames.add("5x5");
		final float[] kernelVals5x5 = {
			2f,   7f,   12f,    7f,   2f,
			7f,  31f,   52f,   31f,   7f,
		   15f,  52f,  127f,   52f,  15f,
			7f,  31f,   52f,   31f,   7f,
			2f,   7f,   12f,    7f,   2f
		};
		
		kernelNames.add("7x7");
		
		final float[] kernelVals7x7 = {
				1f, 12f, 55f, 90f, 55f, 12f, 1f,
				12f, 148f, 665f, 1097f, 665f, 148f, 12f,
				55f, 665f, 2981f, 4915f, 2981f, 665f, 55f,
				90f, 1097f, 4915f, 8103f, 4915f, 1097f, 90f,
				55f, 665f, 2981f, 4915f, 2981f, 665f, 55f,
				12f, 148f, 665f, 1097f, 665f, 148f, 12f,
				1f, 12f, 55f, 90f, 55f, 12f, 1f
			};
		
		
		kernels = new ArrayList<float[]>();
		kernels.add(kernelVals3x3);
		kernels.add(kernelVals5x5);
		kernels.add(kernelVals7x7);
	}

	private void setupTestCases() {		
		int imgSet[] = new int[4];
		imgSet[0] = R.drawable.test_01_256;
		bitmapNames.add("256x256");
		imgSet[1] = R.drawable.test_01_512;
		bitmapNames.add("512x512");
		imgSet[2] = R.drawable.test_01_1024;
		bitmapNames.add("1024x1024");
		imgSet[3] = R.drawable.test_01_2048;
		bitmapNames.add("2048x2048");
		
		for (int i = 0; i < imgSet.length; i++) {
			int imgId = imgSet[i];
			Bitmap bm = BitmapFactory.decodeResource(getResources(), imgId);
			bitmaps.add(bm);
		}
    }
    
    private boolean setupCL(String clFile) {
        if (ClJNIGlue.initCL(clKernelProgType) == 0) {
        	Log.i(TAG, "CL init successful");
        } else {
        	Log.e(TAG, "CL init error");
        	return false;
        }
        
        // ClJNIGlue.printInfo();
        
        ByteBuffer progSrc;
        
        if (kernelSrcFile != null) {
        	Log.i(TAG, "Loading program source code in file " + clFile);
        	progSrc = ClJNIGlue.getProgramBuffer(getBaseContext(), clFile);
        	Log.i(TAG, "Loaded CL program source code containing " + progSrc.capacity() + " bytes");
        } else {
        	progSrc = ByteBuffer.allocateDirect(clKernelProgSrc.length());
        	progSrc.put(clKernelProgSrc.getBytes());
        	progSrc.rewind();
        	Log.i(TAG, "Using generated kernel source of size " + progSrc.capacity() + " bytes");
        }
        
        if (ClJNIGlue.createProg(clKernelProgName, progSrc) == 0) {
        	Log.i(TAG, "CL createProg successful");
        } else {
        	Log.e(TAG, "CL createProg error");
        	return false;
        }
        
        if (ClJNIGlue.createKernel() == 0) {
        	Log.i(TAG, "CL createKernel successful");
        } else {
        	Log.e(TAG, "CL createKernel error");
        	return false;
        }
        
        if (ClJNIGlue.createCmdQueue() == 0) {
        	Log.i(TAG, "CL createCmdQueue successful");
        } else {
        	Log.e(TAG, "CL createCmdQueue error");
        	return false;
        }
        
        return true;
    }
    
	private Bitmap runFilter() {
		Log.i(TAG, "Running filter with " + numTestRuns + " test runs...");
		
		String bmName = "default";
		String benchmarkRes = "";
		
    	int imgWidth = 0;
    	int imgHeight = 0;
    	int imgChan = 0;
    	int imgByteSize = 0;
    	ByteBuffer outputImgBuf = null;
		
    	// run tests for each image size
		for (int bmIdx = 0; bmIdx < bitmaps.size(); bmIdx++) {
			Bitmap bm = bitmaps.get(bmIdx);
			if (useTestCases) {
				bmName = bitmapNames.get(bmIdx);
			}
			
			// get image properties
	    	imgWidth = bm.getWidth();
	    	imgHeight = bm.getHeight();
	    	imgChan = bm.getRowBytes() / imgWidth;
	    	imgByteSize = imgWidth * imgHeight * imgChan;
	    	
	    	Log.i(TAG, "Testing with image '" + bmName + "' with size " + imgWidth + "x" + imgHeight + ", " + imgChan + " channels");
	    	
	    	// allocate input buffer and copy data
			ByteBuffer inputImgBuf = ByteBuffer.allocateDirect(imgByteSize);
			bm.copyPixelsToBuffer(inputImgBuf);
			inputImgBuf.rewind();
			
			// allocate output buffer
			outputImgBuf = ByteBuffer.allocateDirect(imgByteSize);
			
			float pushMsSum = 0.0f;
			float execMsSum = 0.0f;
			float pullMsSum = 0.0f;
			for (int test = 0; test < numTestRuns; test++) {
				float pushMs = ClJNIGlue.setKernelArgs(imgWidth, imgHeight, inputImgBuf);
				if (pushMs < 0.0f) {
					Log.e(TAG, "CL createKernel error");
					return null;
				}
				pushMsSum += pushMs;
				
				float execMs = ClJNIGlue.executeKernel();
				if (execMs < 0.0f) {
					Log.e(TAG, "CL executeKernel error");
					return null;
				}
				execMsSum += execMs;
				
				float pullMs = ClJNIGlue.getResultImg(outputImgBuf);
				if (pullMs < 0.0f)  {
					Log.e(TAG, "CL getResultImg error");
					return null;
				}
				pullMsSum += pullMs;
			}
			
			if (useTestCases) {
				float pushMsAvg = pushMsSum / (float)numTestRuns;
				float execMsAvg = execMsSum / (float)numTestRuns;
				float pullMsAvg = pullMsSum / (float)numTestRuns;
				
				benchmarkRes += selectedKernelType + ","
							  + bmName + ","
							  + pushMsAvg + ","
							  + execMsAvg + ","
							  + pullMsAvg + "\n";
			}			
		}
		
		if (useTestCases) { 
			Log.i(TAG, "Benchmark result:");
			Log.i(TAG, benchmarkRes);
		}
		
		// create a output bitmap of the last processed image
		if (outputImgBuf == null) return null;
		
		Bitmap outputBitmap = Bitmap.createBitmap(imgWidth, imgHeight, Bitmap.Config.ARGB_8888);
		outputBitmap.copyPixelsFromBuffer(outputImgBuf);
//		outputImgBuf.rewind();    	
		
		return outputBitmap;
	}
    
	private class ImageViewClickListener implements View.OnClickListener {
		private boolean filtered;
		
		
		public ImageViewClickListener() {
			this.filtered = false;
		}
		
		@Override
		public void onClick(View v) {
			ImageView imgView = (ImageView)v;
			Bitmap filteredBitmap = null;

			if (!filtered) {
				filteredBitmap = runFilter();
				
				filtered = true;
			} else {
				filteredBitmap = bitmaps.get(0);
				filtered = false;
			}

			if (filteredBitmap != null) {
				BitmapDrawable filteredBitmapDrawable = new BitmapDrawable(getResources(), filteredBitmap);
			
				imgView.setImageDrawable(filteredBitmapDrawable);
			}
		}
	}
}
