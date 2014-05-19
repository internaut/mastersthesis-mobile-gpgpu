attribute vec2 aTexCoord;
uniform vec2 uAreaDim;  // contains number of area rows and columns

uniform sampler2D sTexture;

#define NORM_COORD(c) (-1.0 + 2.0 * (c))

void main() {
    float gray = texture2DLod(sTexture, aTexCoord, 1.0).r;  // gray input image
    
    float areaX = aTexCoord.x * uAreaDim.x;
    float areaY = aTexCoord.y * uAreaDim.y;
    float areaNum = areaY * uAreaDim.x + areaX;
    
//    gl_Position = vec4(NORM_COORD(gray), NORM_COORD(areaNum), 0.0, 1.0);
    gl_Position = vec4(gray, 0.0, 0.0, 1.0);
//    gl_Position = vec4(0.5 * gray, 0.0, 0.0, 1.0);
    gl_PointSize = 1.0;
}