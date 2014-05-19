precision mediump float;

varying vec2 vTexCoord;

uniform sampler2D sTexture;

void main() {
    gl_FragColor = texture2D(sTexture, vTexCoord);
//    gl_FragColor = vec4(1.0);
}