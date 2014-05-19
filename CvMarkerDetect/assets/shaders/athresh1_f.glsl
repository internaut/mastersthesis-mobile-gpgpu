// Adaptive thresholding - Pass 1
// Perform a vertical 7x1 or 5x1 average gray pixel value calculation
//
// Requires a grayscale image as input!

precision mediump float;

#define BLOCKSIZE 5
//#define USE_GAUSS_5

varying vec2 vTexCoord;

uniform vec2 uPxD;           // pixel delta values
uniform sampler2D sTexture;

// Fragment shader main routine
void main() {
    // get center pixel value
    float centerGray = texture2D(sTexture, vTexCoord).r;
    
    // get the sum
#if BLOCKSIZE == 7
    float sum = texture2D(sTexture, vTexCoord + vec2(uPxD.x * -3.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x * -2.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x * -1.0, 0.0)).r +
                centerGray +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x *  1.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x *  2.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x *  3.0, 0.0)).r;
    
    // get the average
    float avg = sum / 7.0;
#else
    
#ifdef USE_GAUSS_5
    float avg = 0.0625 * texture2D(sTexture, vTexCoord + vec2(uPxD.x * -2.0, 0.0)).r +
                0.25   * texture2D(sTexture, vTexCoord + vec2(uPxD.x * -1.0, 0.0)).r +
                0.375  * centerGray +
                0.25   * texture2D(sTexture, vTexCoord + vec2(uPxD.x *  1.0, 0.0)).r +
                0.0625 * texture2D(sTexture, vTexCoord + vec2(uPxD.x *  2.0, 0.0)).r;
#else
    float sum = texture2D(sTexture, vTexCoord + vec2(uPxD.x * -2.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x * -1.0, 0.0)).r +
                centerGray +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x *  1.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.x *  2.0, 0.0)).r;
    
    // get the average
    float avg = sum / 5.0;
#endif
    
#endif
    
    // Result stores average pixel value (R) and original gray value (G)
    gl_FragColor = vec4(avg, centerGray, 0.0, 1.0);
}