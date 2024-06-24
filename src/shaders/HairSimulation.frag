layout (std140, binding = HAIR_DATA_BINDING) uniform HairDataBlock {
    HairRenderData hairData;
};

layout (std140, binding = LIGHT_DATA_BINDING) uniform LightDataBlock {
    LightRenderData lightData;
};

layout (std140, binding = SCENE_DATA_BINDING) uniform SceneDataBlock {
    SceneRenderData sceneData;
};

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out vec4 out_color;

void main() {

	Light light = lightData.lights[0];
	vec3 ambient = hairData.ambient * hairData.color.xyz;
	vec3 lightVec = normalize(light.position - in_pos);
	float diff = max(dot(in_normal, lightVec), 0.0);
	vec3 diffuse = hairData.diffuse * diff * hairData.color.xyz;
	vec3 eyeVec = normalize(in_pos - sceneData.eyePosition);
	vec3 reflectedVec = reflect(lightVec, in_normal);
	float spec = pow(max(dot(eyeVec, reflectedVec), 0.0), hairData.specularPower);
	vec3 specular = hairData.specular * spec * hairData.color.xyz;
	vec3 result = (ambient + diffuse + specular) * light.color.xyz;
	out_color = vec4(result.x, result.y, result.z, hairData.color.w);
}