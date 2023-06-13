#define OUTPUT_PATCH_SIZE 4

cbuffer cbProj : register(b0) //Domain Shader constant buffer slot 0
{
	matrix projMatrix;
};

struct HSPatchOutput
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct DSControlPoint
{
	float4 pos : POSITION;
};

struct PSInput
{
	float4 pos : SV_POSITION;
};

[domain("quad")]
PSInput main(HSPatchOutput factors, float2 uv : SV_DomainLocation,
	const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> input)
{
	PSInput o;
    float4 bottom = lerp(input[0].pos, input[1].pos, uv.x);
    float4 top = lerp(input[3].pos, input[2].pos, uv.x);
    float4 viewPos = lerp(bottom, top, uv.y);
	o.pos = mul(projMatrix, viewPos);
	return o;
}