#version 450
// Vertex attributes
layout(location = 0) in vec3 vPos; // Vertex position in model space
layout(location = 1) in vec3 vNormal; // Vertex normal in model space
layout(location = 2) in vec2 vTexCoord; // Vertex texture coordinate (UV)
layout(location = 3) in vec3 vTangents; // Tangents of the mesh

uniform mat4 _Model; // Model->World Matrix
uniform mat4 _ViewProjection; // Combined View->Projection Matrix

out Surface {
    vec3 WorldPos; // Vertex position in world space
    vec3 WorldNormal; // Vertex normal in world space
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

void main() {
    // Transform vertex position to World Space
    vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));

    // Transform vertex normal to world space using Normal Matrix
    mat3 normalMatrix = transpose(inverse(mat3(_Model)));
    vs_out.WorldNormal = normalize(normalMatrix * vNormal);

    // Transform tangent to world space and ensure orthogonality
    vec3 tangent = normalize(normalMatrix * vTangents);
    vec3 bitangent = normalize(cross(vs_out.WorldNormal, tangent));

    // Construct TBN matrix for transforming normals from tangent to world space
    vs_out.TBN = mat3(tangent, bitangent, vs_out.WorldNormal);

    // Pass texture coordinates to fragment shader
    vs_out.TexCoord = vTexCoord;

    // Transform vertex position to clip space
    gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0);
}