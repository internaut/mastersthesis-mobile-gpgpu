constant float k[49] = {
    1.0/307.0, 2.0/307.0, 4.0/307.0, 7.0/307.0, 4.0/307.0, 2.0/307.0, 1.0/307.0,
    2.0/307.0, 4.0/307.0, 7.0/307.0,10.0/307.0, 7.0/307.0, 4.0/307.0, 2.0/307.0,
    4.0/307.0, 7.0/307.0,10.0/307.0,14.0/307.0,10.0/307.0, 7.0/307.0, 4.0/307.0,
    7.0/307.0,10.0/307.0,14.0/307.0,19.0/307.0,14.0/307.0,10.0/307.0, 7.0/307.0,
    4.0/307.0, 7.0/307.0,10.0/307.0,14.0/307.0,10.0/307.0, 7.0/307.0, 4.0/307.0,
    2.0/307.0, 4.0/307.0, 7.0/307.0,10.0/307.0, 7.0/307.0, 4.0/307.0, 2.0/307.0,
    1.0/307.0, 2.0/307.0, 4.0/307.0, 7.0/307.0, 4.0/307.0, 2.0/307.0, 1.0/307.0
};
__kernel void cl_filter(__read_only image2d_t src,__write_only image2d_t dst,sampler_t sampler,int width, int height) {
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float4 outClr = (float4)(0.0);
    for (int kIdx = 0; kIdx < 49; kIdx++) {
        int2 kOff = (int2)(kIdx % 7 - 3, kIdx / 7 - 3);
        outClr += read_imagef(src, sampler, pos + kOff) * k[kIdx];
    }
    write_imagef(dst, pos, outClr);
}