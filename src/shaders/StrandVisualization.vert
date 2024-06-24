#define POSITIONS_BUFFER_BINDING 3

layout(std430, binding = POSITIONS_BUFFER_BINDING) buffer Positions
{
    vec4 data[];
} positions;

uniform mat4 viewProjectionMatrix;
uniform int doubleSegments;
uniform int verticesPerStrand;

void main()
{
    int strandIndex = gl_VertexID / doubleSegments;
	int lineIndex = gl_VertexID % doubleSegments;
	int vertIndex = lineIndex / 2 + lineIndex % 2;

	vec4 position = positions.data[strandIndex * verticesPerStrand + vertIndex];

	gl_Position = viewProjectionMatrix * vec4(position.xyz, 1.0);
}