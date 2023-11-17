using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows;
using Editor.Common;
using Editor.Utility;

namespace Editor.Content
{
    static class AssetRegistry
    {
        private static readonly Dictionary<string, AssetInfo> _assetDictionary = new Dictionary<string, AssetInfo>();
        private static readonly ObservableCollection<AssetInfo> _assets = new ObservableCollection<AssetInfo>();
        public static ReadOnlyObservableCollection<AssetInfo> Assets { get; } = new ReadOnlyObservableCollection<AssetInfo>(_assets);

        public static AssetInfo GetAssetInfo(string file) => _assetDictionary.ContainsKey(file) ? _assetDictionary[file] : null;
        public static AssetInfo GetAssetInfo(Guid guid) => _assets.FirstOrDefault(x => x.Guid == guid);

        private static void RegisterAsset(string file)
        {
            Debug.Assert(File.Exists(file));
            try
            {
                var fileInfo = new FileInfo(file);

                if (!_assetDictionary.ContainsKey(file) || _assetDictionary[file].RegisterTime.IsOlder(fileInfo.LastWriteTime))
                {
                    var info = Asset.GetAssetInfo(file);
                    Debug.Assert(info != null);
                    info.RegisterTime = DateTime.Now;
                    _assetDictionary[file] = info;
                    Debug.Assert(_assetDictionary.ContainsKey(file));
                    _assets.Add(_assetDictionary[file]);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }

        private static void UnregisterAsset(string file)
        {
            if(_assetDictionary.ContainsKey(file))
            {
                _assets.Remove(_assetDictionary[file]);
                _assetDictionary.Remove(file);
            }
        }

        private static void RegisterAllAssets(string folderPath)
        {
            Debug.Assert(Directory.Exists(folderPath));
            foreach (var entry in Directory.GetFileSystemEntries(folderPath))
            {
                if (ContentHelper.IsDirectory(entry))
                {
                    RegisterAllAssets(entry);
                }
                else
                {
                    RegisterAsset(entry);
                }
            }
        }

        private static void OnContentModified(object sender, ContentModifiedEventArgs e)
        {
            if (ContentHelper.IsDirectory(e.FullPath))
            {
                RegisterAllAssets(e.FullPath);
            }
            else if (File.Exists(e.FullPath))
            {
                RegisterAsset(e.FullPath);
            }

            _assets.Where(x => !File.Exists(x.FullPath)).ToList().ForEach(x => UnregisterAsset(x.FullPath));
        }

        public static void Reset(string contentFolderPath)
        {
            ContentWatcher.ContentModified -= OnContentModified;

            _assetDictionary.Clear();
            _assets.Clear();
            Debug.Assert(Directory.Exists(contentFolderPath));
            RegisterAllAssets(contentFolderPath);

            ContentWatcher.ContentModified += OnContentModified;
        }

        
    }
}
