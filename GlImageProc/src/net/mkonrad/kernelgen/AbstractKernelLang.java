package net.mkonrad.kernelgen;

public abstract class AbstractKernelLang {
	protected String[] srcTemplates;
	protected String[] pT;	// parameter types
	protected String[] p;	// parameter names
	protected String[] vT;	// variable types
	protected String[] v;	// variable names
	
	protected KernelGenerator gen;
	
	public AbstractKernelLang(KernelGenerator gen) {
		this.gen = gen;
	}
	
	abstract public String[] generateKernels(); 
}
