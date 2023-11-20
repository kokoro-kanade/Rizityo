using Editor.ToolAPIStructs;
using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Editor.ToolAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class GeometryImportSetting
    {
        public float SmoothingAngle = 178f;
        public byte CalculateNormals = 0;
        public byte CalculateTangents = 1;
        public byte ReverseHandedness = 0;
        public byte ImportEmbededTextures = 1;
        public byte ImportAnimations = 1;

        private byte ToByte(bool value) => value ? (byte)1 : (byte)0;

        public void FromContentSetting(Content.GeometryImportSetting setting)
        {
            SmoothingAngle = setting.SmoothingAngle;
            CalculateNormals = ToByte(setting.CalculateNormals);
            CalculateTangents = ToByte(setting.CalculateTangents);
            ReverseHandedness = ToByte(setting.ReverseHandedness);
            ImportEmbededTextures = ToByte(setting.ImportEmbeddedTextures);
            ImportAnimations = ToByte(setting.ImportAnimations);
        }
    }


    [StructLayout(LayoutKind.Sequential)]
    class LevelData : IDisposable
    {
        public IntPtr Data;
        public int DataSize;
        public GeometryImportSetting ImportSetting = new GeometryImportSetting();

        public void Dispose()
        {
            Marshal.FreeCoTaskMem(Data);
            GC.SuppressFinalize(this);
        }

        ~LevelData()
        {
            Dispose();
        }
    }


    [StructLayout(LayoutKind.Sequential)]
    class PrimitiveInitInfo
    {
        public Content.PrimitiveMeshType Type;
        public int SegmentX = 1;
        public int SegmentY = 1;
        public int SegmentZ = 1;
        public Vector3 Size = new Vector3(1f);
        public int Lod = 0;
    }
}
    
namespace Editor.DLLWrapper
{

    static class AssetToosAPI
    {
        private const string _toolDLL = "AssetTool.dll";

        private static void GeometryFromLevelData(Content.Geometry geometry, Action<LevelData> levelDataGenerator, string failureMsg)
        {
            Debug.Assert(geometry != null);
            using var levelData = new LevelData();
            try
            {
                levelData.ImportSetting.FromContentSetting(geometry.ImportSetting);
                levelDataGenerator(levelData);
                Debug.Assert(levelData.Data != IntPtr.Zero && levelData.DataSize > 0);
                var data = new byte[levelData.DataSize];
                Marshal.Copy(levelData.Data, data, 0, levelData.DataSize);
                geometry.FromRawData(data);
            }
            catch (Exception ex)
            {
                Logger.Log(Verbosity.Error, failureMsg);
                Debug.WriteLine(ex.Message);
            }
        }

        [DllImport(_toolDLL)]
        private static extern void CreatePrimitiveMesh([In, Out] LevelData data, PrimitiveInitInfo info);
        public static void CreatePrimitiveMesh(Content.Geometry geometry, PrimitiveInitInfo info)
        {
            GeometryFromLevelData(geometry, (levelData) => CreatePrimitiveMesh(levelData, info), $"{info.Type}メッシュの作成に失敗しました");
        }

        [DllImport(_toolDLL)]
        private static extern void ImportFBX(string filePath, [In, Out] LevelData data);
        public static void ImportFBX(string filePath, Content.Geometry geometry)
        {
            GeometryFromLevelData(geometry, (levelData) => ImportFBX(filePath, levelData), $"FBXファイル{filePath}のインポートに失敗しました");
        }
    }
}
