layout(isolines) in;

layout (std140, binding = HAIR_DATA_BINDING) uniform HairDataBlock {
    HairRenderData hairData;
};

layout(std430, binding = POSITIONS_BUFFER_BINDING) buffer Positions {
    vec4 data[];
} positions;

layout(std430, binding = HAIR_INDICES_BUFFER_BINDING) buffer HairIndices {
    ivec4 data[];
} hairIndices;

patch in int triangleIndex;
patch in int segmentIndex;

layout(location = 0) out vec3 out_pos;
layout(location = 1) out vec3 out_tangent;
layout(location = 2) out float out_width;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 getBarycentricCoords()
{
    float t = rand(vec2(gl_TessCoord.y, 0.3));
	float v = rand(vec2(gl_TessCoord.y, -0.7));
	if(t + v > 1.0)
    {
        t = 1.0 - t;
        v = 1.0 - v;
    }
	return vec3(t, v, 1.0 - t - v);
}

float getHairCoords()
{
    return (segmentIndex + gl_TessCoord.x) / hairData.segmentsCount;
}

vec3 getVertexPos(int hairIndex, int vertexIndex)
{
    int index = hairIndex * (hairData.segmentsCount + 1) + clamp(vertexIndex, 0, hairData.segmentsCount);
	return positions.data[index].xyz;
}

vec3 getInterpolatedPosition(ivec3 hairIndices, int vertexIndex, vec3 bSplineWeights)
{
    vec3 position = vec3(0, 0, 0);
	position += getVertexPos(hairIndices[0], vertexIndex) * bSplineWeights[0];
	position += getVertexPos(hairIndices[1], vertexIndex) * bSplineWeights[1];
	position += getVertexPos(hairIndices[2], vertexIndex) * bSplineWeights[2];
	return position;
}

ivec3 getHairIndex(int triangleIndex)
{
	return hairIndices.data[triangleIndex].xyz;
}

void main()
{
	ivec3 hairIndex = getHairIndex(triangleIndex);
	vec3 bSplineWeights = getBarycentricCoords();

    vec3 p0 = getInterpolatedPosition(hairIndex, segmentIndex - 1, bSplineWeights);
	vec3 p1 = getInterpolatedPosition(hairIndex, segmentIndex, bSplineWeights);
	vec3 p2 = getInterpolatedPosition(hairIndex, segmentIndex + 1, bSplineWeights);
	vec3 p3 = getInterpolatedPosition(hairIndex, segmentIndex + 2, bSplineWeights);

	float t = gl_TessCoord.x;
	float t2 = t * t;
	float t3 = t2 * t;
	vec4 tVector = vec4(t3, t2, t, 1) / 6.0;
	mat4 coefficientMatrix;
	coefficientMatrix[0] = vec4(-1, 3, -3, 1);
	coefficientMatrix[1] = vec4(3, -6, 0, 4);
	coefficientMatrix[2] = vec4(-3, 3, 3, 1);
	coefficientMatrix[3] = vec4(1, 0, 0, 0);
	vec4 bSpline = tVector * coefficientMatrix;

	out_pos = p0 * bSpline[0] + p1 * bSpline[1] + p2 * bSpline[2] + p3 * bSpline[3];

	float thinning = getHairCoords() - hairData.thinningStart;
	thinning = clamp(t, 0.0, 1.0);
	out_width = mix(hairData.rootWidth, hairData.tipWidth, thinning);

	vec3 tangentBottom = normalize(p2 - p1);
	vec3 tangentTop = p3 - p2;
	if(length(tangentTop) == 0)
	{
	    out_tangent = tangentBottom;
	}
	else 
	{
	    tangentTop = normalize(tangentTop);
	    out_tangent = mix(tangentBottom, tangentTop, t);
	}
}