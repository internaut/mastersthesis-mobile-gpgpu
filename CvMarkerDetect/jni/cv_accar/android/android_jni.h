#ifndef CV_ACCAR_ANDROID_JNI_H
#define CV_ACCAR_ANDROID_JNI_H

extern "C" {

void cv_accar_init(int camId, long long camIntrinsicsAddr);

void cv_accar_set_dbg_input_frame(long long inFrameAddr);
void cv_accar_set_dbg_output_mode(int mode);
void cv_accar_set_dbg_marker_shader(char *vshSrc, char *fshSrc);

void cv_accar_set_output_display_shader(char *vshSrc, char *fshSrc);
void cv_accar_add_shader_to_pipeline(char *vshSrc, char *fshSrc);

void cv_accar_start();
void cv_accar_stop();
void cv_accar_pause();
void cv_accar_resume();

void cv_accar_view_create();
void cv_accar_view_resize(int w, int h);
void cv_accar_view_draw();

void cv_accar_get_detected_markers();

void cv_accar_cleanup();

};

#endif
