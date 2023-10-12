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

struct HSInput
{
	float4 pos : POSITION;
};

HSInput main(VSInput i)
{
	HSInput o;
	//o.pos = mul(viewMatrix, float4(i.pos, 1.0f));
    o.pos = float4(i.pos, 1.0f);
	return o;
}