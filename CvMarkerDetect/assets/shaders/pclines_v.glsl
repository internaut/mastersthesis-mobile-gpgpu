// OGLES 2.0 has no gl_VertexId, so we must pass the coordinates

attribute vec2 aTexCoord;
attribute float aTSCoordUsageIdx;   // attribs can only be floats, vecs or mats

varying float vFragAddVal;

uniform sampler2D sTexture;


// Define the coordinate factors and summands for
// line beginnings and endings in T and S space, respectively.
// mat2x4 is not available in OGLES 2.0 :(

const mat4 tsCoordFactors = mat4(
     0, -1, 0, 0,  // t begin
     0,  1, 0, 0,  // t end
     0,  1, 0, 0,  // s begin
     0,  1, 0, 0   // s end
);

const mat4 tsCoordAdd = mat4(
    -1,  0, 0, 0,  // t begin
     0,  0, 0, 0,  // t end
     0,  0, 0, 0,  // s begin
     1,  0, 0, 0   // s end
);

const vec3 rgb2gray = vec3(0.299, 0.587, 0.114);

// NOTE: Input texture must be binary image with format RGBA (R=G=B=Bin value)
// "aTexCoord" delivers a texture coordinate for each single pixel in the texture
void main() {
    // get the binary value at aTexCoord
//    float pxVal = texture2DLod(sTexture, aTexCoord, 0.0).r;
//    float bin = step(0.5, pxVal);
    
    float bin = texture2DLod(sTexture, aTexCoord, 0.0).r;
/*    vec4 px = texture2D(sTexture, aTexCoord);
    float gray = dot(px.rgb, rgb2gray);
    float bin = step(0.5, gray);*/
    
    // get the provided index that selects a coordinate factor and summand
    // depending on: start or end point? T or S space?
    int idx = int(aTSCoordUsageIdx);
    vec2 coordFact = tsCoordFactors[idx].xy;
    vec2 coordAdd = tsCoordAdd[idx].xy;
    
    // normalize the coordinates and create a flipped version
    vec2 normCoordStd = -1.0 * aTexCoord + 2.0 * aTexCoord;
    vec2 normCoordFlipped = normCoordStd.yx;
    
    // the following is to select the standard or the flipped version
    // of the normalized coordinates, depening on the provided TS coord select index
//    float normCoordSelect = float(abs(mod(float(idx - 3), 3.0)));  // indices 1 and 2 yield normCoordSelect > 0 so we can perform a step() to select one of the normCoord versions
    
    vec2 finalCoord = vec2(0, 0);
    if (idx == 0 || idx == 3) {
        finalCoord = normCoordStd;
    } else {
        finalCoord = normCoordFlipped;
    }
    
//    float useNormCoordFlipped   = step(1.0, normCoordSelect);
//    float useNormCoordStd       = 1.0 - useNormCoordFlipped;
    
//    vec2 finalCoord = useNormCoordStd * normCoordStd
//                    + useNormCoordFlipped * normCoordFlipped;
    
    // calculate one line point in ST space
    vec2 linePoint = bin * (coordFact * finalCoord + coordAdd);

    // set the value that will be added in the fragment shader
    vFragAddVal = bin * (1.0 / 256.0);
    
    // set the position of the line point
    gl_Position = vec4(linePoint, 0.0, 1.0);
}