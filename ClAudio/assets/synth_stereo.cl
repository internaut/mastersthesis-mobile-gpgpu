constant float TWOPI = M_PI_F * 2.0f;
constant float phStepL = (439.0f * M_PI_F) / 44100.0f;
constant float phStepR = (441.0f * M_PI_F) / 44100.0f;

kernel void cl_synth(int bufOffset, global float *buf) {
    const int gid = get_global_id(0);

    buf[gid] = ((gid + 1) % 2)  * sin((bufOffset + gid)     * phStepL)   // left chan.
             +  (gid % 2)       * sin((bufOffset + gid - 1) * phStepR);  // right chan.
}