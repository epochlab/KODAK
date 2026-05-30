#version 330 core

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vUV;

uniform sampler2D uAlbedo;
uniform sampler2D uSkyHDR;  // equirectangular HDRI — sampled with surface normal for diffuse
uniform int       uViewMode;  // 1=diffuse 2=wireframe 3=depth 4=position 5=normals 6=uv 7=irradiance
uniform float     uNear;
uniform float     uFar;

out vec4 FragColor;

const float PI = 3.14159265358979;

// Direct radiance sample along direction n. M7 replaces this with a preconvolved irradiance map.
vec3 sampleEnv(vec3 n) {
    float phi   = atan(n.z, n.x);
    float theta = acos(clamp(n.y, -1.0, 1.0));
    return texture(uSkyHDR, vec2(phi / (2.0 * PI) + 0.5, theta / PI)).rgb;
}

void main() {
    if (uViewMode == 2) {
        FragColor = vec4(0.35, 0.85, 1.0, 1.0);

    } else if (uViewMode == 3) {
        float z  = gl_FragCoord.z * 2.0 - 1.0;
        float ld = (2.0 * uNear * uFar) / (uFar + uNear - z * (uFar - uNear));
        float d  = (ld - uNear) / (uFar - uNear);
        FragColor = vec4(d, d, d, 1.0);

    } else if (uViewMode == 4) {
        FragColor = vec4(clamp(vFragPos * 0.1 + 0.5, 0.0, 1.0), 1.0);

    } else if (uViewMode == 5) {
        FragColor = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);

    } else if (uViewMode == 6) {
        FragColor = vec4(fract(vUV), 0.0, 1.0);

    } else if (uViewMode == 7) {
        // Irradiance only — HDRI diffuse, no albedo texture
        FragColor = vec4(sampleEnv(normalize(vNormal)), 1.0);

    } else {
        // Mode 1 — albedo × HDRI diffuse irradiance
        vec3 albedo     = texture(uAlbedo, vUV).rgb;
        vec3 irradiance = sampleEnv(normalize(vNormal));
        FragColor       = vec4(albedo * irradiance, 1.0);
    }
}
