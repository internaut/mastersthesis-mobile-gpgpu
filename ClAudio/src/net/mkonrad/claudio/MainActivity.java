package net.mkonrad.claudio;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity
{
	private final String TAG = this.getClass().getSimpleName();
	
	private Thread audioThread = null;
	private Button audioCtrlBtn;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        audioCtrlBtn = (Button)findViewById(R.id.audio_control_button);
        
        cl_audio.create_cl_kernel_from_src(loadStringsFromFile(this, "synth_mono_additive.cl"));
    }
    
	public void audioToggle(View view) {
		if (audioThread == null) {
			playAudio();
			audioCtrlBtn.setText("Stop");
		} else {
			stopAudio();
			audioCtrlBtn.setText("Play");
		}
	}
	
	@Override
	protected void onPause() {
		stopAudio();
		
		super.onPause();
	}
	
	@Override
	protected void onDestroy() {
		cl_audio.cleanup();
		
		super.onDestroy();
	}
	
	private void playAudio() {
		if (audioThread != null) return;
		
		Log.i(TAG, "starting audio");
		
		audioThread = new Thread() {
            public void run() {
                setPriority(Thread.MAX_PRIORITY);
                cl_audio.start_process();
            }
        };
        
        audioThread.start();   
	}
	
	private void stopAudio() {
		if (audioThread == null) return;
		
		Log.i(TAG, "stopping audio");
		
		cl_audio.stop_process();
		
		try {
            audioThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
		
		audioThread = null;
	}
	
	private static String loadStringsFromFile(Context ctx, String file) {
	    String line;
	    InputStream stream;
	    StringBuilder program = new StringBuilder();

	    try {
	      stream = ctx.getResources().getAssets().open(file);
	      BufferedReader br = new BufferedReader(new InputStreamReader(stream)); 
	      while((line = br.readLine()) != null) { 
	        program.append(line.trim()).append("\n"); 
	      }
	      stream.close(); 
	      br.close();
	    } catch (IOException e) {
	      e.printStackTrace();
	    }
	    program.append('\0');
	    
	    return program.toString().trim();
	}
}
