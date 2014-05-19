package net.mkonrad.glimageproc;

import android.app.Activity;
import android.graphics.Bitmap;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

public class MainActivity extends Activity
{
	private static final String TAG = "GlImageProc::MainActivity";
	
	private GLSurfaceView glView = null;
	private GLRenderer glRenderer = null;
	private ImageView imgView = null;
	
	private Bitmap lastResultBm = null;
	
	private boolean testsRan = true;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        
        // sets defaults
        handleTouchOnImg();
        
        // create GL view
        glView = new GLSurfaceView(this);
        glView.setEGLContextClientVersion(2);
        glRenderer = new GLRenderer(this);
        glView.setRenderer(glRenderer);
        glRenderer.setView(glView);
        glView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        glView.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				handleTouchOnImg();
			}
		});
        
        // set up image view
        imgView = (ImageView)findViewById(R.id.main_image_view);
        imgView.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				handleTouchOnImg();
			}
		});
    }
    
    @Override
    protected void onPause() {
        super.onPause();

        glView.onPause();
    }
    
    @Override
    protected void onResume() {
        super.onResume();

        glView.onResume();
    }
    
    private void handleTouchOnImg() {
    	Log.i(TAG, "Touch on image");
    	
    	if (!testsRan) {
    		setContentView(glView);
    		
			runTests();
			testsRan = true;
			
			Log.i(TAG, "Content view is now: glView");
		} else {
			testsRan = false;
			lastResultBm = null;
			
			if (glRenderer != null) {
				lastResultBm = glRenderer.getResultBm();
			}
			
			if (lastResultBm != null) {
				Log.i(TAG, "Updating image view");
				imgView.setImageBitmap(lastResultBm);
			}
			
			setContentView(R.layout.main);
			
			Log.i(TAG, "Content view is now: main");
		}
    }
    
    private void runTests() {
    	glRenderer.runTests();
    }
}
