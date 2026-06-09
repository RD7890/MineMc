#version 300 es

// Per-vertex attributes
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uMVP;

out vec2 vUV;
out float vLight;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    vUV = aUV;

    // Simple directional lighting (no fancy light engine — just AO via face normal)
    vec3 lightDir = normalize(vec3(0.6, 1.0, 0.4));
    float diffuse = max(dot(normalize(aNormal), lightDir), 0.0);
    vLight = 0.45 + 0.55 * diffuse; // ambient + diffuse
}
