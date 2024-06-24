layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

layout (std140, binding = SCENE_DATA_BINDING) uniform SceneDataBlock {
    SceneRenderData sceneData;
};

layout(location = 0) in vec3 in_pos[];
layout(location = 1) in vec3 in_tangent[];
layout(location = 2) in float in_width[];

layout(location = 0) out vec3 out_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;

void calculateVertex(vec3 position, vec3 offset)
{
    vec3 offsetPos = position + offset;
	gl_Position = sceneData.viewProjectionMatrix * vec4(offsetPos, 1.0);

	out_normal = normalize(offset);
	out_uv = vec2(0.0, 0.0);
	out_pos = offsetPos;

	EmitVertex();
}

void main() {
    vec3 eyeVec0 = normalize(sceneData.eyePosition - in_pos[0]);
	vec3 eyeVec1 = normalize(sceneData.eyePosition - in_pos[1]);
	vec3 sideVec0 = normalize(cross(eyeVec0, in_tangent[0])) * in_width[0] / 2.0;
	vec3 sideVec1 = normalize(cross(eyeVec1, in_tangent[1])) * in_width[1] / 2.0;

	calculateVertex(in_pos[0], sideVec0);
	calculateVertex(in_pos[1], sideVec1);
	calculateVertex(in_pos[0], -sideVec0);
	calculateVertex(in_pos[1], -sideVec1);
}