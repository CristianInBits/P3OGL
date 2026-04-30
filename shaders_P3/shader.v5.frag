#version 330 core

out vec4 outColor;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;

uniform sampler2D colorTex;
uniform sampler2D emiTex;

// Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 N;
float alpha = 50.0;
vec3 Ke;

// Optativa 1: Múltiples fuentes de luz
#define MAX_LIGHTS 4
uniform int numLights;
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightId[MAX_LIGHTS];
uniform vec3 lightIs[MAX_LIGHTS];
uniform vec3 Ia;

vec3 shade();

void main()
{
	Ka = texture(colorTex, texCoord).rgb;
	Kd = texture(colorTex, texCoord).rgb;
	Ke = texture(emiTex, texCoord).rgb;
	Ks = vec3(1.0);

	N = normalize(norm);

	outColor = vec4(shade(), 1.0);
}

vec3 shade()
{
	// Ambiental: fuera del bucle
	vec3 c = Ia * Ka;

	// Sumatorio sobre todas las luces
	for (int i = 0; i < numLights; i++)
	{
		// Difusa
		vec3 L = normalize(lightPos[i] - pos);
		vec3 diffuse = lightId[i] * Kd * max(dot(L, N), 0.0);
		c += clamp(diffuse, 0.0, 1.0);

		// Especular
		vec3 V = normalize(-pos);
		vec3 R = normalize(reflect(-L, N));
		float factor = max(dot(R, V), 0.01);
		vec3 specular = lightIs[i] * Ks * pow(factor, alpha);
		c += clamp(specular, 0.0, 1.0);
	}

	// Emisiva: fuera del bucle
	c += Ke;

	return c;
}
