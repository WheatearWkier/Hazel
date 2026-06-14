#include <Hazel.h>
#include "Editor/ModeSelectLayer.h"

#include <Hazel/Core/EntryPoint.h>

namespace Hazel
{

    class Hazelnut : public Application
    {
    public:
        Hazelnut(ApplicationCommandLineArgs args)
            : Application("Hazelnut", args)
        {
            // 启动时显示模式选择界面
            // ModeSelectLayer 完成选择后会自动推入对应的 EditorLayer2D / EditorLayer3D
            PushLayer(new ModeSelectLayer());
        }

        ~Hazelnut() = default;
    };

    Application* CreateApplication(ApplicationCommandLineArgs args)
    {
        return new Hazelnut(args);
    }

} // namespace Hazel
