%module cl_audio
%{
#include "cl_audio.h"
%}

// Enable the JNI class to load the required native library.
// And also the GLES_mali library for OpenCL!
%pragma(java) jniclasscode=%{
  static {
    try {
    	System.load("/system/vendor/lib/egl/libGLES_mali.so");
        java.lang.System.loadLibrary("cl_audio");
    } catch (UnsatisfiedLinkError e) {
        java.lang.System.err.println("native code library failed to load.\n" + e);
        java.lang.System.exit(1);
    }
  }
%}

%include "cl_audio.h"