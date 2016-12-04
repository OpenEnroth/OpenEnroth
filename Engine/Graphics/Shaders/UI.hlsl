cbuffer fast: register(b0)
{
    float2 position : packoffset(c0.x);
    float2 size     : packoffset(c0.z);
};

cbuffer slow: register(b1)
{
};


struct VInput
{
    float4 pos: POSITION0;
};

struct VOutput
{
    float4 pos: SV_Position0;
    float2 uv:  TEXCOORD0;
};


VOutput vs(VInput vin)
{
    VOutput vout;

    float2 normalized_pos = vin.pos.xy * position.xy + vin.pos.zw * size.xy;
    vout.pos.xy = float2(2, -2) * normalized_pos + float2(-1, 1);
    vout.pos.zw = float2(0, 1);
    vout.uv = vin.pos.zw;

    return vout;
}




SamplerState      basic_sampler: register(s0);
Texture2D<float4> image: register(t0);

float4 main(VOutput pin): SV_Target0
{
    return image.Sample(basic_sampler, pin.uv);
}