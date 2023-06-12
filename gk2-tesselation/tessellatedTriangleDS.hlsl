#define OUTPUT_PATCH_SIZE 3

cbuffer cbProj : register(b0) //Domain Shader constant buffer slot 0
{
	matrix projMatrix;
};

struct HSPatchOutput
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct DSControlPoint
{
	float4 pos : POSITION;
};

struct PSInput
{
	float4 pos : SV_POSITION;
};

[domain("tri")]
PSInput main(HSPatchOutput factors, float3 uvw : SV_DomainLocation,
	const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> input)
{
	PSInput o;
	float4 viewPos = input[0].pos * uvw.x + input[1].pos * uvw.y + input[2].pos * uvw.z;
	o.pos = mul(projMatrix, viewPos);
	return o;
}