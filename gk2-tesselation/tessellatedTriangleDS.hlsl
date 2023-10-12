#define OUTPUT_PATCH_SIZE 16

cbuffer cbProj : register(b0) //Domain Shader constant buffer slot 0
{
	matrix projMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
    matrix viewMatrix;
    matrix invViewMatrix;
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
    float3 worldPos : POSITION0;
    float3 normal : NORMAL0;
    float3 viewVec : TEXCOORD0;
};

struct PositionNormal
{
    float4 pos;
    float3 normal;
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

float d_bezier1D(int i, float t)
{
    if (i == 0)
        return -2 * (1 - t) * (1 - t);
    if (i == 1)
        return 3 * (1 - t) * (1 - 3 * t);
    if (i == 2)
        return 3 * t * (2 - 3 * t);
    if (i == 3)
        return 3 * t * t;
    
    return 0.f;
}

PositionNormal bezier(const OutputPatch<DSControlPoint, 16> points, float2 uv)
{
    float4 B = 0.f;
    float4 dB_du = 0.f;
    float4 dB_dv = 0.f;
    for (int i = 0; i < 4; i++)
    {
        float4 bi = 0.f;
        float4 dbi_dv = 0.f;
        for (int j = 0; j < 4; j++)
        {
            bi += points[4 * i + j].pos * bezier1D(j, uv.y);
            dbi_dv += points[4 * i + j].pos * d_bezier1D(j, uv.y);
        }
        B += bi * bezier1D(i, uv.x);
        
        dB_du += bi * d_bezier1D(i, uv.x);
        dB_dv += dbi_dv * bezier1D(i, uv.x);
    }
    
    PositionNormal res;
    res.pos = B;
    res.normal = normalize(cross(dB_du, dB_dv));
    return res;
}

[domain("quad")]
PSInput main(HSPatchOutput factors, float2 uv : SV_DomainLocation,
	const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> input)
{
	PSInput o;
    
    PositionNormal bez = bezier(input, uv);
    o.worldPos = bez.pos;
    o.pos = mul(viewMatrix, bez.pos);
    o.pos = mul(projMatrix, o.pos);
    o.normal = normalize(float4(bez.normal, 0.f));
    
    float3 camPos = mul(invViewMatrix, float4(0.f, 0.f, 0.f, 1.f));
    o.viewVec = camPos - o.worldPos;

	return o;
}