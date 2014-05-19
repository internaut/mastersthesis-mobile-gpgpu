# Android GPGPU Testing Prototypes

This repository contains some of the prototypes that I developed during my master's thesis -- "Parallel Computing for Digital Signal Processing on Mobile Device GPUs". More information about this thesis and the PDF is available on [my website](http://mkonrad.net/projects/mastersthesis_mobile_gpgpu.html).

These prototypes test different algorithms (e.g. image convolution, Hough transform) with different GPGPU technologies and give report about the execution time of individual processing steps. The following technologies are assessed:

* OpenCL (projects are prefixed with "Cl")
* OpenGL ES 2.0 shaders (projects are prefixed with "Gl")
* Android RenderScript (projects are prefixed with "Rs")

For details and results, please refer to the [project page on my website](http://mkonrad.net/projects/mastersthesis_mobile_gpgpu.html).

Unfortunately, the code is not very well documented because of the general time pressure during writing a master's thesis.

The code has been tested with a Nexus 10 running Android 4.2.1. Please note, that OpenCL support is not available on most devices (see p. 20 of my master's thesis) and requires manual setup (see [this post](http://sweetpea.tentacle.net/blog/opencl-on-nexus-4/) and [that post](http://www.openclblog.com/2013/02/opencl-on-nexus-10-part-1.html)).

# Available Prototypes

* *ClAudio* -- OpenCL-driven simple real-time sound synthesis
* *ClImageProc* -- OpenCL-driven image convolution and Hough transform with kernel generator
* *CvMarkerDetect* -- Marker based augmented reality prototype based on OpenCV and partly accelerated on the GPU via OpenGL ES 2.0 shaders
* *GlImageProc* -- OpenGL ES 2.0 shader driven image convolution with kernel generator (uses Java API)
* *GlImageProcNative* -- OpenGL ES 2.0 shader driven image convolution, thresholding and Parallel Coordinate space based Hough transform (uses NDK API)
* *RsImageProc* -- Android RenderScript driven image convolution (limited functionality, crashes)