#ifndef GL_GPGPU_PIPELINE_RENDERER_IMGCONV_SHADER_GAUSS_H
#define GL_GPGPU_PIPELINE_RENDERER_IMGCONV_SHADER_GAUSS_H

#define IMG_CONV_FSHADER_GAUSS_3X3 "\
precision mediump float;\n\
uniform sampler2D sTexture;\n\
uniform vec2 uPxD;\n\
varying vec2 vTexCoord;\n\
void main() {\n\
	vec4 px00 = texture2D(sTexture, vTexCoord + vec2(-1.0*uPxD.x,-1.0*uPxD.y)) * 0.0625;\n\
	vec4 px10 = texture2D(sTexture, vTexCoord + vec2(0.0*uPxD.x,-1.0*uPxD.y)) * 0.125;\n\
	vec4 px20 = texture2D(sTexture, vTexCoord + vec2(1.0*uPxD.x,-1.0*uPxD.y)) * 0.0625;\n\
	vec4 px01 = texture2D(sTexture, vTexCoord + vec2(-1.0*uPxD.x,0.0*uPxD.y)) * 0.125;\n\
	vec4 px11 = texture2D(sTexture, vTexCoord + vec2(0.0*uPxD.x,0.0*uPxD.y)) * 0.25;\n\
	vec4 px21 = texture2D(sTexture, vTexCoord + vec2(1.0*uPxD.x,0.0*uPxD.y)) * 0.125;\n\
	vec4 px02 = texture2D(sTexture, vTexCoord + vec2(-1.0*uPxD.x,1.0*uPxD.y)) * 0.0625;\n\
	vec4 px12 = texture2D(sTexture, vTexCoord + vec2(0.0*uPxD.x,1.0*uPxD.y)) * 0.125;\n\
	vec4 px22 = texture2D(sTexture, vTexCoord + vec2(1.0*uPxD.x,1.0*uPxD.y)) * 0.0625;\n\
	vec4 clr = px00+px10+px20+px01+px11+px21+px02+px12+px22;\n\
	gl_FragColor = clr;\n\
}\
"

#define IMG_CONV_FSHADER_GAUSS_3X1 "\
precision mediump float;\n\
uniform sampler2D sTexture;\n\
uniform float uPxD;\n\
varying vec2 vTexCoord;\n\
void main() {\n\
	vec4 pxC = texture2D(sTexture, vTexCoord);\n\
	vec4 pxL = texture2D(sTexture, vTexCoord - vec2(uPxD, 0.0));\n\
	vec4 pxR = texture2D(sTexture, vTexCoord + vec2(uPxD, 0.0));\n\
	gl_FragColor = 0.25 * (pxL + pxR) + 0.5 * pxC;\n\
}\
"

#define IMG_CONV_FSHADER_GAUSS_5X1 "\
precision mediump float;\n\
uniform sampler2D sTexture;\n\
uniform float uPxD;\n\
varying vec2 vTexCoord;\n\
void main() {\n\
	vec4 pxC = texture2D(sTexture, vTexCoord);\n\
	vec4 pxL1 = texture2D(sTexture, vTexCoord - vec2(uPxD, 0.0));\n\
	vec4 pxL2 = texture2D(sTexture, vTexCoord - vec2(2.0 * uPxD, 0.0));\n\
	vec4 pxR1 = texture2D(sTexture, vTexCoord + vec2(uPxD, 0.0));\n\
	vec4 pxR2 = texture2D(sTexture, vTexCoord + vec2(2.0 * uPxD, 0.0));\n\
	gl_FragColor = 0.0625 * (pxL2 + pxR2)\n\
				 + 0.2500 * (pxL1 + pxR1)\n\
				 + 0.3750 * pxC;\n\
}\
"

#define IMG_CONV_FSHADER_GAUSS_7X1 "\
precision mediump float;\n\
uniform sampler2D sTexture;\n\
uniform float uPxD;\n\
varying vec2 vTexCoord;\n\
void main() {\n\
	vec4 pxC = texture2D(sTexture, vTexCoord);\n\
	vec4 pxL1 = texture2D(sTexture, vTexCoord - vec2(uPxD, 0.0));\n\
	vec4 pxL2 = texture2D(sTexture, vTexCoord - vec2(2.0 * uPxD, 0.0));\n\
	vec4 pxL3 = texture2D(sTexture, vTexCoord - vec2(3.0 * uPxD, 0.0));\n\
	vec4 pxR1 = texture2D(sTexture, vTexCoord + vec2(uPxD, 0.0));\n\
	vec4 pxR2 = texture2D(sTexture, vTexCoord + vec2(2.0 * uPxD, 0.0));\n\
	vec4 pxR3 = texture2D(sTexture, vTexCoord + vec2(3.0 * uPxD, 0.0));\n\
	gl_FragColor = 0.006 * (pxL3 + pxR3)\n\
				 + 0.061 * (pxL2 + pxR2)\n\
				 + 0.242 * (pxL1 + pxR1)\n\
				 + 0.382 * pxC;\n\
}\
"

#endif
