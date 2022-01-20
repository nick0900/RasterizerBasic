struct VertexShaderInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float3 normal : normal;
	float2 uv : uv;
    float3 lightVector : lightvector;
    float lightDist : lightdist;
    float3 cameraVector : cameravector;
};

cbuffer transform : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewProjMatrix;
};

cbuffer light : register(b1)
{
    float3 lpos;
    float padding;
    float3 lcol;
    float padding2;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	
	//Hårdkodat kamera positionen. Hade inte tänkt implementera rörlig kamera
    output.cameraVector = output.position - float4(0.0f, 0.0f, -2.0, 0.0f);
    output.lightVector = float4(lpos, 0.0f) - output.position;
    output.lightDist = length(output.lightVector);
	
	output.position = mul(output.position, viewProjMatrix);
	output.normal = mul(float4(input.normal, 1.0f), worldMatrix);
	output.uv = input.uv;
	
	return output;
}