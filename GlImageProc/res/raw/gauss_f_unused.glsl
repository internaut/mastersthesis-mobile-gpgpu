precision mediump float;

#define KSUM    16.0
#define K1      (1.0 / KSUM)
#define K2      (2.0 / KSUM)
#define K3      (4.0 / KSUM)

varying vec2 vTexCoord;

uniform vec2 pxD;           // pixel delta values
uniform sampler2D sTexture;

void main() {
    vec4 p00 = texture2D(sTexture, vTexCoord + vec2(-pxD.x, -pxD.y)) * K1;
    vec4 p10 = texture2D(sTexture, vTexCoord + vec2(   0.0, -pxD.y)) * K2;
    vec4 p20 = texture2D(sTexture, vTexCoord + vec2( pxD.x, -pxD.y)) * K1;
    vec4 p01 = texture2D(sTexture, vTexCoord + vec2(-pxD.x,    0.0)) * K2;
    vec4 p11 = texture2D(sTexture, vTexCoord + vec2(   0.0,    0.0)) * K3;
    vec4 p21 = texture2D(sTexture, vTexCoord + vec2( pxD.x,    0.0)) * K2;
    vec4 p02 = texture2D(sTexture, vTexCoord + vec2(-pxD.x,  pxD.y)) * K1;
    vec4 p12 = texture2D(sTexture, vTexCoord + vec2(   0.0,  pxD.y)) * K2;
    vec4 p22 = texture2D(sTexture, vTexCoord + vec2( pxD.x,  pxD.y)) * K1;
    
    vec4 clr =    p00 + p01 + p02
                + p10 + p11 + p12
                + p20 + p21 + p22;
    
    gl_FragColor = clr;
}