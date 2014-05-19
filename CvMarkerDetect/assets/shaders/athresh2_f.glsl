// Adaptive thresholding - Pass 2
// Perform a horizontal 7x1 or 5x1 average gray pixel value calculation and
// the final binarization

#define BLOCKSIZE 5
//#define USE_GAUSS_5

precision mediump float;

varying vec2 vTexCoord;

uniform vec2 uPxD;           // pixel delta values
uniform sampler2D sTexture;

// Fragment shader main routine
void main() {
    vec4 centerPx = texture2D(sTexture, vTexCoord); // stores: horizontal avg, orig. gray value, 0, 1
    
    const float bigC = (float(BLOCKSIZE + 4) + 0.5) / 255.0;
    
    // get the sum
#if BLOCKSIZE == 7
    float sum = texture2D(sTexture, vTexCoord + vec2(uPxD.y * -3.0, 0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y * -2.0, 0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y * -1.0, 0)).r +
                centerPx.r                                                +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y *  1.0, 0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y *  2.0, 0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y *  3.0, 0)).r;
    
    // get the average
    float avg = sum / 7.0;
#else
    
#ifdef USE_GAUSS_5
    float avg = 0.0625 * texture2D(sTexture, vTexCoord + vec2(uPxD.y * -2.0, 0)).r +
                0.25   * texture2D(sTexture, vTexCoord + vec2(uPxD.y * -1.0, 0)).r +
                0.375  * centerPx.r                                                +
                0.25   * texture2D(sTexture, vTexCoord + vec2(uPxD.y *  1.0, 0)).r +
                0.0625 * texture2D(sTexture, vTexCoord + vec2(uPxD.y *  2.0, 0)).r;
#else
    float sum = texture2D(sTexture, vTexCoord + vec2(uPxD.y * -2.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y * -1.0, 0.0)).r +
                centerPx.r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y *  1.0, 0.0)).r +
                texture2D(sTexture, vTexCoord + vec2(uPxD.y *  2.0, 0.0)).r;
    
    // get the average
    float avg = sum / 5.0;
#endif

#endif
    
    // create inverted binary value
//    float bin = 1.0 - step(max(0.0, avg - bigC), centerPx.g); // centerPx.g is orig. gray value at current position
    float bin = 1.0 - step(avg - bigC, centerPx.g); // centerPx.g is orig. gray value at current position
    
    // store thresholded values
    gl_FragColor = vec4(bin, bin, bin, 1.0);
}