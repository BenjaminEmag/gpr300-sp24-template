#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D screenTexture;

uniform bool blur;
uniform bool chromaticAberration;
uniform float aberrationStrength;

uniform bool vignette;
uniform bool scanline;

// https://en.wikipedia.org/wiki/Kernel_(image_processing)
#define gaussian_blur mat3(1, 2, 1, 2, 4, 2, 1, 2, 1) * 0.0625

float LinearizeDepth(float depth) {
    float near = 0.1; // Near plane
    float far = 100.0; // Far plane
	float z = depth * 2.0 - 1.0; 

	z = (2.0 * near * far) / (far + near - z * (far - near));	
	return z / far;

}

void main() {
	vec3 totalColors = texture(screenTexture, UV).rgb;

    if(blur) {
        vec2 texelSize = 1.0 / textureSize(screenTexture, 0).xy;
        vec3 result = vec3(0.0);
        
        mat3 kernel = gaussian_blur;
        int index = 0;
        
        for(int i = -1; i <= 1; i++) {
            for(int j = -1; j <= 1; j++) {
                vec2 offset = vec2(float(i), float(j)) * texelSize;
                result += texture(screenTexture, UV + offset).rgb * kernel[i + 1][j + 1];
            }
        }
        totalColors = result;
    }

    if (chromaticAberration) 
	{
        float linearDepth = 1 - LinearizeDepth(gl_FragCoord.z);
        vec2 texelSize = 1.0 / textureSize(screenTexture, 0).xy;
        vec3 shiftedColor;
        shiftedColor.r = texture(screenTexture, UV + aberrationStrength * linearDepth * texelSize).r;
        shiftedColor.g = texture(screenTexture, UV).g;
        shiftedColor.b = texture(screenTexture, UV - aberrationStrength * linearDepth * texelSize).b;
        totalColors = shiftedColor;
    }

	if (scanline)
	{
		float scanline = sin(UV.y * textureSize(screenTexture, 0).y * 3.14159) * 0.1 + 0.9;
		totalColors *= scanline;
	}

	if (vignette)
	{
		float vignette = smoothstep(0.9, 0.1, length(UV - 0.5) * 1.1);
		totalColors *= vignette;
	}

	FragColor = vec4(totalColors, 1.0);
}