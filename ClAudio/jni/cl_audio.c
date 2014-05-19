#include <math.h>
#include <time.h>
#include "opensl_io.h"

#include "tools.h"

// CONFIGURE HERE:

#define USE_OPENCL
#define MEASURE_TIME 100
//#define PRINT_BUFF_VALUES 32

#define BUFFERFRAMES (4096 * 2 * 2)
#define VECSAMPS_STEREO BUFFERFRAMES
#define SR 44100

// Include OpenCL stuff on demand

#ifdef USE_OPENCL
#include "cl_manager.h"
#endif

static int on;

#ifdef MEASURE_TIME
static clock_t t1, t2;
static int tCount = 0;
static clock_t tDeltaSum = 0;

void print_exec_time(clock_t t, const char *msg) {
	LOGINFO("%s,%d,%f\n", msg, BUFFERFRAMES, ((double)t / CLOCKS_PER_SEC) * 1000.0);
}
#endif

#ifdef USE_OPENCL

static int bufOffset = 0;

static inline void calcBuffer(float *buf, unsigned int bufSamples) {
	int i;
#ifdef MEASURE_TIME
	t1 = clock();
#endif

	cl_mgr_exec_kernel();
	cl_mgr_get_result((void *)buf);
	cl_mgr_update_args(bufOffset);
	bufOffset += bufSamples;

#ifdef MEASURE_TIME
	t2 = clock();
#endif

#ifdef PRINT_BUFF_VALUES
	for (i = 0; i < PRINT_BUFF_VALUES / 2; i++) {
		LOGINFO("buf[%d]=%f", i, buf[i]);
	}
	for (i = bufSamples - PRINT_BUFF_VALUES / 2; i < bufSamples; i++) {
		LOGINFO("buf[%d]=%f", i, buf[i]);
	}
#endif
}

#else

static const float TWOPI = M_PI * 2.0f;

static float phaseL = 0.0f;
static float phaseR = 0.0f;
static const float phStepL = 439.0f * M_PI * 2.0f / (float)SR;
static const float phStepR = 441.0f * M_PI * 2.0f / (float)SR;

static inline void calcBuffer(float *buf, unsigned int bufSamples) {
	int i;
#ifdef MEASURE_TIME
	t1 = clock();
#endif

	for(i = 0; i < bufSamples; i+=2) {
		buf[i] 		= sinf(phaseL);
		buf[i+1]	= sinf(phaseR);
		phaseL = fmodf(phaseL + phStepL, TWOPI);
		phaseR = fmodf(phaseR + phStepR, TWOPI);
	}

#ifdef MEASURE_TIME
	t2 = clock();
#endif

#ifdef PRINT_BUFF_VALUES
	for (i = 0; i < PRINT_BUFF_VALUES / 2; i++) {
		LOGINFO("buf[%d]=%f", i, buf[i]);
	}
	for (i = bufSamples - PRINT_BUFF_VALUES / 2; i < bufSamples; i++) {
		LOGINFO("buf[%d]=%f", i, buf[i]);
	}
#endif
}

#endif	// #ifdef USE_OPENCL

void start_process() {
#ifdef USE_OPENCL
  LOGINFO("Starting sound using OpenCL!");
#else
  LOGINFO("Starting sound using CPU!");
#endif

  OPENSL_STREAM  *p;
  int i;
  float  outbuffer[VECSAMPS_STEREO];
  p = android_OpenAudioDevice(SR,0,2,BUFFERFRAMES);
  if(p == NULL) return;

  on = 1;

  while(on) {
	  calcBuffer(outbuffer, VECSAMPS_STEREO);
#ifdef MEASURE_TIME
	  if (tCount < MEASURE_TIME) {
		  tDeltaSum += (t2 - t1) / MEASURE_TIME;
		  tCount++;
	  } else {
		  print_exec_time(tDeltaSum, "gen-buffer");
		  tDeltaSum = 0;
		  tCount = 0;
	  }
#endif
	  android_AudioOut(p,outbuffer,VECSAMPS_STEREO);
  }
  android_CloseAudioDevice(p);
}

void stop_process() {
  on = 0;
}

void create_cl_kernel_from_src(char *src) {
	LOGINFO("Creating CL kernel from source:");
	LOGINFO("%s", src);

	LOGINFO("Setting result buffer size to %d samples", VECSAMPS_STEREO);

	cl_mgr_init();
	cl_mgr_create_kernel_from_src(src);
	cl_mgr_create_result_buf(VECSAMPS_STEREO);
}

void cleanup() {
	LOGINFO("Cleaning up CL stuff...");

	cl_mgr_release();
}
