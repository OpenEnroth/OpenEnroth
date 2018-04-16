#include "Engine/Graphics/IRenderFactory.h"

#include "Engine/Graphics/Direct3D/Render.h"
#include "Engine/Graphics/OpenGL/RenderOpenGL.h"

using Graphics::IRenderFactory;
using Graphics::Configuration;

IRender *IRenderFactory::Create(Configuration *config)
{
    switch (config->renderer_type)
    {
        case RendererType::DirectDraw:
            return new Render(config);

        case RendererType::OpenGl:
            return new RenderOpenGL(config);
    }
    return nullptr;
}