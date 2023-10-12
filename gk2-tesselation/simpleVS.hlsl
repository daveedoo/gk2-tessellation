cbuffer cbView : register(b0) //Vertex Shader constant buffer slot 0
{
    matrix viewMatrix;
    matrix invViewMatrix;
};

cbuffer cbProj : register(b1) //Vertex Shader constant buffer slot 1
{
    matrix projMatrix;
};

struct VSInput
{
    float3 pos : POSITION;
};

struct PSInput
{
    float4 pos : SV_POSITION;
};

PSInput main(VSInput i)
{
    PSInput o;
    
    float4 v = mul(viewMatrix, float4(i.pos, 1.0f));
    o.pos = mul(projMatrix, v);
    return o;
}