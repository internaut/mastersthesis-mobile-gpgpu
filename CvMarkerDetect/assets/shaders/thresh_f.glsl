#define CONVERT_TO_GRAY

precision mediump float;

varying vec2 vTexCoord;

uniform float uThresh;
uniform sampler2D sTexture;

#ifdef CONVERT_TO_GRAY
const vec3 rgb2gray = vec3(0.299, 0.587, 0.114);
#endif

// Fragment shader main routine
void main() {
#ifdef CONVERT_TO_GRAY
    vec4 px = texture2D(sTexture, vTexCoord);
    float gray = dot(px.rgb, rgb2gray);
#else
    float gray = texture2D(sTexture, vTexCoord).r;
#endif
    
    float bin = step(uThresh, gray);
    
    gl_FragColor = vec4(bin, bin, bin, 1.0);
}