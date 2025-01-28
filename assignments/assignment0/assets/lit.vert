#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Vertex position in model space
layout(location = 1) in vec3 vNormal; //Vertex position in model space
layout(location = 2) in vec2 vTexCoord; //Vertex texture coordinate (UV)
layout(location = 3) in vec3 vTangents; //Tangents of the mesh

uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix

out Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
	mat3 TBN;
}vs_out;

void main(){
	//Transform vertex position to World Space.
	vs_out.WorldPos = vec3(_Model * vec4(vPos,1.0));

	//Transform vertex normal to world space using Normal Matrix
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;

    vec3 tangent = normalize(transpose(inverse(mat3(_Model))) * vTangents);
    vec3 bitangent = normalize(cross(vs_out.WorldNormal, tangent));
    vs_out.TBN = mat3(tangent, bitangent, vs_out.WorldNormal);

	vs_out.TexCoord = vTexCoord;
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
