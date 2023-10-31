#include "WaterSurface.hlsli"

Texture2D diffuseMap : register(t0);
SamplerState linearSampler : register(s0);
Texture2D normalMap : register(t1);
Texture2D shadowMap : register(t2);
SamplerComparisonState shadowSampler : register(s1);

float4 main(VS_OUT pin) : SV_TARGET
{
   	// �g�̗h�炬��UV���W�ɗ^���� 
    float2 texcoord1 = pin.texcoord + wave1;
    float2 texcoord2 = pin.texcoord + wave2;

    float4 color;
	{ 
        float4 DC1 = diffuseMap.Sample(linearSampler, texcoord1);
        float4 DC2 = diffuseMap.Sample(linearSampler, texcoord2);
        color = ((DC1 + DC2) * 0.5f) * materialColor;
    }
    
	// �@���}�b�v����xyz�������擾����( -1 �` +1 )�̊ԂɃX�P�[�����O
    float3 N = normalize(pin.normal);
    float3 T = normalize(pin.tangent);
    float3 B = normalize(cross(N, T));
	{ 
        float3 normal1 = normalMap.Sample(linearSampler, texcoord1).xyz * 2.0f - 1.0f;
        float3 normal2 = normalMap.Sample(linearSampler, texcoord2).xyz * 2.0f - 1.0f;
        float3 normal = normal1 + normal2;
        N = normalize(normal.x * T + normal.y * B + normal.z * N);
    }

    float3 L = normalize(-lightDirection.xyz);
    float power = max(0, dot(L, N));

    power = power * 0.7 + 0.3f;

    color.rgb *= lightColor.rgb * power;

	// phong���� �X�Ɍ��𑫂�
    float3 V = normalize(cameraPosition.xyz - pin.position);
    float specular = pow(max(0, dot(N, normalize(V + L))), 128);
    color.rgb += specular;
	
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
	
    return color;
}