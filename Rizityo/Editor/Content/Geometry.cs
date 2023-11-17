using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Editor.Common;
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

    class Mesh : ViewModelBase
    {
        private int _vertexSize;
        public int VertexSize
        {
            get => _vertexSize;
            set
            {
                if (_vertexSize != value)
                {
                    _vertexSize = value;
                    OnPropertyChanged(nameof(VertexSize));
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

        public byte[] Vertices { get; set; }
        public byte[] Indices { get; set; }
    }

    class MeshLod : ViewModelBase
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
        public float LodThreshold
        {
            get => _lodThreshold;
            set
            {
                if (_lodThreshold != value)
                {
                    _lodThreshold = value;
                    OnPropertyChanged(nameof(LodThreshold));
                }
            }
        }

        public ObservableCollection<Mesh> Meshes { get; } = new ObservableCollection<Mesh>();
    }

    class LodGroup : ViewModelBase
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

        public ObservableCollection<MeshLod> Lods { get; } = new ObservableCollection<MeshLod>();
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

        private bool _importEmbededTextures;
        public bool ImportEmbededTextures
        {
            get => _importEmbededTextures;
            set
            {
                if (_importEmbededTextures != value)
                {
                    _importEmbededTextures = value;
                    OnPropertyChanged(nameof(ImportEmbededTextures));
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
            ImportEmbededTextures = true;
            ImportAnimations = true;
        }

        public void ToBinary(BinaryWriter writer)
        {
            writer.Write(SmoothingAngle);
            writer.Write(CalculateNormals);
            writer.Write(CalculateTangents);
            writer.Write(ReverseHandedness);
            writer.Write(ImportEmbededTextures);
            writer.Write(ImportAnimations);
        }
    }

    class Geometry : Asset
    {
        private readonly List<LodGroup> _lodGroups = new List<LodGroup>();

        public GeometryImportSetting ImportSetting { get; } = new GeometryImportSetting();

        public LodGroup GetLodGroup(int lodGroup = 0)
        {
            Debug.Assert(lodGroup >= 0 && lodGroup < _lodGroups.Count());
            return _lodGroups.Any() ? _lodGroups[lodGroup] : null;
        }

        private static void ReadMesh(BinaryReader reader, List<int> lodIds, List<MeshLod> lodList)
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

            var mesh = new Mesh();

            var lodId = reader.ReadInt32();
            mesh.VertexSize = reader.ReadInt32();
            mesh.VertexCount = reader.ReadInt32();
            mesh.IndexSize = reader.ReadInt32();
            mesh.IndexCount = reader.ReadInt32();
            var lodThreshold = reader.ReadSingle();

            // 頂点データ
            var vertexBufferSize = mesh.VertexSize * mesh.VertexCount;
            mesh.Vertices = reader.ReadBytes(vertexBufferSize);

            // インデックスデータ
            var indexBufferSize = mesh.IndexSize * mesh.IndexCount;
            mesh.Indices = reader.ReadBytes(indexBufferSize);

            // 同じLODidのものをまとめる
            MeshLod lod;
            if (ID.IsValid(lodId) && lodIds.Contains(lodId))
            {
                lod = lodList[lodIds.IndexOf(lodId)];
                Debug.Assert(lod != null);
            }
            else
            {
                lodIds.Add(lodId);
                lod = new MeshLod() { Name = meshName, LodThreshold = lodThreshold };
                lodList.Add(lod);
            }

            lod.Meshes.Add(mesh);
        }

        private static List<MeshLod> ReadMeshLods(int numMeshes, BinaryReader reader)
        {
            var lodIds = new List<int>();
            var lodList = new List<MeshLod>();
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
                List<MeshLod> lods = ReadMeshLods(numMeshes, reader);

                var lodGroup = new LodGroup() { Name = lodGroupName };
                lods.ForEach(lod => lodGroup.Lods.Add(lod));

                _lodGroups.Add(lodGroup);
            }
        }

        private void LodToBinary(MeshLod lod, BinaryWriter writer, out byte[] hash)
        {
            writer.Write(lod.Name);
            writer.Write(lod.LodThreshold);
            writer.Write(lod.Meshes.Count);

            var meshDataBegin = writer.BaseStream.Position;

            foreach (var mesh in lod.Meshes)
            {
                writer.Write(mesh.VertexSize);
                writer.Write(mesh.VertexCount);
                writer.Write(mesh.IndexSize);
                writer.Write(mesh.IndexCount);
                writer.Write(mesh.Vertices);
                writer.Write(mesh.Indices);
            }

            var meshDataSize = writer.BaseStream.Position - meshDataBegin;
            Debug.Assert(meshDataSize > 0);

            var data = (writer.BaseStream as MemoryStream).ToArray();
            hash = ContentHelper.ComputeHash(data, (int)meshDataBegin, (int)meshDataSize);
        }

        private byte[] GenerateIcon(MeshLod meshLod)
        {
            var width = 90 * 4;
            BitmapSource bmp = null;
            Application.Current.Dispatcher.Invoke(() =>
            {
                bmp = Editors.GeometryView.RenderToBitmap(new Editors.MeshRenderer(meshLod, null), width, width);
                bmp = new TransformedBitmap(bmp, new ScaleTransform(0.25, 0.25, 0.5, 0.5));
            });

            using var memoryStream = new MemoryStream();
            memoryStream.SetLength(0);
            var encoder = new PngBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(bmp));
            encoder.Save(memoryStream);

            return memoryStream.ToArray();
        }

        public override IEnumerable<string> Save(string file)
        {
            Debug.Assert(_lodGroups.Any());
            var savedFiles = new List<string>();
            if (!_lodGroups.Any())
                return savedFiles;

            var path = Path.GetDirectoryName(file) + Path.DirectorySeparatorChar;
            var fileName = Path.GetFileNameWithoutExtension(file);

            try
            {
                foreach (var lodGroup in _lodGroups)
                {
                    Debug.Assert(lodGroup.Lods.Any());
                    var meshFilename = ContentHelper.SanitizeFileName(_lodGroups.Count > 1 ?
                        path + fileName + "_" + lodGroup.Lods[0].Name + AssetFileExtension :
                        path + fileName + AssetFileExtension);
                    // アセットファイルごとにGUIDを生成
                    Guid = Guid.NewGuid();
                    byte[] lodGroupData = null;
                    using (var writer = new BinaryWriter(new MemoryStream()))
                    {
                        writer.Write(lodGroup.Name);
                        writer.Write(lodGroup.Lods.Count);
                        var hashes = new List<byte>();
                        foreach (var lod in lodGroup.Lods)
                        {
                            LodToBinary(lod, writer, out var hash);
                            hashes.AddRange(hash);
                        }

                        Hash = ContentHelper.ComputeHash(hashes.ToArray());
                        lodGroupData = (writer.BaseStream as MemoryStream).ToArray();
                        Icon = GenerateIcon(lodGroup.Lods[0]);
                    }

                    Debug.Assert(lodGroupData?.Length > 0);

                    using (var writer = new BinaryWriter(File.Open(meshFilename, FileMode.Create, FileAccess.Write)))
                    {
                        WriteAssetFileHeader(writer);
                        ImportSetting.ToBinary(writer);
                        writer.Write(lodGroupData);
                    }

                    savedFiles.Add(meshFilename);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(Verbosity.Error, $"{file}のセーブに失敗しました");
            }

            return savedFiles;
        }

        

        public Geometry() : base(AssetType.Mesh) { }
    }
}
