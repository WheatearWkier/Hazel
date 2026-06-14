#pragma once

#include "Hazel.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/AnimationEditorPanel.h"

#include <filesystem>

namespace Hazel {

    /// @brief 2D/3D编辑器共用基类
    ///
    /// 职责：场景状态机、Framebuffer、Entity Picking、
    ///       公共面板（Hierarchy/ContentBrowser/Animation）、
    ///       公共事件（键盘/鼠标）、公共UI（MenuBar/Toolbar/Viewport/Stats）。
    ///
    /// 子类扩展点：
    ///   - OnBeginRender()  ：在清屏之后、Scene Update之前执行（如3D BeginScene/Skybox）
    ///   - OnOverlayRender()：在Scene Update之后、Framebuffer Unbind之前执行（叠加层绘制）
    ///   - OnImGuiExtra()   ：在公共面板之后追加子类专属面板（如3D Settings）
    class EditorLayerBase : public Layer
    {
    public:
        enum class SceneState : uint8_t
        {
            Edit = 0,
            Play = 1
        };

    public:
        explicit EditorLayerBase(const std::string& debugName);
        virtual ~EditorLayerBase() = default;

        // ── Layer 接口 ───────────────────────────────────────────────────────
        virtual void OnAttach()            override;
        virtual void OnDetach()            override;
        virtual void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender()       override;
        virtual void OnEvent(Event& event) override;

    protected:
        // ── 子类扩展钩子 ─────────────────────────────────────────────────────

        /// 清屏之后、Scene Update之前（3D子类在此BeginScene+DrawSkybox）
        virtual void OnBeginRender() {}

        /// Scene Update之后、Framebuffer Unbind之前（overlay叠加绘制）
        virtual void OnOverlayRender() {}

        //
        virtual void OnPostSceneUpdate() {}

        /// 公共ImGui面板渲染完毕后调用（子类追加Settings等专属面板）
        virtual void OnImGuiExtra() {}

        // ── 场景状态机（供子类调用）──────────────────────────────────────────
        void TransitionToEditScene(Ref<Scene> newScene,
                                   const std::filesystem::path& scenePath = {});
        void TransitionToPlay();
        void TransitionToStop();

        // ── 场景文件操作 ──────────────────────────────────────────────────────
        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveSceneAs();
        void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);
        void InstantiatePrefab(const std::filesystem::path& path);
        void OnDuplicateEntity();

        // ── 访问器（子类只读）────────────────────────────────────────────────
        SceneState            GetSceneState()   const { return m_SceneState; }
        Ref<Scene>            GetActiveScene()  const { return m_ActiveScene; }
        const EditorCamera&   GetEditorCamera() const { return m_EditorCamera; }
        EditorCamera&         GetEditorCamera()       { return m_EditorCamera; }
        const glm::vec2&      GetViewportSize() const { return m_ViewportSize; }
        bool                  IsViewportHovered() const { return m_ViewportHovered; }

        SceneHierarchyPanel&  GetHierarchyPanel()   { return m_SceneHierarchyPanel; }
        ContentBrowserPanel&  GetContentBrowserPanel() { return m_ContentBrowserPanel; }

    private:
        // ── 事件处理 ─────────────────────────────────────────────────────────
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);

        // ── UI子函数 ──────────────────────────────────────────────────────────
        void UI_MenuBar();
        void UI_Toolbar();
        void UI_Viewport();
        void UI_Stats();

        // ── 内部辅助 ──────────────────────────────────────────────────────────
        void SyncPanels();
        void ClearEntitySelection();

    protected:
        // ── 渲染 ──────────────────────────────────────────────────────────────
        Ref<Framebuffer> m_Framebuffer;
        EditorCamera     m_EditorCamera;

        // ── 场景 ──────────────────────────────────────────────────────────────
        Ref<Scene>            m_ActiveScene;
        Ref<Scene>            m_EditorScene;
        std::filesystem::path m_EditorScenePath;
        SceneState            m_SceneState = SceneState::Edit;

        // ── 实体 ──────────────────────────────────────────────────────────────
        Entity m_HoveredEntity;

        // ── 视口 ──────────────────────────────────────────────────────────────
        glm::vec2 m_ViewportSize    = { 0.0f, 0.0f };
        glm::vec2 m_ViewportBounds[2] = {};
        bool      m_ViewportFocused = false;
        bool      m_ViewportHovered = false;

        // ── Gizmo ─────────────────────────────────────────────────────────────
        int  m_GizmoType           = -1;
        bool m_ShowPhysicsColliders = false;

        // ── 图标 ──────────────────────────────────────────────────────────────
        Ref<Texture2D> m_IconPlay;
        Ref<Texture2D> m_IconStop;

        // ── 面板 ──────────────────────────────────────────────────────────────
        SceneHierarchyPanel  m_SceneHierarchyPanel;
        ContentBrowserPanel  m_ContentBrowserPanel;
        AnimationEditorPanel m_AnimationEditorPanel;
        Timestep             m_LastTimestep;
    };

} // namespace Hazel
