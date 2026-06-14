#include "hzpch.h"
#include "Audiodrawer.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Audio/AudioEngine.h"
#include "Hazel/Core/Log.h"
#include <imgui/imgui.h>
#include <filesystem>

namespace Hazel {

    void DrawAudioSourceComponent(Entity entity)
    {
        if (!entity.HasComponent<AudioSourceComponent>()) return;
        auto& asc = entity.GetComponent<AudioSourceComponent>();

        bool open = ImGui::TreeNodeEx("AudioSourceComponent",
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_Framed      |
            ImGuiTreeNodeFlags_SpanAvailWidth);

        if (open)
        {
            // ── 音频文件路径 ──────────────────────────────────────────
            // BUG FIX: 同 ScriptDrawer，改为局部 buffer + entity PushID
            char buf[256];
            memset(buf, 0, sizeof(buf));
            strncpy_s(buf, asc.AudioFilePath.c_str(), sizeof(buf) - 1);

            ImGui::Text("Audio File");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1.0f);

            ImGui::PushID((int)(uint32_t)entity);
            if (ImGui::InputText("##AudioFilePath", buf, sizeof(buf)))
                asc.AudioFilePath = buf;
            ImGui::PopID();

            // 拖放音频文件
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* wpath = (const wchar_t*)payload->Data;
                    std::filesystem::path dropped(wpath);
                    auto ext = dropped.extension().string();

                    if (ext == ".wav" || ext == ".mp3" ||
                        ext == ".ogg" || ext == ".flac")
                    {
                        asc.AudioFilePath = dropped.string();
                        HZ_CORE_INFO("AudioFilePath set to: {0}", asc.AudioFilePath);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (asc.AudioFilePath.empty())
                ImGui::TextDisabled("  Drag an audio file here, or type path");

            // ── 音量 ──────────────────────────────────────────────────
            ImGui::SliderFloat("Volume", &asc.Volume, 0.0f, 1.0f);

            // ── 循环 / PlayOnStart ────────────────────────────────────
            ImGui::Checkbox("Loop", &asc.Loop);
            ImGui::SameLine();
            ImGui::Checkbox("Play On Start", &asc.PlayOnStart);

            ImGui::TreePop();
        }
    }

} // namespace Hazel
