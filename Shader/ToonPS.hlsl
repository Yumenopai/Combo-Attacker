#include "Toon.hlsli"

Texture2D diffuseMap : register(t0);
SamplerState linearSampler : register(s0);
Texture2D normalMap : register(t1);
Texture2D shadowMap : register(t2);
SamplerComparisonState shadowSampler : register(s1);
Texture2D toonTex : register(t3); // �g�D�[���e�N�X�`��

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = diffuseMap.Sample(linearSampler, pin.texcoord) * materialColor;

    float3 N = normalize(pin.normal);
    float3 T = normalize(pin.tangent);
    float3 B = normalize(cross(N, T));

    float3 normal = normalMap.Sample(linearSampler, pin.texcoord).xyz;
    normal = (normal * 2.0f) - 1.0f;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

    float3 L = normalize(-lightDirection.xyz);
	
    const float shadowBias = 0.001f;
    float shadowFactor = 0.0f;
    const float2 shadowTexelOffsets[9] =
    {
        float2(-shadowTexelSize, -shadowTexelSize), //����
		float2(0.0f, -shadowTexelSize), //��
		float2(shadowTexelSize, -shadowTexelSize), //�E��
		float2(-shadowTexelSize, 0.0f), //��
		float2(0.0f, 0.0f), //��
		float2(shadowTexelSize, 0.0f), //�E
		float2(-shadowTexelSize, shadowTexelSize), //����
		float2(0.0f, shadowTexelSize), //��
		float2(shadowTexelSize, shadowTexelSize) //�E��
    };
	
	[unroll]
    for (int i = 0; i < 9; ++i)
    {
        shadowFactor += shadowMap.SampleCmpLevelZero(shadowSampler,
			pin.shadow.xy + shadowTexelOffsets[i], pin.shadow.z - shadowBias).r;
    }
    shadowFactor /= 9.0f;
	
    float3 shadow = lerp(shadowColor.rgb, float3(1.0f, 1.0f, 1.0f), shadowFactor);
    color.rgb *= shadow;

    // Toon����
  	//�n�[�t�����o�[�g�g�U�Ɩ��ɂ�郉�C�e�B���O�v�Z
    float p = saturate(dot(N, L) * 0.3f + 0.7f);
	
    //�v�Z���ʂ��g�D�[���V�F�[�_�[�p�̃e�N�X�`������F���t�F�b�`����
    float4 c = toonTex.Sample(linearSampler, float2(p, 0.1f));
    c = max(c, 0.6);
    color.rgb *= c;
    return color;
}