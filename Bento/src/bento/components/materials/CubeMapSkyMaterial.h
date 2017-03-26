#pragma once

// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/components/textures/TextureResource.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SerializableBase.h>

namespace bento
{
	class CubeMapSkyMaterial
		: public Component
		, public SharedObject<CubeMapSkyMaterial>
		, public SerializableBase
		, public IInspectable
	{
	public:
		CubeMapSkyMaterial(std::string _name = "CubeMapSkyMaterial");

		virtual void AddUIElements() override;

		// Textures
		TextureResource envMap;

		float intensity;
		float mipLevel;
	};
}