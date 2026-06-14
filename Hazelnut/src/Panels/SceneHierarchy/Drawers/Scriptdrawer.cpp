#include "ScriptDrawer.h"

#include "../ComponentDrawers.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

#include "Hazel/Scene/Components.h"
#include "Hazel/Scripting/ScriptEngine.h"

namespace Hazel {

    void DrawScriptComponent(Entity entity)
    {
        DrawComponent<ScriptComponent>("Script", entity, [entity](auto& component)
            {
                const bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

                // BUG FIX: 原来用 static char buffer，所有 ScriptComponent 共享同一块内存，
                // 导致一帧内 strcpy 覆盖输入内容，打一个字就失焦。
                // 改为局部 buffer，ImGui 用 entity handle 作为 ID 隔离。
                char buffer[64];
                memset(buffer, 0, sizeof(buffer));
                strncpy_s(buffer, sizeof(buffer), component.ClassName.c_str(), _TRUNCATE);

                if (!scriptClassExists)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));

                // 用 PushID 确保每个实体的输入框 ID 独立，避免焦点跳跃
                ImGui::PushID((int)(uint32_t)entity);
                if (ImGui::InputText("Class", buffer, sizeof(buffer)))
                    component.ClassName = buffer;
                ImGui::PopID();

                if (!scriptClassExists)
                {
                    ImGui::PopStyleColor();
                    ImGui::TextDisabled("  (class not found)");
                    return;
                }

                // ──────────────────────────────────────────────────────────────
                //  字段显示：编辑态从 ScriptClass 读取字段类型并使用
                //  ScriptEngine::GetScriptFieldMap 存储的编辑态数值，
                //  运行态则从 ScriptInstance 读写
                // ──────────────────────────────────────────────────────────────
                Ref<ScriptClass> scriptClass = ScriptEngine::GetEntityClass(component.ClassName);
                if (!scriptClass) return;

                const auto& fields = scriptClass->GetFields();
                if (fields.empty()) return;

                ImGui::Spacing();
                ImGui::TextDisabled("Fields");
                ImGui::Separator();

                Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());

                // 运行态: 直接读写实例
                // 编辑态: 读写 ScriptFieldMap (需要引擎侧支持 GetScriptFieldMap)
                auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);

                for (auto& [name, field] : fields)
                {
                    if (scriptInstance)
                    {
                        // 运行态
                        if (field.Type == ScriptFieldType::Float)
                        {
                            float value = scriptInstance->GetFieldValue<float>(name);
                            if (ImGui::DragFloat(name.c_str(), &value))
                                scriptInstance->SetFieldValue(name, value);
                        }
                        else if (field.Type == ScriptFieldType::Int)
                        {
                            int value = scriptInstance->GetFieldValue<int>(name);
                            if (ImGui::DragInt(name.c_str(), &value))
                                scriptInstance->SetFieldValue(name, value);
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            bool value = scriptInstance->GetFieldValue<bool>(name);
                            if (ImGui::Checkbox(name.c_str(), &value))
                                scriptInstance->SetFieldValue(name, value);
                        }
                        else if (field.Type == ScriptFieldType::Vector3)
                        {
                            glm::vec3 value = scriptInstance->GetFieldValue<glm::vec3>(name);
                            if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(value)))
                                scriptInstance->SetFieldValue(name, value);
                        }
                    }
                    else
                    {
                        // 编辑态：从 ScriptFieldMap 读写，没有则用默认值
                        ScriptFieldInstance& fieldInst = entityFields[name];
                        fieldInst.Field = field;  // 确保 Field 元信息同步

                        if (field.Type == ScriptFieldType::Float)
                        {
                            float value = fieldInst.GetValue<float>();
                            if (ImGui::DragFloat(name.c_str(), &value))
                                fieldInst.SetValue(value);
                        }
                        else if (field.Type == ScriptFieldType::Int)
                        {
                            int value = fieldInst.GetValue<int>();
                            if (ImGui::DragInt(name.c_str(), &value))
                                fieldInst.SetValue(value);
                        }
                        else if (field.Type == ScriptFieldType::Bool)
                        {
                            bool value = fieldInst.GetValue<bool>();
                            if (ImGui::Checkbox(name.c_str(), &value))
                                fieldInst.SetValue(value);
                        }
                        else if (field.Type == ScriptFieldType::Vector3)
                        {
                            glm::vec3 value = fieldInst.GetValue<glm::vec3>();
                            if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(value)))
                                fieldInst.SetValue(value);
                        }
                    }
                }
            });
    }

} // namespace Hazel
