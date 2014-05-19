constant float k[25] = {
    1.0f / 90.0f, 2.0f / 90.0f, 4.0f / 90.0f, 2.0f / 90.0f, 1.0f / 90.0f,
    2.0f / 90.0f, 4.0f / 90.0f, 7.0f / 90.0f, 4.0f / 90.0f, 2.0f / 90.0f,
    4.0f / 90.0f, 7.0f / 90.0f, 10f  / 90.0f, 7.0f / 90.0f, 4.0f / 90.0f,
    2.0f / 90.0f, 4.0f / 90.0f, 7.0f / 90.0f, 4.0f / 90.0f, 2.0f / 90.0f,
    1.0f / 90.0f, 2.0f / 90.0f, 4.0f / 90.0f, 2.0f / 90.0f, 1.0f / 90.0f
};

__kernel void cl_filter(__read_only image2d_t srcImg,
                              __write_only image2d_t dstImg,
                              sampler_t sampler,
                              int width, int height)
{
    int2 p = (int2)(get_global_id(0), get_global_id(1));
    
    float4 p00 = read_imagef(srcImg, sampler, p + (int2)(-2, -2)) * k[0];
    float4 p01 = read_imagef(srcImg, sampler, p + (int2)(-1, -2)) * k[1];
    float4 p02 = read_imagef(srcImg, sampler, p + (int2)( 0, -2)) * k[2];
    float4 p03 = read_imagef(srcImg, sampler, p + (int2)( 1, -2)) * k[3];
    float4 p04 = read_imagef(srcImg, sampler, p + (int2)( 2, -2)) * k[4];
    
    float4 p10 = read_imagef(srcImg, sampler, p + (int2)(-2, -1)) * k[5];
    float4 p11 = read_imagef(srcImg, sampler, p + (int2)(-1, -1)) * k[6];
    float4 p12 = read_imagef(srcImg, sampler, p + (int2)( 0, -1)) * k[7];
    float4 p13 = read_imagef(srcImg, sampler, p + (int2)( 1, -1)) * k[8];
    float4 p14 = read_imagef(srcImg, sampler, p + (int2)( 2, -1)) * k[9];
    
    float4 p20 = read_imagef(srcImg, sampler, p + (int2)(-2,  0)) * k[10];
    float4 p21 = read_imagef(srcImg, sampler, p + (int2)(-1,  0)) * k[11];
    float4 p22 = read_imagef(srcImg, sampler, p                 ) * k[12];
    float4 p23 = read_imagef(srcImg, sampler, p + (int2)( 1,  0)) * k[13];
    float4 p24 = read_imagef(srcImg, sampler, p + (int2)( 2,  0)) * k[14];
    
    float4 p30 = read_imagef(srcImg, sampler, p + (int2)(-2,  1)) * k[15];
    float4 p31 = read_imagef(srcImg, sampler, p + (int2)(-1,  1)) * k[16];
    float4 p32 = read_imagef(srcImg, sampler, p + (int2)( 0,  1)) * k[17];
    float4 p33 = read_imagef(srcImg, sampler, p + (int2)( 1,  1)) * k[18];
    float4 p34 = read_imagef(srcImg, sampler, p + (int2)( 2,  1)) * k[19];
    
    float4 p40 = read_imagef(srcImg, sampler, p + (int2)(-2,  2)) * k[20];
    float4 p41 = read_imagef(srcImg, sampler, p + (int2)(-1,  2)) * k[21];
    float4 p42 = read_imagef(srcImg, sampler, p + (int2)( 0,  2)) * k[22];
    float4 p43 = read_imagef(srcImg, sampler, p + (int2)( 1,  2)) * k[23];
    float4 p44 = read_imagef(srcImg, sampler, p + (int2)( 2,  2)) * k[24];
    
    float4 outClr = p00 + p01 + p02 + p03 + p04
                  + p10 + p11 + p12 + p13 + p14
                  + p20 + p21 + p22 + p23 + p24
                  + p30 + p31 + p32 + p33 + p34
                  + p40 + p41 + p42 + p43 + p44
    
    write_imagef(dstImg, p, outClr);
}