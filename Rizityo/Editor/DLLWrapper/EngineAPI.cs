using Editor.Components;
using Editor.EngineAPIStructs;
using System.Numerics;
using System.Runtime.InteropServices;

namespace Editor.EngineAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class TransformComponent
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale = new Vector3(1,1,1);
    }

    [StructLayout(LayoutKind.Sequential)]
    class GameEntityDescriptor
    {
        public TransformComponent Transform = new TransformComponent();
    }
}

namespace Editor.DLLWrapper // Rename: EngineWrapper?
{
    static class EngineAPI
    {
        private const string _engineDllName = "EngineDLL.dll";

        [DllImport(_engineDllName, CharSet = CharSet.Ansi)]
        public static extern int LoadGameCodeDll(string dllPath);

        [DllImport(_engineDllName, CharSet = CharSet.Ansi)]
        public static extern int UnLoadGameCodeDll();

        internal static class EntityAPI
        {

            [DllImport(_engineDllName)]
            private static extern int CreateGameEntity(GameEntityDescriptor desc);
            public static int CreateGameEntity(GameEntity entity)
            {
                GameEntityDescriptor desc = new GameEntityDescriptor();

                {
                    var c = entity.GetComponent<Components.Transform>();
                    desc.Transform.Position = c.Position;
                    desc.Transform.Rotation = c.Rotation;
                    desc.Transform.Scale = c.Scale;
                }

                return CreateGameEntity(desc);
            }

            [DllImport(_engineDllName)]
            private static extern void RemoveGameEntity(int id);
            public static void RemoveGameEntity(GameEntity entity)
            {
                RemoveGameEntity(entity.EntityId);
            }
        }
    }
}
