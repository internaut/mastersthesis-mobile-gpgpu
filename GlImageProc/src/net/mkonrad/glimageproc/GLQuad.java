package net.mkonrad.glimageproc;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.opengl.Matrix;
import android.util.Log;

public class GLQuad {
	private static final String TAG = "GlImageProc::GLQuad";
	
	private static final int SHADER_PARAM_TYPE_ATTR = 0;
	private static final int SHADER_PARAM_TYPE_UNIF = 1;
	
	private static final int TEX_INPUT = 0;
	private static final int TEX_FBO = 1;
	
    private static final int COORDS_PER_VERTEX = 3;
    private static final int TEX_COORDS_PER_VERTEX = 2;
	
	private int numRenderPasses;
	
	// filter shader parameters:
	private int f_aTexCoordId;
	private int f_aPosId;
	private int f_uPxDeltaId;
	// display shader parameters:
	private int d_aTexCoordId;
	private int d_aPosId;
	private int d_uMVPMatrixId;
	
	private int[] texIds = null;
	private int texImgW, texImgH;
	private float texImgRatio;
	
	private float pxDeltaW, pxDeltaH;
	
	private ArrayList<FrameBufferObject> fboList;
	
	private GLShader filterShader;
	private GLShader dispShader;

	private ArrayList<IntBuffer> fboBufList;
	
	private FloatBuffer texBufStd;
	private FloatBuffer texBufFlipped;
	
	private FloatBuffer dispVertexBuf;
	private FloatBuffer fboVertexBuf;
    private int vertexCount;
    
    private boolean saveFrameBuffer = false;
    private Bitmap resultBm;
    
    private long t1, t2;

	
	public GLQuad() {
		numRenderPasses = 1;
	}
	
	
	
    public boolean isSaveFrameBuffer() {
		return saveFrameBuffer;
	}

	public void setSaveFrameBuffer(boolean saveFrameBuffer) {
		this.saveFrameBuffer = saveFrameBuffer;
	}

	public int getNumRenderPasses() {
		return numRenderPasses;
	}

	public void setNumRenderPasses(int numRenderPasses) {
		this.numRenderPasses = numRenderPasses;
	}

	public void bindShaders(GLShader filter, GLShader disp) {
    	filterShader 	= filter;
    	dispShader 		= disp;
    	int filterProg 	= filterShader.getProgram();
    	int dispProg 	= dispShader.getProgram();
    	
    	// get ids for filter shader program parameters
    	f_aPosId 		= getShaderParamId(SHADER_PARAM_TYPE_ATTR, filterProg, "aPos");
    	f_aTexCoordId	= getShaderParamId(SHADER_PARAM_TYPE_ATTR, filterProg, "aTexCoord");
    	f_uPxDeltaId 	= getShaderParamId(SHADER_PARAM_TYPE_UNIF, filterProg, "uPxD");

    	// get ids for display shader program parameters
    	d_aPosId 		= getShaderParamId(SHADER_PARAM_TYPE_ATTR, dispProg, "aPos");
    	d_aTexCoordId	= getShaderParamId(SHADER_PARAM_TYPE_ATTR, dispProg, "aTexCoord");
    	d_uMVPMatrixId 	= getShaderParamId(SHADER_PARAM_TYPE_UNIF, dispProg, "uMVPMatrix");
    }
	
	public void prepareTextures() {
    	// Generate all textures
        texIds = new int[numRenderPasses + 1];
    	GLES20.glGenTextures(texIds.length, texIds, 0);
	}
    
    public float setTextureFromBitmap(Bitmap bm) {
    	// Set image information
    	texImgW = bm.getWidth();
    	texImgH = bm.getHeight();
    	
    	// Calculate image ratio
    	texImgRatio = (float)texImgW / texImgH;
    	
    	// Calculate pixel delta values for shaders
        pxDeltaW = 1.0f / (float)(texImgW - 1.0f);
        pxDeltaH = 1.0f / (float)(texImgH - 1.0f);
        
        // Log
        Log.i(TAG, "Set texture from bitmap with size "
        		  + texImgW + "x" + texImgH + ", ratio " + texImgRatio);
    	
    	// Bind input texture
    	GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texIds[TEX_INPUT]);
    	
    	// Load the texture
    	GLES20.glFinish();
    	t1 = System.nanoTime();
    	GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bm, 0);
    	GLES20.glFinish();
    	t2 = System.nanoTime();
//    	printDeltaTimeMs("texture upload");
    	
    	GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
    	GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);
