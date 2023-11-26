using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Editor.Common;
using Editor.DLLWrapper;
using Editor.GameProject;
using Editor.Utility;

namespace Editor.Content
{
    enum PrimitiveMeshType
    {
        Plane,
        Cube,
        UVSphere,
        IcoSphere,
        Cylinder,
        Capsule,
        Count
    }

    enum ElementsType
    {
        Position = 0x00,
        Normals = 0x01,
        TSpace = 0x03,
        Joints = 0x04,
        Colors = 0x08
    }

    enum PrimitveTopology
    {
        PointList = 1,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
    }

    class Mesh : ViewModelBase
    {
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        public static int PositionSize = sizeof(float) * 3;

        private int _elementSize;
        public int ElementSize
        {
            get => _elementSize;
            set
            {
                if (_elementSize != value)
                {
                    _elementSize = value;
                    OnPropertyChanged(nameof(ElementSize));
                }
            }
        }

        private int _vertexCount;
        public int VertexCount
        {
            get => _vertexCount;
            set
            {
                if (_vertexCount != value)
                {
                    _vertexCount = value;
                    OnPropertyChanged(nameof(VertexCount));
                }
            }
        }

        private int _indexSize;
        public int IndexSize
        {
            get => _indexSize;
            set
            {
                if (_indexSize != value)
                {
                    _indexSize = value;
                    OnPropertyChanged(nameof(IndexSize));
                }
            }
        }

        private int _indexCount;
        public int IndexCount
        {
            get => _indexCount;
            set
            {
                if (_indexCount != value)
                {
                    _indexCount = value;
                    OnPropertyChanged(nameof(IndexCount));
                }
            }
        }
        public ElementsType ElementsType { get; set; }
        public PrimitveTopology PrimitveTopology { get; set; }
        public byte[] Positions { get; set; }
        public byte[] Elements { get; set; }
        public byte[] Indices { get; set; }
    }

    class MeshLOD : ViewModelBase
    {

        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        private float _lodThreshold;
        public float LODThreshold

        {
            get => _lodThreshold;
            set
            {
                if (_lodThreshold != value)
                {
                    _lodThreshold = value;
                    OnPropertyChanged(nameof(LODThreshold));
                }
            }
        }

        public ObservableCollection<Mesh> Meshes { get; } = new ObservableCollection<Mesh>();
    }

    class LODGroup : ViewModelBase
    {

        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        public ObservableCollection<MeshLOD> LODs { get; } = new ObservableCollection<MeshLOD>();
    }

    class GeometryImportSetting : ViewModelBase
    {
        private float _smoothingAngle;
        public float SmoothingAngle
        {
            get => _smoothingAngle;
            set
            {
                if (_smoothingAngle != value)
                {
                    _smoothingAngle = value;
                    OnPropertyChanged(nameof(SmoothingAngle));
                }
            }
        }

        private bool _calculateNormals;
        public bool CalculateNormals
        {
            get => _calculateNormals;
            set
            {
                if (_calculateNormals != value)
                {
                    _calculateNormals = value;
                    OnPropertyChanged(nameof(CalculateNormals));
                }
            }
        }

        private bool _calculateTangents;
        public bool CalculateTangents
        {
            get => _calculateTangents;
            set
            {
                if (_calculateTangents != value)
                {
                    _calculateTangents = value;
                    OnPropertyChanged(nameof(CalculateTangents));
                }
            }
        }

        private bool _reverseHandedness;
        public bool ReverseHandedness
        {
            get => _reverseHandedness;
            set
            {
                if (_reverseHandedness != value)
                {
                    _reverseHandedness = value;
                    OnPropertyChanged(nameof(ReverseHandedness));
                }
            }
        }

        private bool _importEmbeddedTextures;
        public bool ImportEmbeddedTextures
        {
            get => _importEmbeddedTextures;
            set
            {
                if (_importEmbeddedTextures != value)
                {
                    _importEmbeddedTextures = value;
                    OnPropertyChanged(nameof(ImportEmbeddedTextures));
                }
            }
        }

