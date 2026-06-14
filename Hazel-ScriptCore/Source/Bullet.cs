using Hazel;
using System;

namespace Sandbox
{
    public class Bullet : Entity
    {
        public float Speed = 10.0f;
        public float Damage = 25.0f;
        public float Direction = 1.0f;

        // 在编辑器里设置音效路径，或者直接硬编码
        public string ShootSoundPath = "D:/programs/Visual Studio/Game Engine/Hazel2/Hazelnut/assets/audios/bullet.mp3";
        public float ShootVolume = 1.0f;

        private Rigidbody2DComponent m_Rigidbody;
        private SpriteRendererComponent m_Sprite;
        private bool m_Initialized = false;

        public override void OnCreate()
        {
            Speed = 10.0f;
            Damage = 25.0f;
            Direction = 1.0f;
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
            m_Sprite = GetComponent<SpriteRendererComponent>();
            if (m_Rigidbody != null)
            {
                m_Rigidbody.Type = Rigidbody2DComponent.BodyType.Dynamic;
                m_Rigidbody.GravityScale = 0.0f;
                // 不在这里设速度
            }
            m_Initialized = false;

            ShootSoundPath = "D:/programs/Visual Studio/Game Engine/Hazel2/Hazelnut/assets/audios/bullet.mp3";
            ShootVolume = 1.0f;
            // 子弹生成时播放音效
            // 方案A：直接用静态 Audio 类（最简单）
            if (!string.IsNullOrEmpty(ShootSoundPath))
                Audio.PlaySound(ShootSoundPath, ShootVolume);

            // 方案B：如果 Bullet 身上挂了 AudioSourceComponent
            // var audio = GetComponent<AudioSourceComponent>();
            // audio?.Play();
        }

        public override void OnUpdate(float ts)
        {
            if (!m_Initialized && m_Rigidbody != null)
            {
                m_Rigidbody.LinearVelocity = new Vector2(Speed * Direction, 0.0f);

                // 同步翻转贴图
                if (m_Sprite != null)
                    m_Sprite.FlipX = Direction < 0.0f;

                m_Initialized = true;
            }
        }

        public override void OnCollisionEnter(Entity other)
        {
            if (other.Tag == "Player") return;
            if (other.Tag == "Enemy")
            {
                Enemy enemy = other.GetScript<Enemy>();
                if (enemy != null)
                    enemy.TakeDamage(Damage);
            }
            InternalCalls.Entity_Destroy(ID);
        }
    }
}