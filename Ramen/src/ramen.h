#pragma once

#include <Ramen/Core/IComponent.h>
#include <Ramen/Core/IProcess.h>
#include <Ramen/Core/IInputManager.h>
#include <Ramen/Core/ISceneObject.h>
#include <Ramen/Core/AbstractValidatable.h>
#include <Ramen/Core/INode.h>
#include <Ramen/Core/IProcess.h>
#include <Ramen/Core/NamedObject.h>
#include <Ramen/Core/NodeGroup.h>
#include <Ramen/Core/Scene.h>
#include <Ramen/Core/Logging.h>
#include <Ramen/Core/SharedObject.h>

namespace Ramen
{
	char* ResourcePath();
	void ResourcePath(char* _resourcePath);
}