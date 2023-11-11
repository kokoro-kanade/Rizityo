using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Editor.Content
{
    enum AssetType
    {
        Unknown,
        Animation, 
        Audio,
        Material,
        Mesh,
        Skeleton,
        Texture,
    }

    abstract class Asset : ViewModelBase
    {
        public AssetType Type { get; private set; }
        public byte[] Icon { get; protected set; }
        public string SourcePath { get; protected set; }
        public Guid Guid { get; protected set; } = Guid.NewGuid();
        public DateTime ImportDate { get; protected set; }
        public byte[] Hash { get; protected set; }

        public static string AssetFileExtension => ".rasset";

        public abstract IEnumerable<string> Save(string file);

        protected void WriteAssetFileHeader(BinaryWriter writer)
        {
            writer.BaseStream.Position = 0;

            writer.Write((int)Type);

            var id = Guid.ToByteArray();
            writer.Write(id.Length);
            writer.Write(id);

            var importDate = DateTime.Now.ToBinary();
            writer.Write(importDate);

            if(Hash?.Length > 0)
            {
                writer.Write(Hash.Length);
                writer.Write(Hash);
            }
            else
            {
                writer.Write(0);
            }

            writer.Write(SourcePath ?? "");

            writer.Write(Icon.Length);
            writer.Write(Icon);
        }

        public Asset(AssetType type)
        {
            Debug.Assert(type != AssetType.Unknown);
            Type = type;
        }
    }
}
