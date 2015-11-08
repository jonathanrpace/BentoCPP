#pragma once

#include <bento/core/Process.h>
#include <bento/core/SharedObject.h>

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
	};
}
