using System;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Hazel
{
    // ── 基类 ──────────────────────────────────────────────────
    public class UIWidgetComponent : Component
    {
        public bool Visible
        {
            get => InternalCalls.UIWidgetComponent_GetVisible(Entity.ID);
            set => InternalCalls.UIWidgetComponent_SetVisible(Entity.ID, value);
        }

        public Vector2 Position
        {
            get { InternalCalls.UIWidgetComponent_GetPosition(Entity.ID, out Vector2 v); return v; }
            set => InternalCalls.UIWidgetComponent_SetPosition(Entity.ID, ref value);
        }

        public Vector2 Size
        {
            get { InternalCalls.UIWidgetComponent_GetSize(Entity.ID, out Vector2 v); return v; }
            set => InternalCalls.UIWidgetComponent_SetSize(Entity.ID, ref value);
        }
    }

    // ── UIImage ───────────────────────────────────────────────
    public class UIImageComponent : Component
    {
        public Vector4 Color
        {
            get { InternalCalls.UIImageComponent_GetColor(Entity.ID, out Vector4 c); return c; }
            set => InternalCalls.UIImageComponent_SetColor(Entity.ID, ref value);
        }
    }

    // ── UIText ────────────────────────────────────────────────
    public class UITextComponent : Component
    {
        public string Text
        {
            get => InternalCalls.UITextComponent_GetText(Entity.ID);
            set => InternalCalls.UITextComponent_SetText(Entity.ID, value);
        }

        public Vector4 Color
        {
            get { InternalCalls.UITextComponent_GetColor(Entity.ID, out Vector4 c); return c; }
            set => InternalCalls.UITextComponent_SetColor(Entity.ID, ref value);
        }
    }

    // ── UIProgressBar ─────────────────────────────────────────
    public class UIProgressBarComponent : Component
    {
        public float Value
        {
            get => InternalCalls.UIProgressBarComponent_GetValue(Entity.ID);
            set => InternalCalls.UIProgressBarComponent_SetValue(Entity.ID, value);
        }

        public float MaxValue
        {
            get => InternalCalls.UIProgressBarComponent_GetMaxValue(Entity.ID);
            set => InternalCalls.UIProgressBarComponent_SetMaxValue(Entity.ID, value);
        }

        // 0~1的归一化值，只读
        public float Normalized => MaxValue > 0 ? Math.Clamp(Value / MaxValue, 0f, 1f) : 0f;

        public Vector4 ForegroundColor
        {
            get { InternalCalls.UIProgressBarComponent_GetForegroundColor(Entity.ID, out Vector4 c); return c; }
            set => InternalCalls.UIProgressBarComponent_SetForegroundColor(Entity.ID, ref value);
        }
    }

    // ── UIButton ──────────────────────────────────────────────
    public class UIButtonComponent : Component
    {
        public bool IsHovered => InternalCalls.UIButtonComponent_GetIsHovered(Entity.ID);
        public bool IsPressed => InternalCalls.UIButtonComponent_GetIsPressed(Entity.ID);

        // 设置点击回调的方法名（在同一Entity的脚本上）
        public string OnClickFunction
        {
            get => InternalCalls.UIButtonComponent_GetOnClickFunction(Entity.ID);
            set => InternalCalls.UIButtonComponent_SetOnClickFunction(Entity.ID, value);
        }
    }
}