struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
    float3 lightVector : lightvector;
    float lightDist : lightdist;
    float3 cameraVector : cameravector;
};

Texture2D inputTexture;

SamplerState inputSampler;

cbuffer light : register(b0)
{
    float3 lpos;
    float padding;
    float3 lcol;
    float padding2;
};

cbuffer material : register(b1)
{
    float ka;
    float kd;
    float ks;
    int shiny;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 output = inputTexture.Sample(inputSampler, input.uv);
    
    float3 ambient = lcol * ka;
    
    float3 lightPath = normalize(input.lightVector);
    
    float3 norm = normalize(input.normal);
    float3 diffuse = 1/input.lightDist * lcol * kd * max(0.0f, dot(norm, lightPath));
    
    float3 reflected = reflect(lightPath, norm);
    
    float3 specular = ks * float3(1, 1, 1) * pow(max(0.0f, dot(reflected, input.cameraVector)), shiny);
    
    float3 lightValue = ambient + diffuse + specular;
    
    return float4(output * lightValue, 1.0f);
}