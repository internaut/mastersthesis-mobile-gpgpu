#ifndef CL_MANAGER_H
#define CL_MANAGER_H

// OpenCL library
#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif
  int cl_mgr_init();
  int cl_mgr_create_kernel_from_src(const char *src);
  int cl_mgr_create_result_buf(size_t bufSize);
  void cl_mgr_update_args(int bufOffset);
  void cl_mgr_exec_kernel();
  void cl_mgr_get_result(void *resBuf);
  void cl_mgr_release();
#ifdef __cplusplus
};
#endif

#endif
