#version 120
varying vec3 vNormal;
varying vec3 vColor;

void main() {
    float light = max(dot(vNormal, vec3(0.0, 1.0, 0.0)), 0.1);
    gl_FragColor = vec4(vColor * light, 1.0);
}
