package net.mkonrad.cvaccar;

import java.io.IOException;
import java.io.InputStream;

import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.content.Context;
import android.graphics.Bitmap;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.util.Xml;

public class CvAccARDroid {
	final private static String TAG = "CvAccARLib";

	private GLSurfaceView glView;
	private Context ctx;

	private String camIntrinsicsXMLFile = null;
	private int camId;
	private Mat dbgFrame = null;
	private boolean dbgMarkerDisplay = false;

	public CvAccARDroid(Context c) {
		ctx = c;
		CvAccARTools.ctx = ctx;
		
		glView = new GLSurfaceView(c);
		glView.setEGLContextClientVersion(2);
		glView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
		glView.setRenderer(new CvAccARRenderer(this));
	}

	public GLSurfaceView getGLView() {
		return glView;
	}
	
	public void drawDbgMarkers(boolean status) {
		dbgMarkerDisplay = status;
	}

	public void setOutputMode(int mode) {
		cv_accar.cv_accar_set_dbg_output_mode(mode);
		
		glView.requestRender();
	}

	public void setInput(int cameraId, String camIntrXMLFile) {
		glView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
		
		camId = cameraId;
		camIntrinsicsXMLFile = camIntrXMLFile;
	}
	
	public void setInput(Bitmap dbgImg) {
		setInput(dbgImg, null, true);
	}

	public void setInput(Bitmap dbgImg, String camIntrXMLFile, boolean renderModeDirty) {
		glView.setRenderMode(renderModeDirty ? GLSurfaceView.RENDERMODE_WHEN_DIRTY : GLSurfaceView.RENDERMODE_CONTINUOUSLY);
		
		camId = -1;
		camIntrinsicsXMLFile = camIntrXMLFile;

		dbgFrame = new Mat();
		Utils.bitmapToMat(dbgImg, dbgFrame);
	}

	public void start() throws IOException, XmlPullParserException {
		long camIntrMatAddr = 0x00;

		if (camIntrinsicsXMLFile != null) {
			Mat camIntrMat = CvAccARTools.getCamIntrinsicsFromFile(camIntrinsicsXMLFile);
			camIntrMatAddr = camIntrMat.getNativeObjAddr();
		}

		cv_accar.cv_accar_init(camId, camIntrMatAddr);
		
		if (camId < 0 && dbgFrame != null) {
			cv_accar.cv_accar_set_dbg_input_frame(dbgFrame.getNativeObjAddr());
		}
		
		cv_accar.cv_accar_start();
	}

	public void pause() {
		cv_accar.cv_accar_pause();
		glView.onPause();
	}

	public void resume() {
		cv_accar.cv_accar_resume();
		glView.onResume();
	}

	public void stop() {
		cv_accar.cv_accar_stop();
		cv_accar.cv_accar_cleanup();
	}
	
	public void addShaders() {
		// load debugging display shaders
		if (dbgMarkerDisplay) {
			String vshSrcMarker = CvAccARTools.loadStringsFromFile("shaders/marker_v.glsl");
			String fshSrcMarker = CvAccARTools.loadStringsFromFile("shaders/marker_f.glsl");
			
			cv_accar.cv_accar_set_dbg_marker_shader(vshSrcMarker, fshSrcMarker);
		}
		
		
		// set display shader
		String vshSrcDisp = CvAccARTools.loadStringsFromFile("shaders/disp_v.glsl");
		String fshSrcDisp = CvAccARTools.loadStringsFromFile("shaders/disp_f.glsl");
		
		cv_accar.cv_accar_set_output_display_shader(vshSrcDisp, fshSrcDisp);
		
		// load shaders for pipeline
		
		// 1. preprocessing
		String vshSrcFilter = CvAccARTools.loadStringsFromFile("shaders/filter_v.glsl");
		String fshSrcPreproc = CvAccARTools.loadStringsFromFile("shaders/preproc_f.glsl");
		
		cv_accar.cv_accar_add_shader_to_pipeline(vshSrcFilter, fshSrcPreproc);
		
		// 2. adaptive threshold - pass 1 & 2
		String fshSrcAThresh1 = CvAccARTools.loadStringsFromFile("shaders/athresh1_f.glsl");
		String fshSrcAThresh2 = CvAccARTools.loadStringsFromFile("shaders/athresh2_f.glsl");
		
		cv_accar.cv_accar_add_shader_to_pipeline(vshSrcFilter, fshSrcAThresh1);
		cv_accar.cv_accar_add_shader_to_pipeline(vshSrcFilter, fshSrcAThresh2);
		
		// 3. marker warp
		String vshSrcMWarp = CvAccARTools.loadStringsFromFile("shaders/markerwarp_v.glsl");
		String fshSrcMWarp = CvAccARTools.loadStringsFromFile("shaders/markerwarp_f.glsl");
		cv_accar.cv_accar_add_shader_to_pipeline(vshSrcMWarp, fshSrcMWarp);		
		
		// 4. histogram
//		String vshSrcHist = CvAccARTools.loadStringsFromFile("shaders/hist_v.glsl");
//		String fshSrcHist = CvAccARTools.loadStringsFromFile("shaders/hist_f.glsl");
//		cv_accar.cv_accar_add_shader_to_pipeline(vshSrcHist, fshSrcHist);
//		
//		// 5. simple thresholding
//		String fshSrcThresh = CvAccARTools.loadStringsFromFile("shaders/thresh_f.glsl");
//		cv_accar.cv_accar_add_shader_to_pipeline(vshSrcFilter, fshSrcThresh);
		
		// 6. pclines / hough
//		String vshSrcPCLines = CvAccARTools.loadStringsFromFile("shaders/pclines_v.glsl");
//		String fshSrcPCLines = CvAccARTools.loadStringsFromFile("shaders/pclines_f.glsl");
//		cv_accar.cv_accar_add_shader_to_pipeline(vshSrcPCLines, fshSrcPCLines);
	}
}
