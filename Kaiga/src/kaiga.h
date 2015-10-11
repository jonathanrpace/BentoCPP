#pragma once

#include <memory>

#include <glew.h>
#include <glfw3.h>

#include <Kaiga/Core/IGeometry.h>
#include <Kaiga/Core/IRenderer.h>
#include <Kaiga/Core/IRenderPass.h>
#include <Kaiga/Core/IRenderTarget.h>
#include <Kaiga/Core/ITexture.h>
#include <Kaiga/Core/ITextureRect.h>
#include <Kaiga/Core/ITextureSquare.h>
#include <Kaiga/Core/RenderParams.h>
#include <Kaiga/Core/RenderPhase.h>
#include <Kaiga/Util/GLErrorUtil.h>
#include <Kaiga/Components/Geom/AbstractGeometry.h>
#include <Kaiga/RenderPasses/AbstractNodeGroupRenderPass.h>
#include <Kaiga/RenderTargets/AbstractRenderTarget.h>
#include <Kaiga/Shaders/Core/AbstractShader.h>
#include <Kaiga/Shaders/Core/AbstractShaderStage.h>
#include <Kaiga/Textures/AbstractTexture.h>
