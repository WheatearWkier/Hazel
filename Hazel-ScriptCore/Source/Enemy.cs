using Hazel;
using System;

namespace Sandbox
{
    public class Enemy : Entity
    {
        public float MaxHp = 100.0f;
        public float Hp = 100.0f;

        private UIProgressBarComponent _healthBar;
        private SpriteAnimatorComponent m_Animator;
        private bool m_IsDying = false;

        public override void OnCreate()
        {
            MaxHp = 100.0f;
            Hp = MaxHp;
            m_IsDying = false;

            // 1. 寻找名为 "HealthBar" 的实体
            // 注意：如果场景里有多个敌人，这种方式只能找到场景中第一个叫这个名字的血条。
            // 建议测试时血条名字保持唯一。
            Entity barEntity = FindEntityByName("HealthBar");

            if (barEntity != null)
            {
                _healthBar = barEntity.GetComponent<UIProgressBarComponent>();
                if (_healthBar != null)
                {
                    _healthBar.MaxValue = MaxHp;
                    _healthBar.Value = Hp;
                }
            }
            else
            {
                Console.WriteLine("Warning: Could not find HealthBar entity!");
            }

            m_Animator = GetComponent<SpriteAnimatorComponent>();
        }

        public override void OnUpdate(float ts)
        {
            if (m_IsDying && m_Animator.IsFinished)
            {
                InternalCalls.Entity_Destroy(ID);
            }
        }

        public void TakeDamage(float amount)
        {
            if (m_IsDying) return;  // 已经在死亡动画中，忽略后续伤害

            Hp -= amount;
            if (_healthBar != null)
                _healthBar.Value = Hp;

            Console.WriteLine($"Enemy hit! HP: {Hp}/{MaxHp}");

            if (Hp <= 0.0f)
            {
                Console.WriteLine("Enemy died!");
                m_IsDying = true;
                m_Animator.Play("boom");
            }
        }
    }
}