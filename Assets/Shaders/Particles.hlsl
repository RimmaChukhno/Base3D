Texture2D u_tex : register(t0);
SamplerState u_samp : register(s0);

cbuffer PerFrame : register(b0)
{
  row_major float4x4 u_mvp;
};

struct VSIn
{
  float3 pos   : POSITION;
  float2 uv    : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOut
{
  float4 pos   : SV_POSITION;
  float2 uv    : TEXCOORD0;
  float4 color : COLOR;
};

VSOut VSMain(VSIn v)
{
  VSOut o;
  o.pos = mul(float4(v.pos, 1.0f), u_mvp);
  o.uv = v.uv;
  o.color = v.color;
  return o;
}

float4 PSMain(VSOut i) : SV_TARGET
{
  float4 tex = u_tex.Sample(u_samp, i.uv);
  return tex * i.color;
}

