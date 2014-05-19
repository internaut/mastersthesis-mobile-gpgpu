package net.mkonrad.cvmarkerdetect;

import java.io.IOException;

import net.mkonrad.cvaccar.CvAccARDroid;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Size;
import org.opencv.imgproc.Imgproc;
import org.xmlpull.v1.XmlPullParserException;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap.Config;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.ImageView;

public class MainActivity extends Activity
{
	private static final String    TAG = "CvMarkerDetect::MainActivity";
	
	static {
	    if (!OpenCVLoader.initDebug()) {
	        Log.e(TAG, "Could not load OpenCV as static library");
	    } else {
	    	Log.i(TAG, "Loaded OpenCV as static library");
	    }
	}
	
	private boolean dbgStillImage = false;					// enable to show only a still image instead of the camera video stream
	private boolean dbgStillImageRendermodeDirty = false;
	private CvAccARDroid cvAccAR;
	private MenuItem[] menuItems;
	
	public MainActivity()
	{
		menuItems = new MenuItem[7];
	}
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        
        cvAccAR = new CvAccARDroid(this);
        cvAccAR.drawDbgMarkers(true);
        
        if (dbgStillImage) {        	
        	Bitmap dbgImgBmp = bitmapFromResId(R.drawable.testimg2marker8);
        	cvAccAR.setInput(dbgImgBmp, "nexus10-vid.xml", dbgStillImageRendermodeDirty);
        } else {
        	cvAccAR.setInput(0, "nexus10-vid.xml");
        }
        
        try {
			cvAccAR.start();
		} catch (IOException e) {
			Log.e(TAG, "Error starting CvAccAR");
			e.printStackTrace();
		} catch (XmlPullParserException e) {
			Log.e(TAG, "Error starting CvAccAR");
			e.printStackTrace();
		}
        
        setContentView(cvAccAR.getGLView());
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
    	menuItems[0] = menu.add("default");
    	menuItems[1] = menu.add("preprocessed");
    	menuItems[2] = menu.add("thresholded");
    	menuItems[3] = menu.add("threshold postproc");
    	menuItems[4] = menu.add("contours");
    	menuItems[5] = menu.add("candidates");
    	menuItems[6] = menu.add("norm. marker");

        return true;
    }
    
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "called onOptionsItemSelected; selected item: " + item);

        for (int i = 0; i < menuItems.length; i++) {
        	if (item == menuItems[i]) {
        		cvAccAR.setOutputMode(i);
        	}
        }

        return true;
    }
    
    @Override
    protected void onPause() {
        cvAccAR.pause();
        
        super.onPause();
    }
    
    @Override
    protected void onResume() {
    	cvAccAR.resume();
    	
    	super.onResume();
    }
    
    public void onDestroy() {
        cvAccAR.stop();
        
        super.onDestroy();
    }
	
	private Bitmap bitmapFromResId(int res) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;	// important!!!
        options.inPreferredConfig = Config.ARGB_8888;
        Bitmap bmp = BitmapFactory.decodeResource(getResources(), res, options);
        
        return bmp;
	}
}
