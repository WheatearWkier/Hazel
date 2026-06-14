namespace Hazel
{
    // 所有组件的基类
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    // ── TransformComponent ────────────────────────────────────

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get { InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 v); return v; }
            set => InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
        }

        public Vector3 Rotation
        {
            get { InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 v); return v; }
            set => InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
        }

        public Vector3 Scale
        {
            get { InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 v); return v; }
            set => InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
        }
    }

    // ── Rigidbody2DComponent ──────────────────────────────────

    public class Rigidbody2DComponent : Component
    {
        public enum BodyType { Static = 0, Dynamic, Kinematic }

        /// <summary> 刚体类型（Static / Dynamic / Kinematic） </summary>
        public BodyType Type
        {
            get => (BodyType)InternalCalls.Rigidbody2DComponent_GetBodyType(Entity.ID);
            set => InternalCalls.Rigidbody2DComponent_SetBodyType(Entity.ID, (int)value);
        }

        /// <summary>
        /// 重力缩放比例。设为 0 完全不受重力，设为 1 为正常重力，也可设负数（反重力）。
        /// 等价于 Unity 的 Rigidbody2D.gravityScale
        /// </summary>
        public float GravityScale
        {
            get => InternalCalls.Rigidbody2DComponent_GetGravityScale(Entity.ID);
            set => InternalCalls.Rigidbody2DComponent_SetGravityScale(Entity.ID, value);
        }

        /// <summary> 锁定旋转。等价于 Unity Rigidbody2D 的 constraints FreezeRotation </summary>
        public bool FixedRotation
        {
            get => InternalCalls.Rigidbody2DComponent_GetFixedRotation(Entity.ID);
            set => InternalCalls.Rigidbody2DComponent_SetFixedRotation(Entity.ID, value);
        }

        public Vector2 LinearVelocity
        {
            get { InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 v); return v; }
            set => InternalCalls.Rigidbody2DComponent_SetLinearVelocity(Entity.ID, ref value);
        }

        public void ApplyLinearImpulse(Vector2 impulse, Vector2 point, bool wake = true)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref point, wake);
        }

        /// <summary> 从质心施加冲量（最常用的跳跃/推力写法） </summary>
        public void ApplyLinearImpulseToCenter(Vector2 impulse, bool wake = true)
        {
            // 用当前线速度查询点作为 world center 近似（Box2D 内部会 clamp）
            Vector2 zero = Vector2.Zero;
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref zero, wake);
        }
    }

    // ── SpriteRendererComponent ───────────────────────────────

    public class SpriteRendererComponent : Component
    {
        public bool FlipX
        {
            get => InternalCalls.SpriteRendererComponent_GetFlipX(Entity.ID);
            set => InternalCalls.SpriteRendererComponent_SetFlipX(Entity.ID, value);
        }
        // 颜色的 get/set 如果需要可后续通过 InternalCall 扩展
        // 目前仅作为 HasComponent / AddComponent 的标记类型使用
    }

    // ── CircleRendererComponent ───────────────────────────────

    public class CircleRendererComponent : Component { }

    // ── SpriteAnimatorComponent ───────────────────────────────

    public class SpriteAnimatorComponent : Component
    {
        /// <summary> 切换并播放指定动画，名字相同时不会重置 </summary>
        public void Play(string clipName)
            => InternalCalls.SpriteAnimatorComponent_Play(Entity.ID, clipName);

        /// <summary> 暂停当前动画（保持在当前帧） </summary>
        public void Stop()
            => InternalCalls.SpriteAnimatorComponent_Stop(Entity.ID);

        /// <summary> 继续播放 </summary>
        public void Resume()
            => InternalCalls.SpriteAnimatorComponent_Resume(Entity.ID);

        /// <summary> 当前正在播放的 clip 名字 </summary>
        public string CurrentClip
            => InternalCalls.SpriteAnimatorComponent_GetCurrentClip(Entity.ID);

        /// <summary> 非循环动画播放完毕后为 true </summary>
        public bool IsFinished
            => InternalCalls.SpriteAnimatorComponent_IsFinished(Entity.ID);
    }

    // ── Collider 组件（标记类型，可后续扩展属性）────────────

    public class BoxCollider2DComponent : Component { }
    public class CircleCollider2DComponent : Component { }
    public class CameraComponent : Component { }

    // ── UI 组件（标记类型，可后续扩展属性）────────────
    public class UICanvasComponent : Component
    {
        public bool Visible
        {
            get => InternalCalls.UIWidgetComponent_GetVisible(Entity.ID);
            set => InternalCalls.UIWidgetComponent_SetVisible(Entity.ID, value);
        }
    }

    // ── AudioSourceComponent ──────────────────────────────
    public class AudioSourceComponent : Component
    {
        /// <summary> 播放挂载在这个组件上的音频 </summary>
        public void Play()
        {
            InternalCalls.AudioSourceComponent_Play(Entity.ID);
        }

        public void Stop()
        {
            InternalCalls.AudioSourceComponent_Stop(Entity.ID);
        }

        public bool IsPlaying
        {
            get => InternalCalls.AudioSourceComponent_IsPlaying(Entity.ID);
        }

        public float Volume
        {
            get => InternalCalls.AudioSourceComponent_GetVolume(Entity.ID);
            set => InternalCalls.AudioSourceComponent_SetVolume(Entity.ID, value);
        }
    }
}