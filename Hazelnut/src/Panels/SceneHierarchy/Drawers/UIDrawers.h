#pragma once
#include "Hazel/Scene/Entity.h"

namespace Hazel {
    void DrawUICanvasComponent(Entity entity);
    void DrawUIWidgetComponent(Entity entity);
    void DrawUIImageComponent(Entity entity);
    void DrawUITextComponent(Entity entity);
    void DrawUIButtonComponent(Entity entity);
    void DrawUIProgressBarComponent(Entity entity);
} // namespace Hazel