        private bool _importAnimations;
        public bool ImportAnimations
        {
            get => _importAnimations;
            set
            {
                if (_importAnimations != value)
                {
                    _importAnimations = value;
                    OnPropertyChanged(nameof(ImportAnimations));
                }
            }
        }

        public GeometryImportSetting()
        {
            SmoothingAngle = 178f;
            CalculateNormals = false;
            CalculateTangents = false;
            ReverseHandedness = false;
            ImportEmbeddedTextures = true;
            ImportAnimations = true;
        }

        public void ToBinary(BinaryWriter writer)
        {
            writer.Write(SmoothingAngle);
            writer.Write(CalculateNormals);
            writer.Write(CalculateTangents);
            writer.Write(ReverseHandedness);
            writer.Write(ImportEmbeddedTextures);
            writer.Write(ImportAnimations);
        }

        public void FromBinary(BinaryReader reader)
        {
            SmoothingAngle = reader.ReadSingle();
            CalculateNormals = reader.ReadBoolean();
            CalculateTangents = reader.ReadBoolean();
            ReverseHandedness = reader.ReadBoolean();
            ImportEmbeddedTextures = reader.ReadBoolean();
            ImportAnimations = reader.ReadBoolean();
        }
    }

    class Geometry : Asset
    {
        private readonly object _lock = new object();

        private readonly List<LODGroup> _lodGroups = new List<LODGroup>();

        public GeometryImportSetting ImportSetting { get; } = new GeometryImportSetting();

        public LODGroup GetLODGroup(int lodGroup = 0)
        {
            Debug.Assert(lodGroup >= 0 && lodGroup < _lodGroups.Count());
            return (lodGroup < _lodGroups.Count) ? _lodGroups[lodGroup] : null;
        }

        private static void ReadMesh(BinaryReader reader, List<int> lodIds, List<MeshLOD> lodList)
        {
            // メッシュ名
            var s = reader.ReadInt32();
            string meshName;
            if (s > 0)
            {
                var nameBytes = reader.ReadBytes(s);
                meshName = Encoding.UTF8.GetString(nameBytes);
            }
            else
            {
                meshName = $"mesh_{ContentHelper.GetRandomString()}";
            }

            var mesh = new Mesh()
            {
                Name = meshName
            };

            var lodId = reader.ReadInt32();
            mesh.ElementSize = reader.ReadInt32();
            mesh.ElementsType = (ElementsType)reader.ReadInt32();
            mesh.PrimitveTopology = PrimitveTopology.TriangleList; // 今のところAssetToolがサポートするのはTriangle Listのみ
            mesh.VertexCount = reader.ReadInt32();
            mesh.IndexSize = reader.ReadInt32();
            mesh.IndexCount = reader.ReadInt32();
            var lodThreshold = reader.ReadSingle();

            // 頂点位置データ
            mesh.Positions = reader.ReadBytes(Mesh.PositionSize * mesh.VertexCount);

            // 頂点属性データ
            var elementBufferSize = mesh.ElementSize * mesh.VertexCount;
            mesh.Elements = reader.ReadBytes(elementBufferSize);

            // インデックスデータ
            var indexBufferSize = mesh.IndexSize * mesh.IndexCount;
            mesh.Indices = reader.ReadBytes(indexBufferSize);

            // 同じLOD IDのものをまとめる
            MeshLOD lod;
            if (ID.IsValid(lodId) && lodIds.Contains(lodId))
            {
                lod = lodList[lodIds.IndexOf(lodId)];
                Debug.Assert(lod != null);
            }
            else
            {
                lodIds.Add(lodId);
                lod = new MeshLOD()
                {
                    Name = meshName,
                    LODThreshold = lodThreshold
                };
                lodList.Add(lod);
            }

            lod.Meshes.Add(mesh);
        }

        private static List<MeshLOD> ReadMeshLods(int numMeshes, BinaryReader reader)
        {
            var lodIds = new List<int>();
            var lodList = new List<MeshLOD>();
            for (int i = 0; i < numMeshes; i++)
            {
                ReadMesh(reader, lodIds, lodList);
            }

            return lodList;
        }

