precision mediump float;

uniform mat4 uMVPMat;
uniform mat4 uTransformMat; // for scaling

attribute vec4 aPos;

void main() {
//    mat4 scaleMat = mat4(1.0);
//    scaleMat[0][0] = uScale;
//    scaleMat[1][1] = uScale;
//    scaleMat[2][2] = uScale;
    
//    gl_Position = uMVPMat * scaleMat * aPos;
    
    gl_Position = uMVPMat * uTransformMat * aPos;
//    gl_Position = aPos * uMVPMat;
    
//    gl_Position =  aPos;
}