/* pmd/edge.vsh */
invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
attribute vec3 inPosition;
varying vec4 outColor;
const float kOne = 1.0;

void main() {
    outColor = color;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}
