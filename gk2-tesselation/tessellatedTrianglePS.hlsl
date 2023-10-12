cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0
{
    float4 surfaceColor;
};

cbuffer cbViewProj : register(b1) //Pixel Shader constant buffer slot 1
{
    matrix viewProj;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 normal : NORMAL0;
    float3 viewVec : TEXCOORD0;
};

static const float3 lightPos = float3(0.f, 0.f, 1.f);
static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float kd = 0.5, ks = 0.2f, m = 100.0f;

float4 main(PSInput i) : SV_TARGET
{
    float3 V = normalize(i.viewVec);
    float3 L = normalize(lightPos - i.worldPos);
    float3 H = normalize(V + L);
    float3 N = i.normal;
    
    float3 color = surfaceColor.rgb * ambientColor; //ambient reflection
    color += lightColor * surfaceColor.xyz * kd * saturate(dot(N, L)); //diffuse reflection
    color += lightColor * ks * pow(saturate(dot(N, H)), m); //specular reflection
    return float4(saturate(color), surfaceColor.a);
    
    //return float4(i.normal, 1.f);
    //return surfaceColor;
}