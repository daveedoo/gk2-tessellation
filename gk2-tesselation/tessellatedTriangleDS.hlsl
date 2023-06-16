#define OUTPUT_PATCH_SIZE 16

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

float bezier1D(int i, float t)
{
    if (i == 0)
        return pow(1 - t, 3);
    if (i == 1)
        return 3.f * t * (1 - t) * (1 - t);
    if (i == 2)
        return 3.f * t * t * (1 - t);
    if (i == 3)
        return t * t * t;
    
    return 0.f;
}

float4 bezier(const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> points, float2 uv)
{
    float4 pos = 0.f;
    for (int i = 0; i < 4; i++)
    {
        float4 bi = 0.f;
        for (int j = 0; j < 4; j++)
        {
            bi += points[4 * i + j].pos * bezier1D(j, uv.y);
        }
        pos += bi * bezier1D(i, uv.x);
    }

    return pos;
}

[domain("quad")]
PSInput main(HSPatchOutput factors, float2 uv : SV_DomainLocation,
	const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> input)
{
	PSInput o;
    //float4 bottom = lerp(input[0].pos, input[12].pos, uv.x);
    //float4 top = lerp(input[3].pos, input[15].pos, uv.x);
    //float4 viewPos = lerp(bottom, top, uv.y);
    //o.pos = mul(projMatrix, viewPos);
    
    o.pos = mul(projMatrix, bezier(input, uv));

	return o;
}