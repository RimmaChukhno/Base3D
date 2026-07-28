cbuffer PerObject : register(b0)
{
  row_major float4x4 u_mvp;
  float4 u_tint;
};

struct VSIn
{
  float3 pos   : POSITION;
  float4 color : COLOR;
};

struct VSOut
{
  float4 pos   : SV_POSITION;
  float4 color : COLOR;
};

VSOut VSMain(VSIn v)
{
  VSOut o;
  o.pos = mul(float4(v.pos, 1.0f), u_mvp);
  o.color = v.color * u_tint;
  return o;
}

float4 PSMain(VSOut i) : SV_TARGET
{
  return i.color;
}

