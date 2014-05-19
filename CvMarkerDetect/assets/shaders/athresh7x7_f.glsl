precision mediump float;

varying vec2 vTexCoord;

uniform vec2 uPxD;           // pixel delta values
uniform sampler2D sTexture;

// values from OpenCV cvtColor
const vec3 rgb2gray = vec3(0.299, 0.587, 0.114);

// define block to calculate average
const float blockSize = 7.0;
const float blockMax = (blockSize - 1.0) / 2.0;

void main() {
    vec4 px0 = texture2D(sTexture, vTexCoord);  // center pixel
    float gray0 = dot(px0.rgb, rgb2gray);
    
    float avg = 0.0;
//    const float avgDiv = 7.0 * 7.0;
    for (float i = -blockMax; i <= blockMax; i++) {
        for (float j = -blockMax; j <= blockMax; j++) {
            vec4 px = texture2D(sTexture, vTexCoord + vec2(uPxD.x * j, uPxD.y * i));
//            float gray = dot(px.rgb, rgb2gray);
//            avg += gray / avgDiv;
            avg += dot(px.rgb, rgb2gray);
        }
    }
    
    avg /= (blockSize * blockSize);
    
    // create binary value
    float bin = 1.0 - step(avg - 0.03, gray0);
    
    gl_FragColor = vec4(bin, bin, bin, 1.0);
}