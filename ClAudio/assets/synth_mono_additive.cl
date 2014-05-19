constant float TWOPI = M_PI_F * 2.0f;
constant float phStepBase = (440.0f * M_PI_F) / 44100.0f;
constant float phStepAdd1 = 3.0f * phStepBase;
constant float phStepAdd2 = 5.0f * phStepBase;
constant float phStepAdd3 = 7.0f * phStepBase;

kernel void cl_synth(int bufOffset, global float *buf) {
    const int gid = get_global_id(0);
    const float p = (float)(bufOffset + gid - 1);    // phase position
    buf[gid] = (gid % 2) * (  sin(p * phStepBase)
                            + sin(p * phStepAdd1) / 3.0f
                            + sin(p * phStepAdd2) / 5.0f
                            + sin(p * phStepAdd3) / 7.0f);  // right chan. only
}