//    	GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGB, 2, 2, 0, GL_RGB,
//    	GL_UNSIGNED_BYTE, pixels);
    	
    	// Create FBO texture(s)
    	if (numRenderPasses > 0) {
    		fboBufList = new ArrayList<IntBuffer>(numRenderPasses);
    		fboList = new ArrayList<FrameBufferObject>(numRenderPasses);
    		
    		// Create FBO buffers and FBOs for each pass
	    	for (int i = 0; i < numRenderPasses; i++) {
	    		int t = texIds[TEX_FBO + i];
	    		
	    		// Bind the FBO texture
		        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, t);
		        
		        // Create texture buffer
		    	IntBuffer fboBuf = ByteBuffer.allocateDirect((int)(texImgW * texImgH * 4))
		                		   .order(ByteOrder.nativeOrder()).asIntBuffer();
		    	
		    	// Create texture
		        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA,
		        		(int)texImgW, (int)texImgH, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE,
		        		fboBuf); // note texW and texH swapped
		        
		        // Set texture parameters
		    	GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
		    	GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);
		    	
		//        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
		//                 GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		//        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
		//                 GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		//        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
		//                 GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
		//        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
		//                 GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
		    	
		    	// create the FBO
		    	FrameBufferObject fbo = new FrameBufferObject();
		    	
		    	fbo.init();
		    	fbo.bind();
		    	fbo.attachTextureID(t);
		    	fbo.unbind();
		    	
		    	// add FBO to FBO list
		    	fboList.add(fbo);
		    	
		    	// add buffer to FBO Buffer List
		    	fboBufList.add(fboBuf);
	    	}
    	}
    	
    	return (float)((double)(t2 - t1) / 1000000.0);
    }

	public void createGeometry() {
    	// Create vertex buffer for display
    	final float w = (texImgRatio > 1.0f) ? 1.0f : texImgRatio;
    	final float h = (texImgRatio > 1.0f) ? 1.0f / texImgRatio : 1.0f;
    	
		final float[] dispVertices = { -w, -h, 0,
									    w, -h, 0,
								       -w,  h, 0,
								        w,  h, 0 };

		vertexCount = dispVertices.length / COORDS_PER_VERTEX;
		if (dispVertexBuf != null) dispVertexBuf.clear();
		dispVertexBuf = ByteBuffer.allocateDirect(dispVertices.length * 4)
	            		.order(ByteOrder.nativeOrder()).asFloatBuffer();
		dispVertexBuf.put(dispVertices).position(0);
		
    	// Create vertex buffer for FBOs
		final float[] fboVertices = { -1, -1, 0,
			    					   1, -1, 0,
			    					  -1,  1, 0,
			    					   1,  1, 0 };
		if (fboVertexBuf != null) fboVertexBuf.clear();
		fboVertexBuf = ByteBuffer.allocateDirect(fboVertices.length * 4)
				       .order(ByteOrder.nativeOrder()).asFloatBuffer();
		fboVertexBuf.put(fboVertices).position(0);

		
		// Create texture buffers
		
	    final float[] texCoordsStd = { 0, 0, 1, 0, 0, 1, 1, 1 };		// standard
	    final float[] texCoordsFlipped = { 0, 1, 1, 1, 0, 0, 1, 0 };	// flip
//	    final float[] texCoordsHeadsup = { 1, 1,
//	    							0, 1,
//	    							1, 0,
//	    							0, 0 };
	    if (texBufStd != null) texBufStd.clear();
	    texBufStd = ByteBuffer.allocateDirect(texCoordsStd.length * 4)
	            .order(ByteOrder.nativeOrder()).asFloatBuffer();
	    texBufStd.put(texCoordsStd).position(0);
	    
	    if (texBufFlipped != null) texBufFlipped.clear();
	    texBufFlipped = ByteBuffer.allocateDirect(texCoordsFlipped.length * 4)
	            .order(ByteOrder.nativeOrder()).asFloatBuffer();
	    texBufFlipped.put(texCoordsFlipped).position(0);
    }
    
    public void clearTextures() {
    	if (texIds != null) {
    		GLES20.glDeleteTextures(texIds.length, texIds, 0);
    	}
    }
    
    public float[] render(float[] mvpMat, int viewW, int viewH) {
    	float[] msMeasures = {0.0f, 0.0f};
    	
    	long tTotal = 0;
    	
    	if (numRenderPasses > 0) {
    		filterShader.use();
    		GLES20.glFinish();
    		
	    	// Draw renderpasses to FBO
	    	for (int pass = 0; pass < numRenderPasses; pass++) {
	    		t1 = System.nanoTime();
	    		render(null, pass, texImgW, texImgH);
	    		GLES20.glFinish();
	    		t2 = System.nanoTime();
	    		
	    		tTotal += t2 - t1;
	    		
	        	if (saveFrameBuffer && pass == numRenderPasses - 1) {
	        		msMeasures[1] = saveFrameBufferToBitmap(texImgW, texImgH);
	        	}
	    	}
    	}
    	
    	msMeasures[0] = (float)((double)(tTotal) / 1000000.0);
    	
    	// Draw to output
    	dispShader.use();
    	render(mvpMat, -1, viewW, viewH);
    	
    	return msMeasures;
    }

	private void render(float[] mvpMat, int renderPass, int viewW, int viewH) {
		FloatBuffer texBuf, vertexBuf;
		FrameBufferObject fbo = null;
		int aPosId, uMVPMatrixId, uPxDeltaId, aTexCoordId;
		
		// Bind input texture depending on if we're drawing to/from a FBO
		// or to the screen
		if (renderPass >= 0) {	// use filter and draw to FBO
			fbo = fboList.get(renderPass);
			
			fbo.bind();
			GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
			
			// set input texture
			GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texIds[TEX_FBO + renderPass - 1]);
			
			// set buffers
			texBuf = texBufStd;
			vertexBuf = fboVertexBuf;
			
			// set parameter ids
			aPosId 			= f_aPosId;
			uPxDeltaId 		= f_uPxDeltaId;
			uMVPMatrixId 	= -1;
			aTexCoordId 	= f_aTexCoordId;
		} else {		// use display shader and draw to screen
			// set input texture
			GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texIds[TEX_FBO + numRenderPasses - 1]);

			// set buffers
			texBuf = texBufFlipped;
			vertexBuf = dispVertexBuf;
			
			// set parameter ids
			aPosId 			= d_aPosId;
			uPxDeltaId 		= -1;
			uMVPMatrixId 	= d_uMVPMatrixId;
			aTexCoordId 	= d_aTexCoordId;
		}
		
		// Set viewport
		GLES20.glViewport(0, 0, viewW, viewH);

	    // Enable a handle to the vertices
	    GLES20.glEnableVertexAttribArray(aPosId);

	    // Prepare the coordinate data
	    GLES20.glVertexAttribPointer(aPosId, COORDS_PER_VERTEX,
	                                 GLES20.GL_FLOAT, false,
	                                 0, vertexBuf);
	    
	    // Pass the projection and view transformation to the shader
	    if (renderPass < 0) {
	    	GLES20.glUniformMatrix4fv(uMVPMatrixId, 1, false, mvpMat, 0);
	    } else {	// Set pixel delta values for the filter
	    	GLES20.glUniform2f(uPxDeltaId, pxDeltaW, pxDeltaH);
	    }
	    
	    // Set the texture
	    GLES20.glVertexAttribPointer(aTexCoordId, TEX_COORDS_PER_VERTEX,
	    							GLES20.GL_FLOAT,
	    							false, 0, texBuf);
	    GLES20.glEnableVertexAttribArray(aTexCoordId);

	    // Draw the quad
	    GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, vertexCount);

	    // Disable arrays
	    GLES20.glDisableVertexAttribArray(aPosId);
	    GLES20.glDisableVertexAttribArray(aTexCoordId);
	    
	    if (renderPass >= 0) {	// drawn to FBO
	    	fbo.unbind();
	    }
    }
	
	public Bitmap getResultBm() {
		return resultBm;
	}
	
	private float saveFrameBufferToBitmap(int w, int h) {
		Log.i(TAG, "Saving frame of size " + w + "x" + h + " to bitmap");
		
		resultBm = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
		
    	IntBuffer resultBuf = ByteBuffer.allocateDirect((int)(w * h * 4))
     		   .order(ByteOrder.nativeOrder()).asIntBuffer();
    	resultBuf.rewind();
    	
    	GLES20.glFinish();
    	
    	t1 = System.nanoTime();
		GLES20.glReadPixels(0, 0, w, h, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, resultBuf);
		t2 = System.nanoTime();
		
//		resultBm.setPixels(resultBuf.array(), 0, w * 4, 0, 0, w, h);
		resultBm.copyPixelsFromBuffer(resultBuf);
		
		return (float)((double)(t2 - t1) / 1000000.0);
	}
	
//    private void printDeltaTimeMs(String s) {
//    	double dtMs = (t2 - t1) / 1000.0  / 1000.0;
//    	Log.i(TAG, "measured ms for task '" + s + "':" + dtMs);
//	}
	
	private static int getShaderParamId(int type, int prog, String p) {
		int id;
		
		id = (type == SHADER_PARAM_TYPE_ATTR) ?
				GLES20.glGetAttribLocation(prog, p) :
				GLES20.glGetUniformLocation(prog, p);
				
		if (id < 0) {
			Log.e(TAG, "Could not get shader param location of type " + type + " and name " + p);
		}
				
		return id;
	}
}
