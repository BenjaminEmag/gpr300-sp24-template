#version 450
out vec4 FragColor; // The color of this fragment
in Surface {
    vec3 WorldPos; // Vertex position in world space
    vec3 WorldNormal; // Vertex normal in world space
    vec2 TexCoord;
    mat3 TBN;
} fs_in;

uniform sampler2D _MainTex; 
uniform sampler2D _NormalMap; 
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0); // Light direction in world space
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

struct Material {
    float Ka; // Ambient coefficient (0-1)
    float Kd; // Diffuse coefficient (0-1)
    float Ks; // Specular coefficient (0-1)
    float Shininess; // Affects size of specular highlight
};
uniform Material _Material;

void main() {
    // Sample normal map and transform from tangent space to world space
    vec3 normal = texture(_NormalMap, fs_in.TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0); // Transform from [0, 1] to [-1, 1]
    normal = normalize(fs_in.TBN * normal); // Transform to world space

    // Light direction is already in world space
    vec3 toLight = normalize(-_LightDirection);

    // Calculate diffuse factor
    float diffuseFactor = max(dot(normal, toLight), 0.0);

    // Calculate view direction in world space
    vec3 toEye = normalize(_EyePos - fs_in.WorldPos);

    // Blinn-Phong uses half angle
    vec3 h = normalize(toLight + toEye);
    float specularFactor = pow(max(dot(normal, h), 0.0), _Material.Shininess);

    // Combine specular and diffuse reflection
    vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
    lightColor += _AmbientColor * _Material.Ka;

    // Sample texture and calculate final color
    vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
    FragColor = vec4(objectColor * lightColor, 1.0);
}