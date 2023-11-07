using Editor.Components;
using Editor.EngineAPIStructs;
using Editor.GameProject;
using Editor.Utility;
using System;
using System.Linq;
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
    class ScriptComponent
    {
        public IntPtr ScriptCreateFunc;
    }

    [StructLayout(LayoutKind.Sequential)]
    class GameEntityDescriptor
    {
        public TransformComponent Transform = new TransformComponent();
        public ScriptComponent Script = new ScriptComponent();
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

        [DllImport(_engineDllName)]
        public static extern IntPtr GetGameScriptCreateFunc(string name);

        [DllImport(_engineDllName)]
        [return: MarshalAs(UnmanagedType.SafeArray)]
        public static extern string[] GetGameScriptNames(); // Rename

        [DllImport(_engineDllName)]
        public static extern int CreateRenderSurface(IntPtr host, int width, int height);

        [DllImport(_engineDllName)]
        public static extern void RemoveRenderSurface(int surfaceId);

        [DllImport(_engineDllName)]
        public static extern IntPtr GetWindowHandle(int surfaceId);

        [DllImport(_engineDllName)]
        public static extern void ResizeRenderSurface(int surfaceId);

        internal static class EntityAPI
        {

            [DllImport(_engineDllName)]
            private static extern int CreateGameEntity(GameEntityDescriptor desc);
            public static int CreateGameEntity(GameEntity entity)
            {
                GameEntityDescriptor desc = new GameEntityDescriptor();


                // Transform Component
                {
                    var c = entity.GetComponent<Transform>();
                    desc.Transform.Position = c.Position;
                    desc.Transform.Rotation = c.Rotation;
                    desc.Transform.Scale = c.Scale;
                }

                // Script Component
                {
                    var c = entity.GetComponent<Script>();
                    // プロジェクトがまだロードされてない場合はゲームコードのDLLがロードされる時に生成
                    if (c != null && Project.Current != null)
                    {
                        if (Project.Current.AvailableScripts.Contains(c.Name))
                        {
                            desc.Script.ScriptCreateFunc = GetGameScriptCreateFunc(c.Name);
                        }
                        else
                        {
                            Logger.Log(Verbosity.Error, $"{c.Name}スクリプトが見つかりませんでした。スクリプトコンポーネントなしで生成されます");
                        }
                    }
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
