package net.mkonrad.glimageproc;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.content.Context;
import android.opengl.GLES20;
import android.util.Log;

public class GLShader {
	private static final String TAG = "GlImageProc::GLShader";
	private Context ctx;
	
	private int program;
	private int vertexShader;
	private int fragShader;
	
	public GLShader(Context ctx, int vShaderSrcFileId, int fShaderSrcFileId) {
		this.ctx = ctx;
		
		createShaders(vShaderSrcFileId, fShaderSrcFileId);
		
		Log.i(TAG, "Shaders created from files!");
	}
	
	public GLShader(Context ctx, String vShaderSrcCode, String fShaderSrcCode) {
		this.ctx = ctx;
		
		createShaders(vShaderSrcCode, fShaderSrcCode);
		
		Log.i(TAG, "Shader created from source code!");
	}
	
	public void use() {
		GLES20.glUseProgram(program);
	}
	
	public int getProgram() {
		return program;
	}
	
	private void createShaders(int vShaderSrcFileId, int fShaderSrcFileId) {
		String vShaderSrc = loadStringsFromFile(ctx, vShaderSrcFileId);
		String fShaderSrc = loadStringsFromFile(ctx, fShaderSrcFileId);
		
		createShaders(vShaderSrc, fShaderSrc);
	}

	private void createShaders(String vShaderSrc, String fShaderSrc) {
		vertexShader = compileShaderSrc(GLES20.GL_VERTEX_SHADER, vShaderSrc);
		fragShader = compileShaderSrc(GLES20.GL_FRAGMENT_SHADER, fShaderSrc);
		
		program = GLES20.glCreateProgram();             // create empty OpenGL ES Program
	    if (program == 0) {
	    	Log.e(TAG, "Error creating shader program");
	        return;
	    }
	    
	    GLES20.glAttachShader(program, vertexShader);   // add the vertex shader to program
	    GLES20.glAttachShader(program, fragShader); // add the fragment shader to program
	    GLES20.glLinkProgram(program);                  // creates OpenGL ES program executables
	    
        int[] linkStatus = new int[1];
        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
        if (linkStatus[0] != GLES20.GL_TRUE) {
            Log.e(TAG, "Could not link shader program: ");
            Log.e(TAG, GLES20.glGetProgramInfoLog(program));
            GLES20.glDeleteProgram(program);
            program = 0;
        }
	}
	
	private static int compileShaderSrc(int type, String src) {
	    int shader = GLES20.glCreateShader(type);
	    
	    if(shader == 0) {
	    	Log.e(TAG, "Error creating shader");
	        return 0;
	    }

	    // add the source code to the shader and compile it
	    GLES20.glShaderSource(shader, src);
	    GLES20.glCompileShader(shader);
	    int[] compiled = new int[1];
	    GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
	    
        if (compiled[0] == 0) {
            Log.e(TAG, "Could not compile shader " + type + ":");
            Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
            GLES20.glDeleteShader(shader);
            shader = 0;
        } else {
        	Log.i(TAG, "Compiled shader of type " + type + " with source length " + src.length());
        }

	    return shader;
	}
	
	private static String loadStringsFromFile(Context ctx, int fileId) {
		StringBuilder strBuilder = new StringBuilder();
		try {
			// read the vertex shader source from file
			InputStream inputStream = ctx.getResources()
					.openRawResource(fileId);
			BufferedReader in = new BufferedReader(new InputStreamReader(
					inputStream));
			String line = in.readLine();
			while (line != null) {
				strBuilder.append(line).append("\n");
				line = in.readLine();
			}
			in.close();
		} catch (IOException e) {
			Log.e(TAG, "Error reading text file: " + e.getLocalizedMessage());
		}

		return strBuilder.toString();
	}
}
