#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

#include "SceneHierarchy/ComponentDrawers.h"

#include <string>
#include <unordered_map>

namespace Hazel {

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        explicit SceneHierarchyPanel(const Ref<Scene>& context);

        // BUG FIX: 切换场景时必须调用此函数来清理旧场景残留状态
        void SetContext(const Ref<Scene>& context);
        void OnImGuiRender();

        Entity GetSelectedEntity() const { return m_SelectionContext; }
        void   SetSelectedEntity(Entity entity);

    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

    private:
        Ref<Scene> m_Context;
        Entity     m_SelectionContext;

        // Rename 请求标志：下一帧聚焦 Tag InputText
        bool m_RenameRequested = false;
    };

} // namespace Hazel
