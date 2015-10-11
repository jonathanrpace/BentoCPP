#pragma once

#include <memory>
#include <typeinfo>

#include <Ramen/Core/IComponent.h>

namespace Ramen
{
	template<typename T>
	class ComponentBase : public IComponent
	{
	public:
		// From IComponent
		virtual const std::type_info& typeInfo() override;

		static std::shared_ptr<T> Create();
	};

	template<typename T>
	std::shared_ptr<T> ComponentBase<T>::Create()
	{
		return std::make_shared<T>();
	}

	template<typename T>
	inline const std::type_info & ComponentBase<T>::typeInfo()
	{
		return typeid(T);
	}
}
