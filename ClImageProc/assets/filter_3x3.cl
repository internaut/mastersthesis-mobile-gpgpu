constant float k[9] = {  1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                         2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                         1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f };

__kernel void cl_filter(__read_only image2d_t srcImg,
                              __write_only image2d_t dstImg,
                              sampler_t sampler,
                              int width, int height)
{
    int2 p = (int2)(get_global_id(0), get_global_id(1));
    
    float4 p00 = read_imagef(srcImg, sampler, p + (int2)(-1, -1)) * k[0];
    float4 p01 = read_imagef(srcImg, sampler, p + (int2)( 0, -1)) * k[1];
    float4 p02 = read_imagef(srcImg, sampler, p + (int2)( 1, -1)) * k[2];
    float4 p10 = read_imagef(srcImg, sampler, p + (int2)(-1,  0)) * k[3];
    float4 p11 = read_imagef(srcImg, sampler, p                 ) * k[4];
    float4 p12 = read_imagef(srcImg, sampler, p + (int2)( 1,  0)) * k[5];
    float4 p20 = read_imagef(srcImg, sampler, p + (int2)(-1,  1)) * k[6];
    float4 p21 = read_imagef(srcImg, sampler, p + (int2)( 0,  1)) * k[7];
    float4 p22 = read_imagef(srcImg, sampler, p + (int2)( 1,  1)) * k[8];
    
    float4 outClr = p00 + p01 + p02
                  + p10 + p11 + p12
                  + p20 + p21 + p22;
    write_imagef(dstImg, p, outClr);
}