        public void FromRawData(byte[] data)
        {
            Debug.Assert(data?.Length > 0);
            _lodGroups.Clear();

            using var reader = new BinaryReader(new MemoryStream(data));

            // レベル名は無視
            var s = reader.ReadInt32();
            reader.BaseStream.Position += s;

            // LODの数
            var numLodGroups = reader.ReadInt32();
            Debug.Assert(numLodGroups > 0);

            // LODデータ
            for (int i = 0; i < numLodGroups; i++)
            {
                // LODの名前
                s = reader.ReadInt32();
                string lodGroupName;
                if (s > 0)
                {
                    var nameBytes = reader.ReadBytes(s);
                    lodGroupName = Encoding.UTF8.GetString(nameBytes);
                }
                else
                {
                    lodGroupName = $"lod_{ContentHelper.GetRandomString()}";
                }

                // LOD内のメッシュの数
                var numMeshes = reader.ReadInt32();
                Debug.Assert(numMeshes > 0);

                // メッシュデータ
                List<MeshLOD> lods = ReadMeshLods(numMeshes, reader);

                var lodGroup = new LODGroup() { Name = lodGroupName };
                lods.ForEach(lod => lodGroup.LODs.Add(lod));

                _lodGroups.Add(lodGroup);
            }
        }

        private void LODToBinary(MeshLOD lod, BinaryWriter writer, out byte[] hash)
        {
            writer.Write(lod.Name);
            writer.Write(lod.LODThreshold);
            writer.Write(lod.Meshes.Count);

            var meshDataBegin = writer.BaseStream.Position;

            foreach (var mesh in lod.Meshes)
            {
                writer.Write(mesh.Name);
                writer.Write(mesh.ElementSize);
                writer.Write((int)mesh.ElementsType);
                writer.Write((int)mesh.PrimitveTopology);
                writer.Write(mesh.VertexCount);
                writer.Write(mesh.IndexSize);
                writer.Write(mesh.IndexCount);
                writer.Write(mesh.Positions);
                writer.Write(mesh.Elements);
                writer.Write(mesh.Indices);
            }

            var meshDataSize = writer.BaseStream.Position - meshDataBegin;
            Debug.Assert(meshDataSize > 0);

            var data = (writer.BaseStream as MemoryStream).ToArray();
            hash = ContentHelper.ComputeHash(data, (int)meshDataBegin, (int)meshDataSize);
        }

        private MeshLOD BinaryToLOD(BinaryReader reader)
        {
            var lod = new MeshLOD();
            lod.Name = reader.ReadString();
            lod.LODThreshold = reader.ReadSingle();
            var meshCount = reader.ReadInt32();

            for (int i = 0; i < meshCount; ++i)
            {
                var mesh = new Mesh()
                {
                    Name = reader.ReadString(),
                    ElementSize = reader.ReadInt32(),
                    ElementsType = (ElementsType)reader.ReadInt32(),
                    PrimitveTopology = (PrimitveTopology)reader.ReadInt32(),
                    VertexCount = reader.ReadInt32(),
                    IndexSize = reader.ReadInt32(),
                    IndexCount = reader.ReadInt32()
                };

                mesh.Positions = reader.ReadBytes(Mesh.PositionSize * mesh.VertexCount);
                mesh.Elements = reader.ReadBytes(mesh.ElementSize * mesh.VertexCount);
                mesh.Indices = reader.ReadBytes(mesh.IndexSize * mesh.IndexCount);

                lod.Meshes.Add(mesh);
            }

            return lod;
        }

        private byte[] GenerateIcon(MeshLOD meshLod)
        {
            var width = ContentInfo.IconWidth * 4;

            using var memoryStream = new MemoryStream();
            BitmapSource bmp = null;
            Application.Current.Dispatcher.Invoke(() =>
            {
                bmp = Editors.GeometryView.RenderToBitmap(new Editors.MeshRenderer(meshLod, null), width, width);
                bmp = new TransformedBitmap(bmp, new ScaleTransform(0.25, 0.25, 0.5, 0.5));
                memoryStream.SetLength(0);
                var encoder = new PngBitmapEncoder();
                encoder.Frames.Add(BitmapFrame.Create(bmp));
                encoder.Save(memoryStream);
            });

            return memoryStream.ToArray();
        }

