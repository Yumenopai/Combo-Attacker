
struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 inverse_view_projection;
    float4 camera_position;
};

static const float PI = 3.141592654f;