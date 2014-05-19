#ifndef CL_AUDIO_H
#define CL_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif
  void start_process();
  void stop_process();
  void create_cl_kernel_from_src(char *src);
  void cleanup();
#ifdef __cplusplus
};
#endif

#endif
