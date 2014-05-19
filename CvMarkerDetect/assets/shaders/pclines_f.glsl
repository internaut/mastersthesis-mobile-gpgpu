precision mediump float;

varying float vFragAddVal;

void main() {
    //gl_FragColor = vec4(vFragAddVal, vFragAddVal, vFragAddVal, 1.0);
//    gl_FragColor = vec4(1.0);
    gl_FragColor = vec4(vFragAddVal);
}