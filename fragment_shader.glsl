// fragment_shader.glsl
#version 330 core

out vec4 FragColor;
in vec3 Normal;
in vec4 Color;

void main() {
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    float diff = max(dot(N, L), 0.0);
    FragColor = vec4(diff * color.rgb, 1.0);
}

