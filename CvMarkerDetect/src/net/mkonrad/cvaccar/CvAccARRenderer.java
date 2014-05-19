package net.mkonrad.cvaccar;

import static android.opengl.GLES20.*;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView.Renderer;

public class CvAccARRenderer implements Renderer {
	CvAccARDroid arLib;
	
	public CvAccARRenderer(CvAccARDroid coreLib) {
		arLib = coreLib;
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		glClear(GL_COLOR_BUFFER_BIT);
		cv_accar.cv_accar_view_draw();
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int w, int h) {
		cv_accar.cv_accar_view_resize(w, h);
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig conf) {
		arLib.addShaders();
		cv_accar.cv_accar_view_create();
	}

}
