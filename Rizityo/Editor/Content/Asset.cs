﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

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

    sealed class AssetInfo
    {
        public AssetType Type { get; set; }
        public byte[] Icon { get; set; }
        public string FullPath { get; set; }
        public string FileName => Path.GetFileNameWithoutExtension(FullPath);
        public string SourcePath { get; set; }
        public DateTime RegisterTime { get; set; }
        public DateTime ImportDate { get; set; }
        public Guid Guid { get; set; }
        public byte[] Hash { get; set; }
    }

    abstract class Asset : ViewModelBase
    {
        public AssetType Type { get; private set; }
        public byte[] Icon { get; protected set; }

        private string _fullPath;
        public string FullPath
        {
            get => _fullPath;
            set
            {
                if (_fullPath != value)
                {
                    _fullPath = value;
                    OnPropertyChanged(nameof(FullPath));
                    OnPropertyChanged(nameof(FileName));
                }
            }
        }
        public string FileName => Path.GetFileNameWithoutExtension(FullPath);
        public string SourcePath { get; protected set; }
        public Guid Guid { get; protected set; } = Guid.NewGuid();
        public DateTime ImportDate { get; protected set; }
        public byte[] Hash { get; protected set; }

        public static string AssetFileExtension => ".rasset";

        public abstract void Import(string filePath);
        public abstract void Load(string filePath);
        public abstract IEnumerable<string> Save(string filePath);
        public abstract byte[] PackForEngine();

        private static AssetInfo GetAssetInfo(BinaryReader reader)
        {
            reader.BaseStream.Position = 0;
            var info = new AssetInfo();

            info.Type = (AssetType)reader.ReadInt32();
            var idSize = reader.ReadInt32();
            info.Guid = new Guid(reader.ReadBytes(idSize));
            info.ImportDate = DateTime.FromBinary(reader.ReadInt64());
            var hashSize = reader.ReadInt32();
            if (hashSize > 0)
            {
                info.Hash = reader.ReadBytes(hashSize);
            }
            var iconSize = reader.ReadInt32();
            info.Icon = reader.ReadBytes(iconSize);

            return info;
        }

        public static AssetInfo GetAssetInfo(string filePath)
        {
            Debug.Assert(File.Exists(filePath) && Path.GetExtension(filePath) == AssetFileExtension);
            try
            {
                using var reader = new BinaryReader(File.Open(filePath, FileMode.Open, FileAccess.Read));
                var info = GetAssetInfo(reader);
                info.FullPath = filePath;
                return info;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }

            return null;
        }

        public static AssetInfo TryGetAssetInfo(string filePath) =>
            (File.Exists(filePath) && Path.GetExtension(filePath) == AssetFileExtension) ? AssetRegistry.GetAssetInfo(filePath) ?? GetAssetInfo(filePath) : null;

        protected void ReadAssetFileHeader(BinaryReader reader)
        {
            var info = GetAssetInfo(reader);

            Debug.Assert(Type == info.Type);
            Guid = info.Guid;
            ImportDate = info.ImportDate;
            Hash = info.Hash;
            Icon = info.Icon;
        }

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
