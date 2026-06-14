using System;
using System.Collections.Generic;

namespace Hazel
{
    public class Entity
    {
        public readonly ulong ID;

        // 缓存已经获取过的组件，避免每帧 new 产生 GC
        private Dictionary<Type, Component> _componentCache = new Dictionary<Type, Component>();

        protected Entity() { ID = 0; }
        internal Entity(ulong id) { ID = id; }

        // 像 Unity 一样直接访问 Transform，免去每次 GetComponent 调用
        public TransformComponent Transform => GetComponent<TransformComponent>();

        public string Tag => InternalCalls.Entity_GetTag(ID);

        // ── 组件操作 ──────────────────────────────────────────

        /// <summary>
        /// 获取组件。带缓存，每个类型只会创建一次 C# 实例。
        /// </summary>
        public T GetComponent<T>() where T : Component, new()
        {
            Type type = typeof(T);

            if (_componentCache.TryGetValue(type, out var cached))
                return cached as T;

            if (!InternalCalls.Entity_HasComponent(ID, type))
                return null;

            T comp = new T();
            comp.Entity = this;
            _componentCache[type] = comp;
            return comp;
        }

        public bool HasComponent<T>() where T : Component
        {
            return InternalCalls.Entity_HasComponent(ID, typeof(T));
        }

        /// <summary>
        /// 运行时添加组件。若已存在则直接返回缓存的实例。
        /// </summary>
        public T AddComponent<T>() where T : Component, new()
        {
            Type type = typeof(T);

            if (_componentCache.TryGetValue(type, out var cached))
                return cached as T;

            if (HasComponent<T>())
                return GetComponent<T>();

            InternalCalls.Entity_AddComponent(ID, type);

            T comp = new T();
            comp.Entity = this;
            _componentCache[type] = comp;
            return comp;
        }

        // ── 场景查找（类似 Unity 的 FindObjectOfType / GameObject.Find）──

        /// <summary>
        /// 按名字在场景中找到第一个匹配的 Entity，并以指定脚本类型返回。
        /// 用法：var player = FindEntityByName&lt;PlayerController&gt;("Player");
        /// </summary>
        public static T FindEntityByName<T>(string name) where T : Entity
        {
            ulong id = InternalCalls.Scene_FindEntityByName(name);
            if (id == 0) return null;

            object instance = InternalCalls.Entity_GetScriptInstance(id);
            return instance as T;
        }

        /// <summary>
        /// 按名字找 Entity，不关心脚本类型，返回裸 Entity。
        /// 用法：Entity e = FindEntityByName("Bullet");
        /// </summary>
        public static Entity FindEntityByName(string name)
        {
            ulong id = InternalCalls.Scene_FindEntityByName(name);
            if (id == 0) return null;
            return new Entity(id);
        }

        // ── 跨脚本通信 ────────────────────────────────────────

        /// <summary>
        /// 获取另一个 Entity 上挂载的脚本实例（强转到指定类型）。
        /// 用法：var enemy = someEntity.GetScript&lt;EnemyController&gt;();
        /// </summary>
        public T GetScript<T>() where T : Entity
        {
            object instance = InternalCalls.Entity_GetScriptInstance(ID);
            return instance as T;
        }

        // ── 生命周期虚方法 ────────────────────────────────────
        public virtual void OnCreate() { }
        public virtual void OnUpdate(float ts) { }

        // 碰撞回调（子类重写这两个）
        public virtual void OnCollisionEnter(Entity other) { }
        public virtual void OnCollisionExit(Entity other) { }

        // C++ 用 ulong 重载调用，避免与上面的 Entity 重载混淆
        internal void OnCollisionEnter(ulong otherID) => OnCollisionEnter(new Entity(otherID));
        internal void OnCollisionExit(ulong otherID) => OnCollisionExit(new Entity(otherID));
    }
}
