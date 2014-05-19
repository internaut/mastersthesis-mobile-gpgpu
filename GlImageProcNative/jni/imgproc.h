#ifndef IMGPROC_H
#define IMGPROC_H

extern "C" {

void init();

void glview_create();

void glview_resize(int w, int h);

void glview_draw();

void set_input_image(long long inputImgAddr);

void get_output_image(long long outputImgAddr);

void cleanup();

float get_image_push_ms();
float get_render_ms();
float get_image_pull_ms();

};

#endif
