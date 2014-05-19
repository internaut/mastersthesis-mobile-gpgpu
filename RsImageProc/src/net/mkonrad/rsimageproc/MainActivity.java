package net.mkonrad.rsimageproc;

import java.util.ArrayList;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap.Config;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.support.v8.renderscript.*;

public class MainActivity extends Activity
{
	private static final String TAG = "RsImageProc::MainActivity";
	
	private static boolean useTestCases = true;	// if true, <numTestRuns> are run and statistics are provided
	private static int numTestRuns = 2;
	private static int useKernel = 1; 			// 0 is 3x3 gauss kernel, 1 is 5x5, 2 is 7x7
	
	private ArrayList<String> kernelNames;
	private String selectedKernelType;
	
	private ArrayList<Bitmap> bitmaps;
	private ArrayList<String> bitmapNames;
	
    private RenderScript rs;
    private Allocation inAlloc;
    private Allocation outAlloc;
    private ScriptC_gauss_3x3 rsGauss3x3 = null;
    private ScriptC_gauss_for_3x3 rsGaussFor3x3 = null;
	
	private ImageView imgView;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        imgView = (ImageView)findViewById(R.id.main_image_view);
        
		bitmaps = new ArrayList<Bitmap>();
		bitmapNames = new ArrayList<String>();
		
		kernelNames = new ArrayList<String>();
		kernelNames.add("3x3");
		kernelNames.add("3x3-for");
		
		if (useTestCases) {
			setupTestCases();
		} else {
			bitmaps.add(BitmapFactory.decodeResource(getResources(), R.drawable.test_01_2048));
			numTestRuns = 1;
		}
		
		imgView.setImageBitmap(bitmaps.get(0));
		
		selectedKernelType = kernelNames.get(useKernel);
                
        rs = RenderScript.create(this);
        
        if (useKernel == 0) {
        	rsGauss3x3 = new ScriptC_gauss_3x3(rs);
        } else if (useKernel == 1) {
        	rsGaussFor3x3 = new ScriptC_gauss_for_3x3(rs);
        }
        
        ImageViewClickListener clickListener = new ImageViewClickListener();
        imgView.setOnClickListener(clickListener);
    }
    
    private void setupTestCases() {		
		int imgSet[] = new int[1];
//		imgSet[0] = R.drawable.test_01_256;
//		bitmapNames.add("256x256");
//		imgSet[0] = R.drawable.test_01_512;
//		bitmapNames.add("512x512");
//		imgSet[0] = R.drawable.test_01_1024;
//		bitmapNames.add("1024x1024");
		imgSet[0] = R.drawable.test_01_2048;
		bitmapNames.add("2048x2048");
		
		for (int i = 0; i < imgSet.length; i++) {
			int imgId = imgSet[i];
			Bitmap bm = BitmapFactory.decodeResource(getResources(), imgId);
			bitmaps.add(bm);
		}
    }
    
    private Bitmap createRsAlloc(Bitmap inBm) {    	
        inAlloc = Allocation.createFromBitmap(rs, inBm,
                	Allocation.MipmapControl.MIPMAP_NONE,
                	Allocation.USAGE_SCRIPT);
        
        Log.i(TAG, "Creating new output bitmap with size " + inBm.getWidth() + "x" + inBm.getHeight());
        
        Bitmap outBm = Bitmap.createBitmap(inBm.getWidth(), inBm.getHeight(), inBm.getConfig());
        
        outAlloc = Allocation.createFromBitmap(rs, outBm,
                	Allocation.MipmapControl.MIPMAP_NONE,
                	Allocation.USAGE_SCRIPT);
        
        return outBm;
    }
    
	private Bitmap runFilter() {
		Log.i(TAG, "Running filter with " + numTestRuns + " test runs...");
		
		String bmName = "default";
		String benchmarkRes = "";
		
		Bitmap outBm = null;
		
    	// run tests for each image size
		for (int bmIdx = 0; bmIdx < bitmaps.size(); bmIdx++) {
			Bitmap bm = bitmaps.get(bmIdx);
			if (useTestCases) {
				bmName = bitmapNames.get(bmIdx);
			}
			
			// create allocations for in- and output buffers			
			float execMsSum = 0.0f;
			long t1, t2;
			for (int test = 0; test < numTestRuns; test++) {
				outBm = createRsAlloc(bm);
				
				if (useKernel == 0) {
					Log.i(TAG, "Using 3x3 kernel");
					
					t1 = System.nanoTime();
					rsGauss3x3.set_in(inAlloc);
					t2 = System.nanoTime();
					execMsSum += (float)((double)(t2 - t1) / 1000000.0);
					
					t1 = System.nanoTime();
					rsGauss3x3.forEach_make_gauss(outAlloc);
					t2 = System.nanoTime();
					execMsSum += (float)((double)(t2 - t1) / 1000000.0);
				} else if (useKernel == 1) {
					Log.i(TAG, "Using 3x3-for kernel");
					
					t1 = System.nanoTime();
					rsGaussFor3x3.set_in(inAlloc);
					t2 = System.nanoTime();
					execMsSum += (float)((double)(t2 - t1) / 1000000.0);
					
					t1 = System.nanoTime();
					rsGaussFor3x3.forEach_make_gauss(outAlloc);
					t2 = System.nanoTime();
					execMsSum += (float)((double)(t2 - t1) / 1000000.0);					
				}
				
				t1 = System.nanoTime();
				outAlloc.copyTo(outBm);
				t2 = System.nanoTime();
				execMsSum += (float)((double)(t2 - t1) / 1000000.0);
			}
			
			float execMsAvg = execMsSum / (float)numTestRuns;
			
			if (useTestCases) {				
				benchmarkRes += selectedKernelType + ","
							  + bmName + ","
							  + execMsAvg + "\n";
			} else {
				Log.i(TAG, "Test result - exec " + execMsAvg);
			}
		}
		
		if (useTestCases) { 
			Log.i(TAG, "Benchmark result:");
			Log.i(TAG, benchmarkRes);
		}
		
		return outBm;
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
