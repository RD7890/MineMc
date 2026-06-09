#version 300 es
precision mediump float;

in vec2 vUV;
in float vLight;

uniform sampler2D uTex;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(uTex, vUV);
    // Discard fully transparent pixels (for future leaves/glass support)
    if (texColor.a < 0.1) discard;
    fragColor = vec4(texColor.rgb * vLight, texColor.a);
}