        public override void Import(string filePath)
        {
            Debug.Assert(File.Exists(filePath));
            Debug.Assert(!string.IsNullOrEmpty(FullPath));

            var extension = Path.GetExtension(filePath).ToLower();

            SourcePath = filePath;

            try
            {
                if (extension == ".fbx")
                {
                    ImportFBX(filePath);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                var msg = $"{filePath}のインポート中に読み込みに失敗しました";
                Debug.WriteLine(msg);
                Logger.Log(Verbosity.Error, msg);
            }

        }

        private void ImportFBX(string filePath)
        {
            Logger.Log(Verbosity.Display, $"FBXファイル{filePath}を読み込み中です");
            var tmpFolderPath = Application.Current.Dispatcher.Invoke(() => Project.Current.TmpFolder);
            if (string.IsNullOrEmpty(tmpFolderPath))
                return;

            lock (_lock)
            {
                if (!Directory.Exists(tmpFolderPath))
                {
                    Directory.CreateDirectory(tmpFolderPath);
                }
            }

            var tmpFilePath = $"{tmpFolderPath}{ContentHelper.GetRandomString()}.fbx";
            File.Copy(filePath, tmpFilePath, true);
            AssetToosAPI.ImportFBX(tmpFilePath, this);
        }

        public override void Load(string filePath)
        {
            Debug.Assert(File.Exists(filePath));
            Debug.Assert(Path.GetExtension(filePath).ToLower() == AssetFileExtension);

            try
            {
                byte[] data = null;
                using (var reader = new BinaryReader(File.Open(filePath, FileMode.Open, FileAccess.Read)))
                {
                    ReadAssetFileHeader(reader);
                    ImportSetting.FromBinary(reader);
                    int dataLength = reader.ReadInt32();
                    Debug.Assert(dataLength > 0);
                    data = reader.ReadBytes(dataLength);
                }

                Debug.Assert(data.Length > 0);

                using (var reader = new BinaryReader(new MemoryStream(data)))
                {
                    LODGroup lodGroup = new LODGroup();
                    lodGroup.Name = reader.ReadString();
                    var lodCount = reader.ReadInt32();

                    for (int i = 0; i < lodCount; ++i)
                    {
                        lodGroup.LODs.Add(BinaryToLOD(reader));
                    }

                    _lodGroups.Clear();
                    _lodGroups.Add(lodGroup);
                }

                // テスト用
                // PackForEngine();

            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, $"{filePath}ジオメトリアセットファイルのロードに失敗しました");
            }
        }

        public override IEnumerable<string> Save(string filePath)
        {
            Debug.Assert(_lodGroups.Any());
            var savedFiles = new List<string>();
            if (!_lodGroups.Any())
                return savedFiles;

            var path = Path.GetDirectoryName(filePath) + Path.DirectorySeparatorChar;
            var fileName = Path.GetFileNameWithoutExtension(filePath);

            try
            {
                foreach (var lodGroup in _lodGroups)
                {
                    Debug.Assert(lodGroup.LODs.Any());
                    var meshFileName = ContentHelper.SanitizeFileName(
                        path + fileName +
                        ((_lodGroups.Count > 1) ? "_" + ((lodGroup.LODs.Count > 1) ? lodGroup.Name : lodGroup.LODs[0].Name) : string.Empty))
                        + AssetFileExtension;
                    // アセットファイルごとにGUIDを生成(同じファイル名のジオメトリは同じID)
                    Guid = TryGetAssetInfo(meshFileName) is AssetInfo info && info.Type == Type ? info.Guid : Guid.NewGuid();
                    byte[] lodGroupData = null;
                    using (var writer = new BinaryWriter(new MemoryStream()))
                    {
                        writer.Write(lodGroup.Name);
                        writer.Write(lodGroup.LODs.Count);
                        var hashes = new List<byte>();
                        foreach (var lod in lodGroup.LODs)
                        {
                            LODToBinary(lod, writer, out var hash);
                            hashes.AddRange(hash);
                        }

                        Hash = ContentHelper.ComputeHash(hashes.ToArray());
                        lodGroupData = (writer.BaseStream as MemoryStream).ToArray();
                        Icon = GenerateIcon(lodGroup.LODs[0]);
                    }

                    Debug.Assert(lodGroupData?.Length > 0);

                    using (var writer = new BinaryWriter(File.Open(meshFileName, FileMode.Create, FileAccess.Write)))
                    {
                        WriteAssetFileHeader(writer);
                        ImportSetting.ToBinary(writer);
                        writer.Write(lodGroupData.Length);
                        writer.Write(lodGroupData);
                    }

                    savedFiles.Add(meshFileName);
                    Logger.Log(Verbosity.Display, $"ジオメトリを{meshFileName}に保存しました");
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, $"{filePath}のセーブに失敗しました");
            }

            return savedFiles;
        }

