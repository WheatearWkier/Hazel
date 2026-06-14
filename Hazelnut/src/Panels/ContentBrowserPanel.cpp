#include "hzpch.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Hazel {

    const std::filesystem::path g_AssetPath = "assets";

    // ═══════════════════════════════════════════════════════
    //  构造
    // ═══════════════════════════════════════════════════════

    ContentBrowserPanel::ContentBrowserPanel()
        : m_CurrentDirectory(g_AssetPath)
    {
        m_Icons[AssetType::Directory] = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
        m_Icons[AssetType::Unknown]   = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
        m_Icons[AssetType::Prefab]    = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
        // 以后可以扩展：
        // m_Icons[AssetType::Scene]   = Texture2D::Create("Resources/Icons/ContentBrowser/SceneIcon.png");
        // m_Icons[AssetType::Texture] = Texture2D::Create("Resources/Icons/ContentBrowser/TextureIcon.png");

        NavigateTo(g_AssetPath);
    }

    // ═══════════════════════════════════════════════════════
    //  工具函数
    // ═══════════════════════════════════════════════════════

    AssetType ContentBrowserPanel::GetAssetType(const std::filesystem::path& path) const
    {
        if (std::filesystem::is_directory(path)) return AssetType::Directory;

        const std::string ext = path.extension().string();
        if (ext == ".hazel")                                  return AssetType::Scene;
        if (ext == ".hzprefab")                               return AssetType::Prefab;
        if (ext == ".png" || ext == ".jpg"
            || ext == ".jpeg" || ext == ".tga")               return AssetType::Texture;
        if (ext == ".glsl" || ext == ".hlsl")                 return AssetType::Shader;
        if (ext == ".mp3" || ext == ".wav"
            || ext == ".ogg")                                  return AssetType::Audio;
        if (ext == ".lua" || ext == ".cs")                    return AssetType::Script;
        if (ext == ".hzmaterial")                             return AssetType::Material;

        return AssetType::Unknown;
    }

    Ref<Texture2D> ContentBrowserPanel::GetIconForType(AssetType type) const
    {
        auto it = m_Icons.find(type);
        if (it != m_Icons.end()) return it->second;
        return m_Icons.at(AssetType::Unknown);
    }

    std::vector<std::filesystem::directory_entry>
        ContentBrowserPanel::GetFilteredEntries() const
    {
        std::vector<std::filesystem::directory_entry> result;
        const std::string filter = m_SearchBuffer;

        for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const std::string name = entry.path().filename().string();
            if (filter.empty() || name.find(filter) != std::string::npos)
                result.push_back(entry);
        }

        // 目录排在前面
        std::stable_sort(result.begin(), result.end(),
            [](const auto& a, const auto& b) {
                return a.is_directory() > b.is_directory();
            });

        return result;
    }

    void ContentBrowserPanel::NavigateTo(const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path)) return;

        // 截断前进历史
        if (m_HistoryIndex >= 0)
            m_History.erase(m_History.begin() + m_HistoryIndex + 1, m_History.end());

        m_History.push_back(path);
        m_HistoryIndex = static_cast<int>(m_History.size()) - 1;
        m_CurrentDirectory = path;
        m_SelectedPath.clear();
        std::memset(m_SearchBuffer, 0, sizeof(m_SearchBuffer));
    }

    void ContentBrowserPanel::NavigateBack()
    {
        if (m_HistoryIndex > 0)
        {
            m_HistoryIndex--;
            m_CurrentDirectory = m_History[m_HistoryIndex];
            m_SelectedPath.clear();
        }
    }

    void ContentBrowserPanel::NavigateForward()
    {
        if (m_HistoryIndex < static_cast<int>(m_History.size()) - 1)
        {
            m_HistoryIndex++;
            m_CurrentDirectory = m_History[m_HistoryIndex];
            m_SelectedPath.clear();
        }
    }

    // ═══════════════════════════════════════════════════════
    //  工具栏（导航 + 面包屑 + 搜索）
    // ═══════════════════════════════════════════════════════

    void ContentBrowserPanel::DrawToolbar()
    {
        const bool canBack    = m_HistoryIndex > 0;
        const bool canForward = m_HistoryIndex < static_cast<int>(m_History.size()) - 1;

        // 后退
        if (!canBack) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
        if (ImGui::Button("< ##back") && canBack) NavigateBack();
        if (!canBack) ImGui::PopStyleVar();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Back");

        ImGui::SameLine();

        // 前进
        if (!canForward) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
        if (ImGui::Button("> ##fwd") && canForward) NavigateForward();
        if (!canForward) ImGui::PopStyleVar();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Forward");

        ImGui::SameLine();

        // 上一级（模仿 Unity 的向上箭头）
        const bool canUp = (m_CurrentDirectory != g_AssetPath);
        if (!canUp) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
        if (ImGui::Button("^ ##up") && canUp)
            NavigateTo(m_CurrentDirectory.parent_path());
        if (!canUp) ImGui::PopStyleVar();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Up one level");

        ImGui::SameLine();

        // 面包屑路径
        {
            auto relative = std::filesystem::relative(m_CurrentDirectory, g_AssetPath.parent_path());
            std::filesystem::path accumulated;
            bool first = true;

            for (const auto& part : relative)
            {
                accumulated /= part;
                if (!first) { ImGui::SameLine(0, 2); }
                first = false;

                if (ImGui::SmallButton(part.string().c_str()))
                    NavigateTo(g_AssetPath.parent_path() / accumulated);

                ImGui::SameLine(0, 2);
                ImGui::TextDisabled("/");
            }
        }

        // 右侧：搜索框 + 侧边栏切换
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200.0f);
        ImGui::SetNextItemWidth(150.0f);
        // BUG FIX: 搜索框如果清空要重置 buffer（这里已经由 NavigateTo 处理，
        // 但手动清空时同样需要响应，InputTextWithHint 本身不需要额外修改）
        ImGui::InputTextWithHint("##search", "Search...", m_SearchBuffer, sizeof(m_SearchBuffer));

        ImGui::SameLine();
        if (ImGui::Button(m_ShowSidebar ? "<<" : ">>"))
            m_ShowSidebar = !m_ShowSidebar;
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Folder Tree");

        ImGui::Separator();
    }

    // ═══════════════════════════════════════════════════════
    //  侧边栏：目录树
    // ═══════════════════════════════════════════════════════

    void ContentBrowserPanel::DrawSidebar()
    {
        if (!m_ShowSidebar) return;

        ImGui::BeginChild("##sidebar", ImVec2(150, 0), true);

        std::function<void(const std::filesystem::path&, int)> DrawTree;
        DrawTree = [&](const std::filesystem::path& dir, int depth)
            {
                for (const auto& entry : std::filesystem::directory_iterator(dir))
                {
                    if (!entry.is_directory()) continue;

                    const std::string name = entry.path().filename().string();
                    const bool isCurrent = (entry.path() == m_CurrentDirectory);

                    ImGuiTreeNodeFlags flags =
                        ImGuiTreeNodeFlags_OpenOnArrow |
                        ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (isCurrent)  flags |= ImGuiTreeNodeFlags_Selected;
                    if (depth == 0) flags |= ImGuiTreeNodeFlags_DefaultOpen;

                    const bool open = ImGui::TreeNodeEx(
                        entry.path().string().c_str(), flags, "%s", name.c_str());

                    if (ImGui::IsItemClicked())
                        NavigateTo(entry.path());

                    if (open)
                    {
                        DrawTree(entry.path(), depth + 1);
                        ImGui::TreePop();
                    }
                }
            };

        DrawTree(g_AssetPath, 0);
        ImGui::EndChild();
    }

    // ═══════════════════════════════════════════════════════
    //  文件网格
    // ═══════════════════════════════════════════════════════

    void ContentBrowserPanel::DrawFileGrid()
    {
        ImGui::BeginChild("##filegrid", ImVec2(0, 0), false);

        // 网格布局参数
        const float cellSize    = m_ThumbnailSize + m_Padding;
        const float panelWidth  = ImGui::GetContentRegionAvail().x;
        const int   columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));

        // 支持 Ctrl+A 全选（实用习惯）
        if (ImGui::IsWindowFocused() && ImGui::GetIO().KeyCtrl
            && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
        {
            // 此处可扩展多选，暂时只是 UI 预留
        }

        ImGui::Columns(columnCount, nullptr, false);

        const auto entries = GetFilteredEntries();

        for (const auto& entry : entries)
        {
            const auto& path         = entry.path();
            const auto  relativePath = std::filesystem::relative(path, g_AssetPath);
            const auto  filename     = relativePath.filename().string();
            const bool  isSelected   = (path == m_SelectedPath);

            ImGui::PushID(path.string().c_str());  // BUG FIX: 用完整路径作 ID，避免同名文件冲突

            const AssetType     type = GetAssetType(path);
            const Ref<Texture2D>& icon = GetIconForType(type);

            // 选中高亮背景
            if (isSelected)
            {
                ImVec2 pos  = ImGui::GetCursorScreenPos();
                ImVec2 size = ImVec2(m_ThumbnailSize + m_Padding,
                                     m_ThumbnailSize + m_Padding + 20.0f);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    pos, ImVec2(pos.x + size.x, pos.y + size.y),
                    IM_COL32(70, 130, 180, 80), 4.0f
                );
            }

            // 悬停高亮
            ImVec2 hoverCheckPos  = ImGui::GetCursorScreenPos();
            ImVec2 hoverCheckSize = ImVec2(m_ThumbnailSize + m_Padding,
                                           m_ThumbnailSize + m_Padding + 20.0f);

            // 图标按钮
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.08f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1, 1, 1, 0.12f));

            ImGui::ImageButton(
                reinterpret_cast<ImTextureID>(static_cast<uint64_t>(icon->GetRendererID())),
                { m_ThumbnailSize, m_ThumbnailSize },
                { 0, 1 }, { 1, 0 }
            );

            ImGui::PopStyleColor(3);

            // 拖拽源
            if (ImGui::BeginDragDropSource())
            {
                const wchar_t* itemPath = relativePath.c_str();
                ImGui::SetDragDropPayload(
                    "CONTENT_BROWSER_ITEM",
                    itemPath,
                    (wcslen(itemPath) + 1) * sizeof(wchar_t)
                );
                // 显示缩略图 + 文件名作为拖拽预览
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(static_cast<uint64_t>(icon->GetRendererID())),
                    { 32, 32 }, { 0, 1 }, { 1, 0 }
                );
                ImGui::SameLine();
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }

            // 单击选中
            if (ImGui::IsItemClicked())
                m_SelectedPath = path;

            // 双击：目录则进入，文件（场景）则可触发打开
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (entry.is_directory())
                    NavigateTo(path);
                // 其他类型的双击动作可在这里扩展
            }

            // BUG FIX: 右键菜单使用 BeginPopupContextItem()，
            // 原来所有 item 共用 "##itemctx" 字符串 ID 导致菜单混乱；
            // 改为不传 ID（使用 ImGui 的 item 上下文），由 PushID 隔离。
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Show in Explorer"))
                {
                    // Windows: ShellExecuteW(nullptr, L"explore", path.c_str(), ...)
                }

                if (!entry.is_directory())
                {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Rename"))
                    {
                        // 可扩展：进入内联重命名模式
                    }
                    if (ImGui::MenuItem("Delete"))
                    {
                        // 可扩展：弹出确认对话框后删除
                    }
                }
                ImGui::EndPopup();
            }

            // 工具提示：悬停显示完整路径
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", path.string().c_str());

            // 文件名（自动换行）
            ImGui::TextWrapped("%s", filename.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        // 空白区右键菜单：在当前目录创建文件夹等
        if (ImGui::BeginPopupContextWindow(
            "##DirCtx",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("New Folder"))
            {
                // 可扩展：创建新文件夹并进入重命名模式
            }
            if (ImGui::MenuItem("Refresh"))
            {
                // 强制刷新目录列表（目前每帧都扫描，此处可加缓存刷新）
            }
            ImGui::EndPopup();
        }

        ImGui::Columns(1);
        ImGui::EndChild();
    }

    // ═══════════════════════════════════════════════════════
    //  Inspector 右侧详情
    // ═══════════════════════════════════════════════════════

    void ContentBrowserPanel::DrawInspector()
    {
        if (!m_ShowInspector) return;

        ImGui::BeginChild("##inspector", ImVec2(180, 0), true);
        ImGui::TextDisabled("INSPECTOR");
        ImGui::Separator();

        if (!m_SelectedPath.empty() && std::filesystem::exists(m_SelectedPath))
        {
            const AssetType type = GetAssetType(m_SelectedPath);
            const Ref<Texture2D>& icon = GetIconForType(type);

            // 预览图
            ImGui::Image(
                reinterpret_cast<ImTextureID>(static_cast<uint64_t>(icon->GetRendererID())),
                ImVec2(160, 160), { 0, 1 }, { 1, 0 }
            );

            ImGui::Spacing();

            const std::string name = m_SelectedPath.filename().string();
            const std::string ext  = m_SelectedPath.extension().string();

            ImGui::TextDisabled("Name");
            ImGui::TextWrapped("%s", name.c_str());

            ImGui::Spacing();
            ImGui::TextDisabled("Type");

            const char* typeStr = "Unknown";
            switch (type)
            {
            case AssetType::Directory: typeStr = "Folder";   break;
            case AssetType::Scene:     typeStr = "Scene";    break;
            case AssetType::Texture:   typeStr = "Texture";  break;
            case AssetType::Shader:    typeStr = "Shader";   break;
            case AssetType::Audio:     typeStr = "Audio";    break;
            case AssetType::Script:    typeStr = "Script";   break;
            case AssetType::Prefab:    typeStr = "Prefab";   break;
            default: break;
            }
            ImGui::Text("%s", typeStr);

            if (!ext.empty())
            {
                ImGui::Spacing();
                ImGui::TextDisabled("Extension");
                ImGui::Text("%s", ext.c_str());
            }

            if (!std::filesystem::is_directory(m_SelectedPath))
            {
                ImGui::Spacing();
                ImGui::TextDisabled("Size");
                const auto bytes = std::filesystem::file_size(m_SelectedPath);
                if (bytes < 1024)
                    ImGui::Text("%llu B", (unsigned long long)bytes);
                else if (bytes < 1024 * 1024)
                    ImGui::Text("%.1f KB", bytes / 1024.0f);
                else
                    ImGui::Text("%.1f MB", bytes / (1024.0f * 1024.0f));
            }

            // 最后修改时间
            ImGui::Spacing();
            ImGui::TextDisabled("Modified");
            auto ftime = std::filesystem::last_write_time(m_SelectedPath);
            // 简单显示（完整格式化可用 std::chrono）
            ImGui::TextDisabled("(see OS)");
        }
        else
        {
            ImGui::TextDisabled("Nothing selected");
        }

        ImGui::EndChild();
    }

    // ═══════════════════════════════════════════════════════
    //  状态栏
    // ═══════════════════════════════════════════════════════

    void ContentBrowserPanel::DrawStatusBar()
    {
        ImGui::Separator();

        // 缩略图大小滑块
        ImGui::SetNextItemWidth(80.0f);
        ImGui::SliderFloat("##thumb", &m_ThumbnailSize, 32.0f, 128.0f, "%.0f");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Thumbnail size");
        ImGui::SameLine();
        ImGui::TextDisabled("size");

        ImGui::SameLine(0, 20);

        // BUG FIX: 原来在 DrawStatusBar 里又调用一次 GetFilteredEntries()，
        // 导致每帧遍历目录两次。改为使用主渲染函数中已有的缓存结果。
        // 由于此函数是独立调用的，这里保持调用，但加上注释提醒未来可缓存。
        const auto entries = GetFilteredEntries(); // TODO: 可缓存到成员变量
        ImGui::TextDisabled("%zu items", entries.size());

        if (!m_SelectedPath.empty())
        {
            ImGui::SameLine();
            ImGui::TextDisabled("|  %s", m_SelectedPath.filename().string().c_str());
        }

        // Inspector 切换按钮放状态栏右侧
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.0f);
        if (ImGui::SmallButton("i"))
            m_ShowInspector = !m_ShowInspector;
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Inspector");
    }

    // ═══════════════════════════════════════════════════════
    //  主入口
    // ═══════════════════════════════════════════════════════

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        DrawToolbar();

        // 侧边栏 + 文件网格 + Inspector 水平排列
        if (m_ShowSidebar)
        {
            DrawSidebar();
            ImGui::SameLine();
        }

        if (m_ShowInspector)
        {
            const float inspectorWidth = 190.0f;
            ImGui::BeginChild("##gridwrap",
                ImVec2(ImGui::GetContentRegionAvail().x - inspectorWidth, 0), false);
            DrawFileGrid();
            ImGui::EndChild();

            ImGui::SameLine();
            DrawInspector();
        }
        else
        {
            DrawFileGrid();
        }

        DrawStatusBar();

        ImGui::End();
    }

} // namespace Hazel
