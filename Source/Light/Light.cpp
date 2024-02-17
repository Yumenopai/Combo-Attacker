#include "Graphics/Graphics.h"
#include "Light.h"
#include "imgui.h"

Light::Light(LightType lightType) : lightType(lightType)
{
}

void Light::PushRenderContext(RenderContext& rc) const
{
	// “o˜^‚³‚ê‚Ä‚¢‚éŒõŒ¹‚Ìî•ñ‚ðÝ’è
	switch (lightType)
	{
		case LightType::Directional:
		{
			rc.directionalLightData.direction.x = direction.x;
			rc.directionalLightData.direction.y = direction.y;
			rc.directionalLightData.direction.z = direction.z;
			rc.directionalLightData.direction.w = 0.0f;
			rc.directionalLightData.color = color;
			break;
		}
		case LightType::Point:
		{
			if (rc.pointLightCount >= PointLightMax) break;

			rc.pointLightData[rc.pointLightCount].position.x = position.x;
			rc.pointLightData[rc.pointLightCount].position.y = position.y;
			rc.pointLightData[rc.pointLightCount].position.z = position.z;
			rc.pointLightData[rc.pointLightCount].position.w = 1.0f;
			rc.pointLightData[rc.pointLightCount].color = color;
			rc.pointLightData[rc.pointLightCount].range = range;
			++rc.pointLightCount;
			break;
		}
		case LightType::Spot:
		{
			if (rc.spotLightCount >= SpotLightMax) break;
			rc.spotLightData[rc.spotLightCount].position.x = position.x;
			rc.spotLightData[rc.spotLightCount].position.y = position.y;
			rc.spotLightData[rc.spotLightCount].position.z = position.z;
			rc.spotLightData[rc.spotLightCount].position.w = 1.0f;
			rc.spotLightData[rc.spotLightCount].direction.x = direction.x;
			rc.spotLightData[rc.spotLightCount].direction.y = direction.y;
			rc.spotLightData[rc.spotLightCount].direction.z = direction.z;
			rc.spotLightData[rc.spotLightCount].direction.w = 0.0f;
			rc.spotLightData[rc.spotLightCount].color = color;
			rc.spotLightData[rc.spotLightCount].range = range;
			rc.spotLightData[rc.spotLightCount].innerCorn = innerCorn;
			rc.spotLightData[rc.spotLightCount].outerCorn = outerCorn;
			++rc.spotLightCount;
			break;
		}
	}
}
void Light::DrawDebugGUI()
{
	static constexpr char* lightTypeName[] =
	{
		"Directional",
		"Point",
		"Spot",
	};

	if (ImGui::TreeNode(lightTypeName[static_cast<int>(lightType)]))
	{
		switch (lightType)
		{
			case LightType::Directional:
			{
				if (ImGui::SliderFloat3("direction", &direction.x, -1.0f, 1.0f))
				{
					float l = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
					direction.x /= l;
					direction.y /= l;
					direction.z /= l;
				}
				ImGui::ColorEdit3("color", &color.x);
				break;
			}
			case LightType::Point:
			{
				ImGui::DragFloat3("position", &position.x);
				ImGui::ColorEdit3("color", &color.x);
				ImGui::DragFloat("range", &range, 0.1f, 0, FLT_MAX);
				break;
			}
			case LightType::Spot:
			{
				ImGui::DragFloat3("position", &position.x);
				ImGui::DragFloat3("direction", &direction.x, 0.01f);
				DirectX::XMStoreFloat3(&direction,
					DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction)));
				ImGui::ColorEdit3("color", &color.x);
				ImGui::DragFloat("range", &range, 0.1f, 0, FLT_MAX);
				ImGui::SliderFloat("innerCorn", &innerCorn, 0, 1.0f);
				ImGui::SliderFloat("outerCorn", &outerCorn, 0, 1.0f);
				break;
			}
		}
		ImGui::TreePop();
	}
}