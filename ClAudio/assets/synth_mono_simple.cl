constant float TWOPI = M_PI_F * 2.0f;
constant float phStepR = (440.0f * M_PI_F) / 44100.0f;

kernel void cl_synth(int bufOffset, global float *buf) {
    const int gid = get_global_id(0);

    buf[gid] = (gid % 2) * sin((float)(bufOffset + gid - 1) * phStepR);  // right chan. only
}