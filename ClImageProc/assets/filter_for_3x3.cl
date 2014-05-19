constant float k[9] = {  1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                         2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                         1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f };

__kernel void cl_filter(__read_only image2d_t srcImg,
                              __write_only image2d_t dstImg,
                              sampler_t sampler,
                              int width, int height)
{
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    
    float4 outClr = (float4)(0.0);
    
    for (int kIdx = 0; kIdx < 9; kIdx++) {
        int2 kOff = (int2)(kIdx % 3 - 1, kIdx / 3 - 1);

        outClr += read_imagef(srcImg, sampler, pos + kOff) * k[kIdx];
    }
    
    write_imagef(dstImg, pos, outClr);
}