package net.mkonrad.kernelgen;

public class KernelLangGLSL extends AbstractKernelLang {
	public static final int SHADER_TYPE_VERTEX = 0;
	public static final int SHADER_TYPE_FRAGMENT = 1;
	
	private static final int P_V_TEX_COORD 	= 0;
	private static final int P_U_PX_D 		= 1;
	private static final int P_S_TEXTURE 	= 2;
	private static final int P_A_TEX_COORD 	= 3;
	private static final int P_A_POS 		= 4;
	private static final int P_U_MVPMAT 	= 5;
	private static final int NUM_PARAMS 	= 6;
	
	private static final int V_PX 			= 0;
	private static final int V_CLR 			= 1;
	private static final int NUM_VARS 		= 2;
	
	public KernelLangGLSL(KernelGenerator gen) {
		super(gen);
		
		setDefaults();
	}

	@Override
	public String[] generateKernels() {
		String[] src = new String[2];
		
		src[SHADER_TYPE_VERTEX] = generateVertexShaderCode();
		src[SHADER_TYPE_FRAGMENT] = generateFragmentShaderCode();
		
		return src;
	}

	private String generateVertexShaderCode() {
		final int[] paramIds = {
			P_U_MVPMAT, P_A_POS, P_A_TEX_COORD, P_V_TEX_COORD
		};
		
		String params = generateParamDeclarations(paramIds);
		String body =
				"gl_Position = " + p[P_A_POS] + ";\n" +
			    p[P_V_TEX_COORD] + " = " + p[P_A_TEX_COORD] + ";\n";
		
		String src = srcTemplates[SHADER_TYPE_VERTEX];
		src = src.replace("%PARAMS%", params);
		src = src.replace("%BODY%", body);
		
		return src;
	}

	private String generateFragmentShaderCode() {
		final int[] paramIds = {
			P_S_TEXTURE, P_U_PX_D, P_V_TEX_COORD
		};
		
		String params = generateParamDeclarations(paramIds);
		
		String pxValuesCode = generatePxValuesCode();
		String pxSumCode = generatePxSumCode();
		
		String body = pxValuesCode + "\n\n" +
					  pxSumCode + "\n\n" +
					  "gl_FragColor = " + v[V_CLR] + ";\n";
		
		String src = srcTemplates[SHADER_TYPE_FRAGMENT];
		src = src.replace("%PARAMS%", params);
		src = src.replace("%BODY%", body);
		
		return src;
	}

	private String generatePxValuesCode() {
		final int w = gen.getSizeX();
		final int h = gen.getSizeY();
		final float w_2 = w/2;
		final float h_2 = h/2;
		final float[] kernValues = gen.getValues();
		
		String s = "";
		
		final float norm = gen.getNormFact();
		
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				float kernVal = kernValues[(y * w) + x] * norm;  

				String texCoordPosX = String.valueOf((float)x - w_2);
				texCoordPosX += "*" + p[P_U_PX_D] + ".x";
				String texCoordPosY = String.valueOf((float)y - h_2);
				texCoordPosY += "*" + p[P_U_PX_D] + ".y";
				String texCoordPos = "vec2(" + texCoordPosX + "," + texCoordPosY + ")";
				String texAccess = "texture2D(" + 
								   p[P_S_TEXTURE] +
								   ", " + p[P_V_TEX_COORD] +
								   " + " + texCoordPos + ") * " +
								   kernVal;

				s += vT[V_PX] + " " + generatePxVarName(x, y) + " = " + texAccess  + ";\n";
			}
		}
		
		return s;
	}
	
	private String generatePxSumCode() {
		final int w = gen.getSizeX();
		final int h = gen.getSizeY();
		
		String s = "";
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				s += generatePxVarName(x, y);
				
				if (x == w - 1 && y == h - 1) {
					s += ";";
				} else {
					s += "+";
				}
			}
		}
		
		return vT[V_CLR] + " " + v[V_CLR] + " = " + s;
	}
	
	private String generatePxVarName(int x, int y) {
		return v[V_PX] + String.valueOf(x) + String.valueOf(y); 
	}

	private String generateParamDeclarations(int[] paramIds) {
		String s = "";
		
		for (int pId : paramIds) {
			s += pT[pId] + " " + p[pId] + ";\n";
		}
		
		return s;
	}

	private void setDefaults() {
		srcTemplates = new String[2];
		srcTemplates[SHADER_TYPE_VERTEX] =
				"%PARAMS%\n\n" + 
			    "void main() {\n" +
			    	"%BODY%\n" +
			    "}";
		
		srcTemplates[SHADER_TYPE_FRAGMENT] =
				"precision mediump float;\n\n" +
				"%PARAMS%\n\n" + 
			    "void main() {\n" +
			    	"%BODY%\n" +
			    "}";
		
		pT = new String[NUM_PARAMS];
		p = new String[NUM_PARAMS];
		pT[P_V_TEX_COORD] 	= "varying vec2";		p[P_V_TEX_COORD] 	= "vTexCoord";
		pT[P_U_PX_D] 		= "uniform vec2";		p[P_U_PX_D] 		= "uPxD";
		pT[P_S_TEXTURE] 	= "uniform sampler2D";	p[P_S_TEXTURE]	 	= "sTexture";
		pT[P_A_TEX_COORD] 	= "attribute vec2";		p[P_A_TEX_COORD] 	= "aTexCoord";
		pT[P_A_POS] 		= "attribute vec4";		p[P_A_POS] 			= "aPos";
		pT[P_U_MVPMAT] 		= "uniform mat4";		p[P_U_MVPMAT] 		= "uMVPMatrix";
		
		vT = new String[NUM_VARS];
		v = new String[NUM_VARS];
		vT[V_PX] 			= "vec4";				v[V_PX] 			= "px";
		vT[V_CLR] 			= "vec4";				v[V_CLR] 			= "clr";
	}
}
