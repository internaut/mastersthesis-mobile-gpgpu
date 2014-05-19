package net.mkonrad.kernelgen;

public class KernelLangCL extends AbstractKernelLang {
	private static final int P_SRC_IMG 	= 0;
	private static final int P_DST_IMG 	= 1;
	private static final int P_SAMPLER 	= 2;
	private static final int P_IMG_W 	= 3;
	private static final int P_IMG_H 	= 4;
	private static final int NUM_PARAMS = 5;
	
	private static final int V_POS 	= 0;
	private static final int V_PX 	= 1;
	private static final int V_RES 	= 2;
	private static final int NUM_VARS = 3;

	public KernelLangCL(KernelGenerator gen) {
		super(gen);
		
		setDefaults();
	}

	@Override
	public String[] generateKernels() {
		String[] src = new String[2];
		
		src[0] = generateCLKernelCode();
		
		return src;
	}

	private void setDefaults() {
		// main template
		
		srcTemplates = new String[1];
		
		srcTemplates[0] =
			    "__kernel void %NAME%(%PARAMS%) {\n" +
			    	"%BODY%\n" +
			    "}";
		
		// parameter types and names
		
		pT = new String[NUM_PARAMS];
		p = new String[NUM_PARAMS];
		
		pT[P_SRC_IMG] 	= "__read_only image2d_t"; 		p[P_SRC_IMG] 	= "srcImg";
		pT[P_DST_IMG] 	= "__write_only image2d_t"; 	p[P_DST_IMG] 	= "dstImg";
		pT[P_SAMPLER] 	= "sampler_t"; 					p[P_SAMPLER] 	= "sampler";
		pT[P_IMG_W] 	= "int"; 						p[P_IMG_W] 		= "width";
		pT[P_IMG_H] 	= "int"; 						p[P_IMG_H] 		= "height";
		
		// variable types and names
		
		vT = new String[NUM_VARS];
		v = new String[NUM_VARS];
		
		vT[V_POS]		= "int2";						v[V_POS]		= "pos";
		vT[V_PX]		= "float4";						v[V_PX]			= "px";
		vT[V_RES]		= "float4";						v[V_RES]		= "resPx";
	}
	

	private String generateCLKernelCode() {
		int[] paramIds = {P_SRC_IMG, P_DST_IMG, P_SAMPLER, P_IMG_W, P_IMG_H};
		String params = generateParamDeclarations(paramIds);
		String body = generateBodyStatements();
		
		String src = srcTemplates[0];
		src = src.replace("%NAME%", gen.getName());
		src = src.replace("%PARAMS%", params);
		src = src.replace("%BODY%", body);
		
		return src;
	}

	private String generateParamDeclarations(int[] paramIds) {
		String src = "";
		
		for (int i = 0; i < paramIds.length; i++) {
			src += pT[i] + " " + p[i];
			
			if (i < paramIds.length - 1) {
				src += ", ";
			}
		}
		
		return src;
	}
	
	private String generateBodyStatements() {
		String src = vT[V_POS] + " " + v[V_POS] + " = " + "(int2)(get_global_id(0), get_global_id(1));\n";
		
		src += generatePxValuesCode();
		src += generatePxSumCode();
		
		src += "write_imagef(" + p[P_DST_IMG] + ", " + v[V_POS] + ", " + v[V_RES] + ");\n";
		
		return src;
	}
	
	private String generatePxValuesCode() {
		String s = "";
		
		final int w = gen.getSizeX();
		final int h = gen.getSizeY();
		final float w_2 = w/2;
		final float h_2 = h/2;
		final float[] kernValues = gen.getValues();
		
		final float norm = gen.getNormFact();
		
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				float kernVal = kernValues[(y * w) + x] * norm;  

				String texCoordPosX = Integer.toString((int)(x - w_2));
				String texCoordPosY = Integer.toString((int)(y - h_2));
				String texCoordPos = texCoordPosX + "," + texCoordPosY;
				String texAccess = "read_imagef(" + 
								   		p[P_SRC_IMG] + ", " +
								   		p[P_SAMPLER] + ", " +
								   		v[V_POS] + " + (int2)(" + texCoordPos + ")) * " +
								   kernVal + "f";

				s += vT[V_PX] + " " + generatePxVarName(x, y) + " = " + texAccess  + ";\n";
			}
		}
		
		return s + "\n";
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
		
		return vT[V_RES] + " " + v[V_RES] + " = " + s;
	}

	private String generatePxVarName(int x, int y) {
		return v[V_PX] + String.valueOf(x) + String.valueOf(y); 
	}
}
