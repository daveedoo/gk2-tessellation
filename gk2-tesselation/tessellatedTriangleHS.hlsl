#define INPUT_PATCH_SIZE 3
#define OUTPUT_PATCH_SIZE 3

struct HSInput
{
	float4 pos : POSITION;
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

HSPatchOutput HS_Patch(InputPatch<HSInput, INPUT_PATCH_SIZE> ip, uint patchId : SV_PrimitiveID)
{
	HSPatchOutput o;
	o.edges[0] = o.edges[1] = o.edges[2] = 8.0f;
	o.inside = 8.0f;
	return o;
}

[domain("tri")]
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