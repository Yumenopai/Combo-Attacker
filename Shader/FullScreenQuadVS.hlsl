#include "FullScreenQuad.hlsli"

VS_OUT main(uint vertexId : SV_VERTEXID)
{
	const float2 position[4] =
	{
		{-1,+1},//����
		{+1,+1},//�E��
		{-1,-1},//����
		{+1,-1},//�E��
	};
	const float2 texcoords[4] =
	{
		{0,0},//����
		{1,0},//�E��
		{0,1},//����
		{1,1},//�E��
	};
	VS_OUT vout;
	vout.position = float4(position[vertexId], 0, 1);
	vout.texcoord = texcoords[vertexId];
	return vout;
}