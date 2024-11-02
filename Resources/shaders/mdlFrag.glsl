#version 330 core

// Final color of pixel
out vec4 FragmentColor;

// Imports the current position we calculated
in vec3 crntPos;

// Imports the normal from the Vertex Shader
in vec3 Normal;

// Imports the texture coordinates from the Vertex Shader
in vec2 texCoord;

in vec4 fragPosLight;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirLight dirLight;


struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform int NR_POINT_LIGHTS;
#define MAX_LIGHTS 20
uniform PointLight pointLights[MAX_LIGHTS];
uniform samplerCube shadowCubeMap[MAX_LIGHTS];

// light uniforms
uniform vec4 lightColor;
uniform vec3 lightPos;

// Mesh uniform
uniform vec3 camPos;

// Textures
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D shadowMap;
//uniform samplerCube shadowCubeMap;

uniform float far_plane;

vec4 pointLight(PointLight light, samplerCube cubeMap)
{
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(light.position - crntPos);

	//float diffuseLight = 0.8f;
	float diffuseAmount = max(dot(normal, lightDirection), 0.0f);
	//float diffuse = diffuseAmount * diffuseLight;

	//float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specAmount = pow(max(dot(normal, halfwayDir), 0.0f), material.shininess);
	//float specular = specAmount * specularLight;

    // attenuation
    float distance    = length(light.position - crntPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                            light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient  * material.ambient * vec3(texture(diffuse0, texCoord));
    vec3 diffuse  = light.diffuse  * material.diffuse * diffuseAmount * vec3(texture(diffuse0, texCoord));
    vec3 specular = light.specular * material.specular * specAmount * vec3(texture(specular0, texCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    //vec3 texflat = ambient + diffuse + specular;

	// Shadow value
	float shadow = 0.0f;
	vec3 fragToLight = crntPos - light.position;
	float currentDepth = length(fragToLight);
	float bias = max(0.5f * (1.0f - dot(normal, lightDirection)), 0.0005f);

	// Not really a radius, more like half the width of a square
	int sampleRadius = 2;
	float offset = 0.02f;
	for(int z = -sampleRadius; z <= sampleRadius; z++)
	{
		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for(int x = -sampleRadius; x <= sampleRadius; x++)
		    {
		        float closestDepth = texture(cubeMap, fragToLight + vec3(x, y, z) * offset).r;
				// Remember that we divided by the farPlane?
				// Also notice how the currentDepth is not in the range [0, 1]
				closestDepth *= far_plane;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;
		    }
		}
	}
	// Average shadow
	shadow /= pow((sampleRadius * 2 + 1), 3);

	vec3 rawCol = ambient + (diffuse * (1.0f - shadow)) + (specular * (1.0f - shadow));
	//return vec4(rawCol,1.0);
	return vec4(rawCol,1.0);
}

vec4 direcLight()
{
	// ambient lighting
	float ambient = 1.0f;

	// diffuse lighting
	float diffuseLight = 0.50f;
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(vec3(-4.0f, 10.0f, -6.0f));
	float diffuseAmount = max(dot(normal, lightDirection), 0.0f);
	float diffuse = diffuseAmount * diffuseLight;

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	float specAmount = pow(max(dot(normal, halfwayDir), 0.0f), 16);
	float specular = specAmount * specularLight;

	// Shadow value
	float shadow = 0.0f;
	// Sets lightCoords to cull space
	vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
	if(lightCoords.z <= 1.0f)
	{
		// Get from [-1, 1] range to [0, 1] range just like the shadow map
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;
		// Prevents shadow acne
		float bias = max(0.025f * (1.0f - dot(normal, lightDirection)), 0.0005f);

		// Smoothens out the shadows
		int sampleRadius = 2;
		vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);
		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for(int x = -sampleRadius; x <= sampleRadius; x++)
		    {
		        float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;
		    }
		}
		// Get average shadow
		shadow /= pow((sampleRadius * 2 + 1), 2);

	}

	return (texture(diffuse0, texCoord) * (diffuse * (1.0f - shadow) + ambient) + texture(specular0, texCoord).r * specular * (1.0f - shadow)) * lightColor;
}


void main()
{
	//FragmentColor = pointLight() + direcLight();
	//FragmentColor = direcLight();

	vec4 tmp = vec4(0.0);
	tmp += direcLight();
	//for(int i = 0; i < 2; i++){
	  //  tmp += pointLight(pointLights[i], shadowCubeMap[i]);
	//}
	tmp += pointLight(pointLights[0], shadowCubeMap[0]);
	tmp += pointLight(pointLights[1], shadowCubeMap[1]);

    FragmentColor = tmp;
}
