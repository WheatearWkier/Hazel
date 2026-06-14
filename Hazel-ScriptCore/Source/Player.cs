using Hazel;
using System;

namespace Sandbox
{
    public class Player : Entity
    {
        // 配置项
        public float Speed = 6.0f;
        public float JumpForce = 6.0f;
        // 引用缓存
        private Rigidbody2DComponent m_Rigidbody;
        private SpriteAnimatorComponent m_Animator;
        private SpriteRendererComponent m_Sprite;
        // 输入缓存
        private bool m_LastJumpDown = false;
        private bool m_LastShootDown = false;
        private float m_MoveInput;
        public override void OnCreate()
        {
            Speed = 6.0f;
            JumpForce = 6.0f;
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
            m_Animator = GetComponent<SpriteAnimatorComponent>();
            m_Sprite = GetComponent<SpriteRendererComponent>();
            if (m_Animator != null)
                m_Animator.Play("idle");
        }
        public override void OnUpdate(float ts)
        {
            if (m_Rigidbody == null) return;
            // ===== 输入处理 =====
            m_MoveInput = 0.0f;
            if (Input.IsKeyDown(KeyCode.A)) m_MoveInput -= 1.0f;
            if (Input.IsKeyDown(KeyCode.D)) m_MoveInput += 1.0f;
            bool jumpDown = Input.IsKeyDown(KeyCode.W);
            bool jumpPressed = jumpDown && !m_LastJumpDown;
            m_LastJumpDown = jumpDown;
            // ===== 物理逻辑 =====
            Vector2 velocity = m_Rigidbody.LinearVelocity;
            velocity.X = m_MoveInput * Speed;
            bool isGrounded = Math.Abs(velocity.Y) < 0.01f;
            if (jumpPressed && isGrounded)
                velocity.Y = JumpForce;
            if (velocity.Y < 0)
                velocity.Y += -10.0f * ts;
            m_Rigidbody.LinearVelocity = velocity;
            // ===== 动画更新 =====
            UpdateAnimation(velocity, isGrounded);
            // ===== 射击逻辑 =====
            bool shootDown = Input.IsKeyDown(KeyCode.Space);
            if (shootDown && !m_LastShootDown)
                Shoot();
            m_LastShootDown = shootDown;
        }
        private void UpdateAnimation(Vector2 velocity, bool isGrounded)
        {
            if (m_Animator == null) return;

            // 翻转方向 ← 加这段
            if (m_Sprite != null)
            {
                if (m_MoveInput > 0.0f) m_Sprite.FlipX = false; // 朝右
                else if (m_MoveInput < 0.0f) m_Sprite.FlipX = true;  // 朝左
                // m_MoveInput == 0 时保持上一帧方向，不重置
            }

            if (!isGrounded)
                m_Animator.Play(velocity.Y > 0 ? "jump" : "fall");
            else if (Math.Abs(m_MoveInput) > 0.01f)
                m_Animator.Play("run");
            else
                m_Animator.Play("idle");
        }
        private void Shoot()
        {
            Vector3 spawnPos = Transform.Translation;
            float dir = (m_Sprite != null && m_Sprite.FlipX) ? -1.0f : 1.0f;
            spawnPos.X += 0.5f * dir;

            ulong bulletID = InternalCalls.Scene_InstantiateFromPrefab(
                "D:/programs/Visual Studio/Game Engine/Hazel2/Hazelnut/assets/prefabs/Bullet.hzprefab",
                ref spawnPos
            );

            if (bulletID != 0)
            {
                // 找到刚生成的子弹 Entity，拿到它的脚本，设置方向
                Entity bulletEntity = new Entity(bulletID);
                Bullet bullet = bulletEntity.GetScript<Bullet>();
                if (bullet != null)
                {
                    bullet.Direction = dir;
                    // 如果想在 Player 端控制音效，可以在这里覆盖路径
                    // bullet.ShootSoundPath = "assets/sounds/player_shoot.wav";
                }
            }
        }
    }
}