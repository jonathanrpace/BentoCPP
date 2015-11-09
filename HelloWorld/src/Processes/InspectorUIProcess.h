#pragma once

#include <bento/core/Process.h>
#include <bento/core/SharedObject.h>
#include <bento/core/Component.h>
#include <bento/core/Process.h>
#include <bento/core/Reflection.h>

namespace bento
{
	class InspectorUIProcess
		: public Process
		, public SharedObject<InspectorUIProcess>
	{
	public:
		InspectorUIProcess(std::string _name = "InspectorUIProcess");

		// From Process
		virtual void Advance(double dt) override;
		void AddControlsIfReflectable(ComponentPtr _component);
		void AddControlsIfReflectable(ProcessPtr _process);
		void AddControls(Reflectable * _reflectable);
	};
}
