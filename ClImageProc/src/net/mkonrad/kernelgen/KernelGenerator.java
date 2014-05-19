package net.mkonrad.kernelgen;

import java.util.Arrays;

public class KernelGenerator {
	public enum KernelLang {
		GLSL,
		CL,
		RS
	}

//	private KernelLang lang;
	private String name;
	private int sizeX;
	private int sizeY;
	private float[] values;
	private float normFact;	// normalizing factor
	
	public void setKernel(String n, int w, int h, float[] v) {
		name = n;
		sizeX = w;
		sizeY = h;
		values = Arrays.copyOf(v, v.length);
		
		float kernSum = arraySum(v);
		if (kernSum != 0) {
			normFact = 1.0f / kernSum;
		} else {
			normFact = 1.0f;
		}
	}
	
	public String[] getKernelCodeStrings(KernelLang lang) {
		AbstractKernelLang kernelLangGen = null;
		
		switch (lang) {
		case GLSL:
			kernelLangGen = new KernelLangGLSL(this);
			break;
			
		case CL:
			kernelLangGen = new KernelLangCL(this);
			break;

		default:
			break;
		}
		
		if (kernelLangGen != null) {
			return kernelLangGen.generateKernels();
		} else {
			return null;
		}
	}
	
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public int getSizeX() {
		return sizeX;
	}

	public int getSizeY() {
		return sizeY;
	}

	public float[] getValues() {
		return values;
	}

	public float getNormFact() {
		return normFact;
	}

	static private float arraySum(float[] v) {
		float s = 0.0f;
		
		for (int i = 0; i < v.length; i++) {
			s += v[i];
		}
		
		return s;
	}
}
