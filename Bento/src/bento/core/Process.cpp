#include "Process.h"

namespace bento
{
	Process::Process(std::string _name, const std::type_info& _typeInfo)
		: SceneObject(_name, _typeInfo)
		, m_scene(nullptr)
	{

	}

	void Process::BindToScene(Scene& _scene)
	{
		m_scene = &_scene;
	}

	void Process::UnbindFromScene(Scene& _scene)
	{
		m_scene = nullptr;
	}
}
