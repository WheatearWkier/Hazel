#include "hzpch.h"
#include "SceneHierarchyPanel.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>
#include <cstring>
#include <filesystem>

#include "Hazel/Scene/Components.h"
#include "SceneHierarchy/ComponentDrawers.h"
#include "Hazel/Scene/SceneSerializer.h"

namespace Hazel {

    // ═══════════════════════════════════════════════════════
    //  构建 / 上下文
    // ═══════════════════════════════════════════════════════

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
    {
        m_Context = context;
        // BUG FIX: 切换场景时必须清除选中实体，否则旧实体 handle 悬空，
        // 导致 Properties 面板仍然渲染上一个场景的组件（残留 bug）
        m_SelectionContext = {};
    }

    // ═══════════════════════════════════════════════════════
    //  主渲染入口
    // ═══════════════════════════════════════════════════════

    void SceneHierarchyPanel::OnImGuiRender()
    {
        // ──── Scene Hierarchy ──────────────────────────────────────────
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            m_Context->m_Registry.each([&](auto entityID)
                {
                    DrawEntityNode(Entity{ entityID, m_Context.get() });
                });

            // 点击空白处取消选中
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0)
                && !ImGui::IsAnyItemHovered())
                m_SelectionContext = {};

            // 右键菜单——创建实体
            if (ImGui::BeginPopupContextWindow(
                "##HierarchyCtx",
                ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_SelectionContext = m_Context->CreateEntity("Empty Entity");

                ImGui::Separator();

                // 常用快速创建（模仿 Unity GameObject 菜单）
                if (ImGui::BeginMenu("2D Object"))
                {
                    if (ImGui::MenuItem("Sprite"))
                    {
                        auto e = m_Context->CreateEntity("Sprite");
                        e.AddComponent<SpriteRendererComponent>();
                        m_SelectionContext = e;
                    }
                    if (ImGui::MenuItem("Circle"))
                    {
                        auto e = m_Context->CreateEntity("Circle");
                        e.AddComponent<CircleRendererComponent>();
                        m_SelectionContext = e;
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Camera"))
                {
                    if (ImGui::MenuItem("Camera"))
                    {
                        auto e = m_Context->CreateEntity("Camera");
                        e.AddComponent<CameraComponent>();
                        m_SelectionContext = e;
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
        }

        ImGui::End();

        // ──── Properties ──────────────────────────────────────────────
        ImGui::Begin("Properties");
        if (m_SelectionContext)
            DrawComponents(m_SelectionContext);
        ImGui::End();
    }

    void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
    {
        m_SelectionContext = entity;
    }

    // ═══════════════════════════════════════════════════════
    //  实体节点
    // ═══════════════════════════════════════════════════════

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        const auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth;
        if (m_SelectionContext == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        const bool opened = ImGui::TreeNodeEx(
            reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))),
            flags,
            "%s", tag.c_str()
        );

        if (ImGui::IsItemClicked())
            m_SelectionContext = entity;

        bool entityDeleted = false;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Rename"))
            {
                // 选中后 Properties 面板的 Tag InputText 会聚焦
                m_SelectionContext = entity;
                m_RenameRequested = true;
            }

            if (ImGui::MenuItem("Duplicate Entity"))
            {
                // 复制实体（需要引擎侧支持 DuplicateEntity）
                Entity dup = m_Context->DuplicateEntity(entity);
                m_SelectionContext = dup;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save as Prefab"))
            {
                std::filesystem::path prefabDir = "assets/prefabs";
                std::filesystem::create_directories(prefabDir);

                std::string baseName = entity.GetName() + "Prefab";
                std::filesystem::path savePath;

                // 找一个不冲突的文件名
                int index = 0;
                do
                {
                    std::string filename = baseName;
                    if (index > 0)
                        filename += std::to_string(index);
                    filename += ".hzprefab";
                    savePath = prefabDir / filename;
                    index++;
                } while (std::filesystem::exists(savePath));

                SceneSerializer::SerializePrefab(entity, savePath);
                HZ_CORE_INFO("Saved prefab: {}", savePath.string());
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
            ImGui::TreePop();

        // BUG FIX: 延迟删除，避免在 registry.each() 迭代中修改 registry
        if (entityDeleted)
        {
            if (m_SelectionContext == entity)
                m_SelectionContext = {};
            m_Context->DestroyEntity(entity);
        }
    }

    // ═══════════════════════════════════════════════════════
    //  组件面板
    // ═══════════════════════════════════════════════════════

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        // ── Tag 编辑 ───────────────────────────────────────────────────
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256] = {};
            std::strncpy(buffer, tag.c_str(), sizeof(buffer) - 1);

            // 如果刚刚请求 Rename，自动聚焦
            if (m_RenameRequested)
            {
                ImGui::SetKeyboardFocusHere();
                m_RenameRequested = false;
            }

            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                tag = buffer;
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        // ── Add Component 按钮 ────────────────────────────────────────
        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            auto TryAddMenuItem = [&]<typename T>(const char* label)
            {
                if (!m_SelectionContext.HasComponent<T>())
                {
                    if (ImGui::MenuItem(label))
                    {
                        m_SelectionContext.AddComponent<T>();
                        ImGui::CloseCurrentPopup();
                    }
                }
            };

            ImGui::TextDisabled("Rendering");
            TryAddMenuItem.template operator() < SpriteRendererComponent > ("Sprite Renderer");
            TryAddMenuItem.template operator() < SpriteAnimatorComponent > ("Sprite Animator");
            TryAddMenuItem.template operator() < CircleRendererComponent > ("Circle Renderer");
            TryAddMenuItem.template operator() < MeshRendererComponent > ("Mesh Renderer");
            TryAddMenuItem.template operator() < CameraComponent > ("Camera");

            ImGui::Separator();
            ImGui::TextDisabled("Lighting");
            TryAddMenuItem.template operator() < DirectionalLightComponent > ("Directional Light");
            TryAddMenuItem.template operator() < PointLightComponent > ("Point Light");

            ImGui::Separator();
            ImGui::TextDisabled("Physics");
            TryAddMenuItem.template operator() < Rigidbody2DComponent > ("Rigidbody 2D");
            TryAddMenuItem.template operator() < BoxCollider2DComponent > ("Box Collider 2D");
            TryAddMenuItem.template operator() < CircleCollider2DComponent > ("Circle Collider 2D");

            ImGui::Separator();
            ImGui::TextDisabled("Scripting & Audio");
            TryAddMenuItem.template operator() < ScriptComponent > ("Script");
            TryAddMenuItem.template operator() < AudioSourceComponent > ("Audio Source");

            ImGui::Separator();
            ImGui::TextDisabled("UI");
            TryAddMenuItem.template operator() < UICanvasComponent > ("UI Canvas");
            TryAddMenuItem.template operator() < UIWidgetComponent > ("UI Widget");
            TryAddMenuItem.template operator() < UIImageComponent > ("UI Image");
            TryAddMenuItem.template operator() < UITextComponent > ("UI Text");
            TryAddMenuItem.template operator() < UIButtonComponent > ("UI Button");
            TryAddMenuItem.template operator() < UIProgressBarComponent > ("UI Progress Bar");

            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        // ── 各组件 ────────────────────────────────────────────────────
        DrawTransformComponent(entity);
        DrawCameraComponent(entity);
        DrawSpriteRendererComponent(entity);
        DrawSpriteAnimatorComponent(entity);
        DrawCircleRendererComponent(entity);
        DrawMeshRendererComponent(entity);
        DrawDirectionalLightComponent(entity);
        DrawPointLightComponent(entity);
        DrawRigidbody2DComponent(entity);
        DrawBoxCollider2DComponent(entity);
        DrawCircleCollider2DComponent(entity);
        DrawScriptComponent(entity);
        DrawAudioSourceComponent(entity);

        // ── UI 组件 ──────────────────────────────────────────────────
        DrawUICanvasComponent(entity);
        DrawUIWidgetComponent(entity);
        DrawUIImageComponent(entity);
        DrawUITextComponent(entity);
        DrawUIButtonComponent(entity);
        DrawUIProgressBarComponent(entity);
    }

} // namespace Hazel