#pragma version(1)
#pragma rs java_package_name(net.mkonrad.rsimageproc)

static const float k[9] = { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                         	2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                         	1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f };
                         	
rs_allocation in;

// git commit 53eef6c had a version using plain uchar4 instead of float4
// however, this version introduced artefacts and was not really faster
                         	
uchar4 __attribute__((kernel)) make_gauss(uint32_t x, uint32_t y) {
	float4 f4out = { 0, 0, 0, 0};

	for (int kIdx = 0; kIdx < 9; kIdx++) {
		int kX = kIdx % 3 - 1;
		int kY = kIdx / 3 - 1;
	
		f4out += convert_float4(rsGetElementAt_uchar4(in, x + kX, y - kY)) * k[kIdx];
		
	}
	
	return convert_uchar4(f4out);
}