        /// <summary>
        /// ジオメトリをエンジンで使えるようにバイト列にパック
        /// </summary>
        /// <returns>
        /// 以下のバイト列
        /// struct{
        ///     uint32 LODCount,
        ///     struct {
        ///         f32 LODThreshold,
        ///         uint32 SubmeshCount,
        ///         uint32 SizeOfSubmeshes,
        ///         struct {
        ///             uint32 ElementSize, uint32 VertexCount,
        ///             uint32 IndexCount, uint32 ElementsType, uint32 PrimitiveTopology
        ///             uint8 Positions[sizeof(float32) * 3 * VertexCount],     // sizeof(Positions)は4バイトの倍数(必要ならパディング)
        ///             uint8 Elements[sizeof(ElementSize) * VertexCount], // sizeof(Elements)は4バイトの倍数(必要ならパディング)
        ///             uint8 Indices[IndexSize * IndexCount]
        ///         } Submeshes[SubmeshCount]
        ///     } MeshLODs[LODCount]
        /// } Geometry;
        /// </returns>
        public override byte[] PackForEngine()
        {
            using var writer = new BinaryWriter(new MemoryStream());

            writer.Write(GetLODGroup().LODs.Count);
            foreach (var lod in GetLODGroup().LODs)
            {
                writer.Write(lod.LODThreshold);
                writer.Write(lod.Meshes.Count);
                var sizeOfSubmeshesPosition = writer.BaseStream.Position;
                writer.Write(0);
                foreach (var mesh in lod.Meshes)
                {
                    writer.Write(mesh.ElementSize);
                    writer.Write(mesh.VertexCount);
                    writer.Write(mesh.IndexCount);
                    writer.Write((int)mesh.ElementsType);
                    writer.Write((int)mesh.PrimitveTopology);

                    var alignedPositionBuffer = new byte[MathUtil.AlignSizeUp(mesh.Positions.Length, 4)];
                    Array.Copy(mesh.Positions, alignedPositionBuffer, mesh.Positions.Length);
                    var alignedElementBuffer = new byte[MathUtil.AlignSizeUp(mesh.Elements.Length, 4)];
                    Array.Copy(mesh.Elements, alignedElementBuffer, mesh.Elements.Length);

                    writer.Write(alignedPositionBuffer);
                    writer.Write(alignedElementBuffer);
                    writer.Write(mesh.Indices); // アラインメントする必要はない
                }

                var endOfSubmeshes = writer.BaseStream.Position;
                var sizeOfSubmeshes = (int)(endOfSubmeshes - sizeOfSubmeshesPosition - sizeof(int));

                writer.BaseStream.Position = sizeOfSubmeshesPosition;
                writer.Write(sizeOfSubmeshes);
                writer.BaseStream.Position = endOfSubmeshes;
            }

            writer.Flush();
            var data = (writer.BaseStream as MemoryStream)?.ToArray();
            Debug.Assert(data?.Length > 0);

            // テスト用
            using (var fs = new FileStream(@"..\..\Test\test.model", FileMode.Create))
            {
                fs.Write(data, 0, data.Length);
            }

            return data;
        }

        public Geometry() : base(AssetType.Mesh) { }
    }
}
