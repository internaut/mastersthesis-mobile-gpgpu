precision mediump float;

varying vec2 vTexCoord;

uniform sampler2D sTexture;

const vec3 rgb2gray = vec3(0.299, 0.587, 0.114);

void main() {
    float gray = dot(texture2D(sTexture, vTexCoord).rgb, rgb2gray);
    
    gl_FragColor = vec4(gray, gray, gray, 1.0);
}