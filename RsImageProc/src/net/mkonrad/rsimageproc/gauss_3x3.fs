#pragma version(1)
#pragma rs java_package_name(net.mkonrad.rsimageproc)

static const float k[9] = { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                         	2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                         	1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f };
                         	
rs_allocation in;

// git commit 53eef6c had a version using plain uchar4 instead of float4
// however, this version introduced artefacts and was not really faster
                         	
uchar4 __attribute__((kernel)) make_gauss(uint32_t x, uint32_t y) {
	float4 p00 = convert_float4(rsGetElementAt_uchar4(in, x - 1, y - 1)) * k[0];
	float4 p10 = convert_float4(rsGetElementAt_uchar4(in, x    , y - 1)) * k[1];
	float4 p20 = convert_float4(rsGetElementAt_uchar4(in, x + 1, y - 1)) * k[2];
	float4 p01 = convert_float4(rsGetElementAt_uchar4(in, x - 1, y    )) * k[3];
	float4 p11 = convert_float4(rsGetElementAt_uchar4(in, x    , y    )) * k[4];
	float4 p21 = convert_float4(rsGetElementAt_uchar4(in, x + 1, y    )) * k[5];
	float4 p02 = convert_float4(rsGetElementAt_uchar4(in, x - 1, y + 1)) * k[6];
	float4 p12 = convert_float4(rsGetElementAt_uchar4(in, x    , y + 1)) * k[7];
	float4 p22 = convert_float4(rsGetElementAt_uchar4(in, x + 1, y + 1)) * k[8];
	
	float4 f4out = p00 + p10 + p20
			     + p01 + p11 + p21
			     + p02 + p12 + p22;
	
	return convert_uchar4(f4out);
}