package net.mkonrad.climageproc;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

import org.opencv.android.OpenCVLoader;

import android.content.Context;
import android.util.Log;

public class ClJNIGlue {
	private final static String TAG = "ClJNIGlue";
	
	static {
	    if (!OpenCVLoader.initDebug()) {
	        Log.e(TAG, "Could not load OpenCV as static library");
	    } else {
	    	Log.i(TAG, "Loaded OpenCV as static library");
	    }
		
		try {
			System.load("/system/vendor/lib/egl/libGLES_mali.so");
			System.loadLibrary("cl_jni");
		} catch (UnsatisfiedLinkError e) {
	        java.lang.System.err.println("native code library failed to load.\n" + e);
	        java.lang.System.exit(1);
	    }
	  }
	
	static public native int initCL(int type);
	static public native int createProg(String progName, ByteBuffer progSrc);
	static public native int createKernel();
	static public native int createCmdQueue();
	static public native float setKernelArgs(int width, int height, ByteBuffer inputImgBuf);
	static public native float executeKernel();
	static public native float getResultImg(ByteBuffer outputImgBuf);
	static public native void dealloc();
	
	static public native void printInfo();
	
	// Read kernel file
	static public ByteBuffer getProgramBuffer(Context ctx, String file) {
	    String line;
	    InputStream stream;
	    byte[] programBytes;
	    StringBuilder program = new StringBuilder();
	    ByteBuffer buffer = null;

	    try {
	      stream = ctx.getResources().getAssets().open(file);
	      BufferedReader br = new BufferedReader(new InputStreamReader(stream)); 
	      while((line = br.readLine()) != null) { 
	        program.append(line); 
	      }
	      stream.close(); 
	      br.close();
	    } catch (IOException e) {
	      e.printStackTrace();
	    }
	    program.append("\0");
	    
	    // Create ByteBuffer
	    try {
	      programBytes = program.toString().getBytes("UTF-8");
	      buffer = ByteBuffer.allocateDirect(programBytes.length);
	      buffer.put(programBytes);
	      buffer.rewind();
	    } catch (UnsupportedEncodingException e) {
	      e.printStackTrace();
	    }

	    return buffer;
	  }
}
