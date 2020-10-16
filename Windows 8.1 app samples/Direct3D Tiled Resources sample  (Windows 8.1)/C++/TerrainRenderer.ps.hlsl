TextureCube<float3> ColorTexture : register(t0);
TextureCube<float> ColorResidency : register(t1);
TextureCube<float2> NormalTexture : register(t2);
TextureCube<float> NormalResidency : register(t3);
SamplerState Trilinear : register(s0);
SamplerState MaxFilter : register(s1);

cbuffer PixelShaderConstants : register(b0)
{
    float3 SunPosition;
};

struct PS_IN
{
    float3 tex : TEXCOORD0;
    float3 utan : TANGENT0;
    float3 vtan : TANGENT1;
};

float4 main(PS_IN input) : SV_TARGET
{
    float3 tex = normalize(input.tex);

    float diffuseMinLod = ColorResidency.Sample(MaxFilter, tex) * 16.0f;
    float3 diffuse = ColorTexture.Sample(Trilinear, tex, diffuseMinLod);

    float normalMinLod = NormalResidency.Sample(MaxFilter, tex) * 16.0f;
    float2 tangent = NormalTexture.Sample(Trilinear, tex, normalMinLod);
    float3 normal = tangent.x * input.utan + tangent.y * input.vtan;
    float arg = 1.0f - tangent.x * tangent.x - tangent.y * tangent.y;
    if (arg > 0.0f)
    {
        normal += sqrt(arg) * cross(input.utan, input.vtan);
    }
    float ambient = 0.2f;
    float lighting = ambient + (1.0f - ambient) * saturate(dot(normal,SunPosition));
    return float4(diffuse * lighting, 1.0f);
}
