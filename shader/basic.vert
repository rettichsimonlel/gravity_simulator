#version 120
varying vec3 vNormal;
varying vec3 vColor;

uniform vec3 objectColor;

void main() {
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    vColor = objectColor;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
