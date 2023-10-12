#define INPUT_PATCH_SIZE 16
#define OUTPUT_PATCH_SIZE 16

cbuffer cbTesselationFactors : register(b0) // Hull shader constant buffer slot 0
{
    float outside;
    float inside;
};

struct HSInput
{
	float4 pos : POSITION;
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

HSPatchOutput HS_Patch(InputPatch<HSInput, INPUT_PATCH_SIZE> ip, uint patchId : SV_PrimitiveID)
{
	HSPatchOutput o;
    o.edges[0] = o.edges[1] = o.edges[2] = o.edges[3] = outside;
    o.inside[0] = o.inside[1] = inside;
	return o;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(OUTPUT_PATCH_SIZE)]
[patchconstantfunc("HS_Patch")]
DSControlPoint main(InputPatch<HSInput, INPUT_PATCH_SIZE> ip, uint i : SV_OutputControlPointID,
	uint patchID : SV_PrimitiveID)
{
	DSControlPoint o;
    o.pos = ip[i].pos;
	return o;
}