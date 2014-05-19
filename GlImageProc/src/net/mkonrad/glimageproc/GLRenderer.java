package net.mkonrad.glimageproc;

import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import net.mkonrad.kernelgen.KernelGenerator;
import net.mkonrad.kernelgen.KernelLangGLSL;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.SystemClock;
import android.util.Log;

public class GLRenderer implements GLSurfaceView.Renderer {
	private static final String TAG = "GlImageProc::OGLES20Renderer";
	
	private static final boolean useTestCases = false;	// if true, <numTestRuns> are run and statistics are provided
	private static final int numTestRuns = 1;
	private static final int useKernel = 0;				// 0 is 3x3 gauss kernel, 1 is 5x5, 2 is 7x7
	
	private ArrayList<String> kernelNames = null;
	private ArrayList<float[]> kernels = null;
	private String selectedKernelType;
	
	private ArrayList<Bitmap> bitmaps = null;
	private ArrayList<String> bitmapNames = null;
	
	private int curRunBmIndex = 0;
	private int curTestRun = 0;
	private float pushMsSum;
	private float execMsSum;
	private float pullMsSum;
	private String benchmarkRes = "";
	
	private Context ctx;
	
	private GLQuad quad;
	
	private int screenW = 0;
	private int screenH = 0;
	
	private float[] identMat = new float[16];
	private float[] projMat = new float[16];
	private float[] viewMat = new float[16];
	private float[] mvpMat = new float[16];
	
	private GLShader gaussShader;
	private GLShader dispShader;

	private GLSurfaceView view;
	
	public GLRenderer(Context ctx) {
		this.ctx = ctx;

		Matrix.setIdentityM(identMat, 0);
	}
	
	public void setView(GLSurfaceView view) {
		this.view = view;
	}
	
	public void runTests() {
		curRunBmIndex = 0;
		
		pushMsSum = execMsSum = pullMsSum = 0.0f;
		
		view.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
		view.requestRender();
	}

	@Override
	public void onSurfaceCreated(GL10 unused, EGLConfig conf) {
		GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		GLES20.glDisable(GLES20.GL_DEPTH_TEST);
		GLES20.glEnable(GLES20.GL_CULL_FACE);
		GLES20.glCullFace(GLES20.GL_BACK);
		
		GLES20.glEnable(GLES20.GL_TEXTURE_2D);
		
		Log.i(TAG, "onSurfaceCreated");
		
		
		// Generate Kernel shader code
		KernelGenerator kernelGen = new KernelGenerator();
		
		setupKernels();
		
		selectedKernelType = kernelNames.get(useKernel);
		float[] kernelVals = kernels.get(useKernel);
		int kernelSize = (int) Math.sqrt(kernelVals.length);
		kernelGen.setKernel("gauss" + selectedKernelType, kernelSize, kernelSize, kernelVals);
		Log.i(TAG, "Kernel set to: " + useKernel + " - " + selectedKernelType + " (" + kernelSize + "x" + kernelSize + ")");
		
		String[] kernelSrcCode = kernelGen.getKernelCodeStrings(KernelGenerator.KernelLang.GLSL);
		
		Log.i(TAG, "Generated Code for SHADER_TYPE_VERTEX:\n" + kernelSrcCode[KernelLangGLSL.SHADER_TYPE_VERTEX]);
		Log.i(TAG, "Generated Code for SHADER_TYPE_FRAGMENT:\n" + kernelSrcCode[KernelLangGLSL.SHADER_TYPE_FRAGMENT]);
		
		// create shaders
		gaussShader = new GLShader(ctx,
				kernelSrcCode[KernelLangGLSL.SHADER_TYPE_VERTEX],
				kernelSrcCode[KernelLangGLSL.SHADER_TYPE_FRAGMENT]);
		dispShader = new GLShader(ctx, R.raw.disp_v, R.raw.disp_f);
		
		// load testcases or not
		bitmaps = new ArrayList<Bitmap>();
		bitmapNames = new ArrayList<String>();
		
		if (useTestCases) {
			setupTestCases();
		} else {
			bitmaps.add(BitmapFactory.decodeResource(ctx.getResources(), R.drawable.test_01_1024));
			bitmapNames.add("1024x1024");
		}
		
		quad = new GLQuad();
		quad.setNumRenderPasses(1);
		quad.setSaveFrameBuffer(true);
		quad.bindShaders(gaussShader, dispShader);
	}


	@Override
	public void onDrawFrame(GL10 unused) {
		Log.i(TAG, "test run: " + curTestRun);
		
		// Redraw background color
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        
        // Set the camera position (View matrix)
        Matrix.setLookAtM(viewMat, 0, 0.0f, 0.0f, 2.5f, 0f, 0f, 0f, 0f, 1.0f, 0.0f);

        // Calculate the projection and view transformation
        Matrix.multiplyMM(mvpMat, 0, projMat, 0, viewMat, 0);
        
        // do preparations (only for measuring the times!)
        quad.clearTextures();
        quad.prepareTextures();
        
        pushMsSum += quad.setTextureFromBitmap(bitmaps.get(curRunBmIndex));
        quad.createGeometry();
        
        // Render
        float[] execAndPullMs = quad.render(mvpMat, screenW, screenH);
        execMsSum += execAndPullMs[0];
        pullMsSum += execAndPullMs[1];
        
        GLES20.glFinish();
        
        curTestRun++;
        
        if (curTestRun >= numTestRuns) {
        	testsFinished();
        }
	}

	@Override
	public void onSurfaceChanged(GL10 unused, int w, int h) {
		Log.i(TAG, "onSurfaceChanged with new size " + w + "x" + h);
		
		screenW = w;
		screenH = h;
		
		GLES20.glViewport(0, 0, w, h);
		
		float ratio = (float) w / h;

	    // this projection matrix is applied to object coordinates
	    // in the onDrawFrame() method
	    Matrix.orthoM(projMat, 0, -ratio, ratio, -1, 1, -1, 1);
	}
	
	public Bitmap getResultBm() {
		return quad.getResultBm();
	}
	
	private void testsFinished() {    	
    	float pushMsAvg = pushMsSum / (float)numTestRuns;
    	float execMsAvg = execMsSum / (float)numTestRuns;
    	float pullMsAvg = pullMsSum / (float)numTestRuns;
		
        benchmarkRes += selectedKernelType + ","
				  	  + bitmapNames.get(curRunBmIndex) + ","
				  	  + pushMsAvg + "," + execMsAvg + "," + pullMsAvg + "\n";
        
    	curTestRun = 0;
    	curRunBmIndex++;	// take next image
    	
    	if (curRunBmIndex >= bitmaps.size()) {	// stop testing
    		Log.i(TAG, "All tests finished");
    		Log.i(TAG, benchmarkRes);
    		
    		benchmarkRes = "";
    		
    		view.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
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
			Bitmap bm = BitmapFactory.decodeResource(ctx.getResources(), imgId);
			bitmaps.add(bm);
		}
    }
}
