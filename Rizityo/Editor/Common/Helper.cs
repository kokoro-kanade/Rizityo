using Editor.Content;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace Editor.Common
{
    static class VisualExtensions
    {
        public static T FindVisualAncestor<T>(this DependencyObject depObject) where T : DependencyObject
        {
            if (!(depObject is Visual))
                return null;

            var parent = VisualTreeHelper.GetParent(depObject);
            while (parent != null)
            {
                if (parent is T type)
                {
                    return type;
                }
                parent = VisualTreeHelper.GetParent(parent);
            }
            return null;
        }
    }

    public static class ContentHelper
    { 
        public static string GetRandomString(int length = 8)
        {
            if (length <= 0)
                length = 8;
            var n = length / 11;
            var sb = new StringBuilder();
            for (int i = 0; i <= n; i++)
            {
                sb.Append(Path.GetRandomFileName().Replace(".", ""));
            }
            return sb.ToString(0, length);
            
        }

        public static string SanitizeFileName(string name)
        {
            var path = new StringBuilder(name.Substring(0, name.LastIndexOf(Path.DirectorySeparatorChar) + 1));
            var file = new StringBuilder(name[(name.LastIndexOf(Path.DirectorySeparatorChar) + 1)..]);
            foreach (var c in Path.GetInvalidPathChars())
            {
                path.Replace(c, '_');
            }
            foreach (var c in Path.GetInvalidPathChars())
            {
                file.Replace(c, '_');
            }
            return path.Append(file).ToString();

        }

        public static byte[] ComputeHash(byte[] data, int offset = 0, int count = 0)
        {
            if (data?.Length > 0)
            {
                using var sha256 = SHA256.Create();
                return sha256.ComputeHash(data, offset, count > 0 ? count : data.Length);
            }

            return null;
        }

        public static bool IsDirectory(string path)
        {
            try
            {
                return File.GetAttributes(path).HasFlag(FileAttributes.Directory);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
            
            return false;
        }

        public static bool IsDirectory(this FileInfo info) => info.Attributes.HasFlag(FileAttributes.Directory);

        public static bool IsOlder(this DateTime date, DateTime other) => date < other;

        private static void Import(Asset asset, string name, string filePath, string dstFolderPath)
        {
            Debug.Assert(asset != null);

            asset.FullPath = dstFolderPath + name + Asset.AssetFileExtension;
            if (!string.IsNullOrEmpty(filePath))
            {
                asset.Import(filePath);
            }

            asset.Save(asset.FullPath);
            return;
        }

        private static void Import(string filePath, string dstFolderPath)
        {
            Debug.Assert(!string.IsNullOrEmpty(filePath));
            if (IsDirectory(filePath))
                return;

            if(!dstFolderPath.EndsWith(Path.DirectorySeparatorChar))
            {
                dstFolderPath += Path.DirectorySeparatorChar;
            }

            var name = Path.GetFileNameWithoutExtension(filePath).ToLower();
            var extension = Path.GetExtension(filePath).ToLower();

            Asset asset = null;

            switch (extension)
            {
                case ".fbx":
                    asset = new Content.Geometry();
                    break;
                case ".bmp": break;
                case ".png": break;
                case ".jpg": break;
                case ".jpeg": break;
                case ".tiff": break;
                case ".tif": break;
                case ".tga": break;
                case ".wav": break;
                case ".ogg": break;
                default:
                    break;
            }

            if (asset != null)
            {
                Import(asset, name, filePath, dstFolderPath);
            }
        }

        public static async Task ImportFilesAsync(string[] filePaths, string dstFolderPath)
        {
            try
            {
                Debug.Assert(!string.IsNullOrEmpty(dstFolderPath));
                ContentWatcher.EnableFileWatcher(false);
                var tasks = filePaths.Select(async filePath => await Task.Run(() => { Import(filePath, dstFolderPath); }));
                await Task.WhenAll(tasks);
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"{dstFolderPath}へのファイルのインポートに失敗しました");
                Debug.WriteLine(ex.Message);
            }
            finally
            {
                ContentWatcher.EnableFileWatcher(true);
            }
        }

        
    }

}
