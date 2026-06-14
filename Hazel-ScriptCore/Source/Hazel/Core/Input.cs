using System;

namespace Hazel
{
    public static class Input
    {
        public static bool IsKeyDown(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyDown((int)keycode);
        }
    }
}
