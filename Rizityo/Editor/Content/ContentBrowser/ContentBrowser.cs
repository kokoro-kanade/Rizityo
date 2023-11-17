using Editor.Common;
using Editor.GameProject;
using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace Editor.Content
{
    sealed class ContentInfo
    {
        public static int IconWidth => 90;
        public byte[] Icon { get; }
        public byte[] IconSmall { get; }
        public string FullPath { get; }
        public string FileName => Path.GetFileNameWithoutExtension(FullPath);
        public bool IsDirectory { get; }
        public DateTime DateModified { get; }
        public long? Size { get; }

        public ContentInfo(string fullPath, byte[] icon = null, byte[] smallIcon = null, DateTime? lastModified = null)
        {
            Debug.Assert(File.Exists(fullPath));
            var info = new FileInfo(fullPath);
            IsDirectory = ContentHelper.IsDirectory(fullPath);
            DateModified = lastModified ?? info.LastWriteTime;
            Size = IsDirectory ? (long?)null : info.Length;
            Icon = icon;
            IconSmall = smallIcon ?? icon;
            FullPath = fullPath;
        }
    }
    class ContentBrowser : ViewModelBase, IDisposable
    {
        private readonly DelayEventTimer _refreshTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));

        public string ContentFolderPath { get; }

        private readonly ObservableCollection<ContentInfo> _folderContent = new ObservableCollection<ContentInfo>();
        public ReadOnlyObservableCollection<ContentInfo> FolderContent { get; }

        private string _selectedFolderPath;
        public string SelectedFolderPath
        {
            get => _selectedFolderPath;
            set
            {
                if (_selectedFolderPath != value)
                {
                    _selectedFolderPath = value;
                    if (!string.IsNullOrEmpty(_selectedFolderPath))
                    {
                        _ = GetFolderContent();
                    }
                    OnPropertyChanged(nameof(SelectedFolderPath));
                }
            }
        }

        private void Refresh(object sender, DelayEventTimerArgs e)
        {
            _ = GetFolderContent();
        }

        private async Task GetFolderContent()
        {
            var folderContent = new List<ContentInfo>();
            await Task.Run(() =>
            {
                folderContent = GetFolderContent(SelectedFolderPath);
            });

            _folderContent.Clear();
            folderContent.ForEach(x => _folderContent.Add(x));
        }

        private List<ContentInfo> GetFolderContent(string path)
        {
            Debug.Assert(!string.IsNullOrEmpty(path));
            var folderContent = new List<ContentInfo>();

            try
            {
                foreach (var dir in Directory.GetDirectories(path))
                {
                    folderContent.Add(new ContentInfo(dir));
                }
                foreach (var file in Directory.GetFiles(path, $"*{Asset.AssetFileExtension}"))
                {
                    var fileInfo = new FileInfo(file);
                    folderContent.Add(ContentInfoCache.Add(file));
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }

            return folderContent;
        }

        private void OnContentModified(object sender, ContentModifiedEventArgs e)
        {
            if (Path.GetDirectoryName(e.FullPath) != SelectedFolderPath)
                return;

            _refreshTimer.Trigger();
        }

        public void Dispose()
        {
            ContentWatcher.ContentModified -= OnContentModified;
            ContentInfoCache.Save();
        }

        public ContentBrowser(Project project)
        {
            Debug.Assert(project != null);
            var contentFolderPath = project.ContentFolderPath;
            Debug.Assert(!string.IsNullOrEmpty(contentFolderPath.Trim()));
            contentFolderPath = Path.TrimEndingDirectorySeparator(contentFolderPath);
            ContentFolderPath = contentFolderPath;
            SelectedFolderPath = contentFolderPath;
            FolderContent = new ReadOnlyObservableCollection<ContentInfo>(_folderContent);

            ContentWatcher.ContentModified += OnContentModified;
            _refreshTimer.Triggered += Refresh;
        }


    }
}
