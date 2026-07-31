Texture2D u_scene : register(t0);
SamplerState u_samp : register(s0);

cbuffer PostParams : register(b0)
{
  float u_brightness; // -1..+1
  float u_contrast;   // 0..2 (1 = no change)
  float u_saturation; // 0..2 (1 = no change)
  float _pad;
};

struct VSOut
{
  float4 pos : SV_POSITION;
  float2 uv  : TEXCOORD0;
};

VSOut VSMain(uint id : SV_VertexID)
{
  // Fullscreen triangle (no vertex buffer).
  float2 pos = float2((id == 2) ? 3.0 : -1.0, (id == 1) ? 3.0 : -1.0);
  float2 uv  = float2((pos.x + 1.0) * 0.5, 1.0 - (pos.y + 1.0) * 0.5);

  VSOut o;
  o.pos = float4(pos, 0.0, 1.0);
  o.uv = uv;
  return o;
}

static float3 applyPost(float3 c)
{
  // Brightness
  c += u_brightness;

  // Contrast around 0.5 gray
  c = (c - 0.5) * u_contrast + 0.5;

  // Saturation
  const float luma = dot(c, float3(0.2126, 0.7152, 0.0722));
  c = lerp(float3(luma, luma, luma), c, u_saturation);

  return saturate(c);
}

float4 PSMain(VSOut i) : SV_TARGET
{
  float4 col = u_scene.Sample(u_samp, i.uv);
  col.rgb = applyPost(col.rgb);
  return col;
}

