using Editor.GameProject;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Input;

namespace Editor.Content
{
    class DataSizeToStringConverter : IValueConverter
    {
        private static readonly string[] _sizeSuffixes = { "B", "KB", "MB", "GB", "TB" };

        static string SizeSuffix(long value, int decimalPlaces = 1)
        {
            if (value <= 0 || decimalPlaces < 0)
                return string.Empty;

            int mag = (int)Math.Log(value, 1024);

            decimal adjustedSize = (decimal)value / (1L << (mag * 10));

            if (Math.Round(adjustedSize, decimalPlaces) >= 1000)
            {
                mag += 1;
                adjustedSize /= 1024;
            }

            return string.Format("{0:n" + decimalPlaces + "} {1}", adjustedSize, _sizeSuffixes[mag]);
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return (value is long size) ? SizeSuffix(size, 0) : null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    class PlainView : ViewBase
    {

        public static readonly DependencyProperty ItemContainerStyleProperty =
          ItemsControl.ItemContainerStyleProperty.AddOwner(typeof(PlainView));

        public Style ItemContainerStyle
        {
            get { return (Style)GetValue(ItemContainerStyleProperty); }
            set { SetValue(ItemContainerStyleProperty, value); }
        }

        public static readonly DependencyProperty ItemTemplateProperty =
            ItemsControl.ItemTemplateProperty.AddOwner(typeof(PlainView));

        public DataTemplate ItemTemplate
        {
            get { return (DataTemplate)GetValue(ItemTemplateProperty); }
            set { SetValue(ItemTemplateProperty, value); }
        }

        public static readonly DependencyProperty ItemWidthProperty =
            WrapPanel.ItemWidthProperty.AddOwner(typeof(PlainView));

        public double ItemWidth
        {
            get { return (double)GetValue(ItemWidthProperty); }
            set { SetValue(ItemWidthProperty, value); }
        }

        public static readonly DependencyProperty ItemHeightProperty =
            WrapPanel.ItemHeightProperty.AddOwner(typeof(PlainView));

        public double ItemHeight
        {
            get { return (double)GetValue(ItemHeightProperty); }
            set { SetValue(ItemHeightProperty, value); }
        }

        protected override object DefaultStyleKey => new ComponentResourceKey(GetType(), "PlainVewResourceID");
    }

    /// <summary>
    /// ContentBrowserView.xaml の相互作用ロジック
    /// </summary>
    public partial class ContentBrowserView : UserControl, IDisposable
    {
        private string _sortProperty = nameof(ContentInfo.FileName);
        private ListSortDirection _sortDirection;

        public SelectionMode SelectionMode
        {
            get { return (SelectionMode)GetValue(SelectionModeProperty); }
            set { SetValue(SelectionModeProperty, value); }
        }

        public static readonly DependencyProperty SelectionModeProperty =
            DependencyProperty.Register("SelectionMode", typeof(SelectionMode), typeof(ContentBrowserView), new PropertyMetadata(SelectionMode.Extended));



        public FileAccess FileAccess
        {
            get { return (FileAccess)GetValue(FileAccessProperty); }
            set { SetValue(FileAccessProperty, value); }
        }

        public static readonly DependencyProperty FileAccessProperty =
            DependencyProperty.Register("FileAccess", typeof(FileAccess), typeof(ContentBrowserView), new PropertyMetadata(FileAccess.ReadWrite));


        internal ContentInfo SelectedItem
        {
            get { return (ContentInfo)GetValue(SelectedInfoProperty); }
            set { SetValue(SelectedInfoProperty, value); }
        }

        public static readonly DependencyProperty SelectedInfoProperty =
            DependencyProperty.Register("SelectedInfo", typeof(ContentInfo), typeof(ContentBrowserView), new PropertyMetadata(null));



        public ContentBrowserView()
        {
            DataContext = null;
            InitializeComponent();
            Loaded += OnContentBrowserLoaded;
        }

        private void OnContentBrowserLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnContentBrowserLoaded;
            if (Application.Current.MainWindow != null)
            {
                Application.Current.MainWindow.DataContextChanged += OnProjectChanged;
            }

            OnProjectChanged(null, new DependencyPropertyChangedEventArgs(DataContextProperty, null, Project.Current));
            folderListView.AddHandler(Thumb.DragDeltaEvent, new DragDeltaEventHandler(Thumb_DragDelta), true);
            folderListView.Items.SortDescriptions.Add(new SortDescription(_sortProperty, _sortDirection));
        }

        private void Thumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            if (e.OriginalSource is Thumb thumb && thumb.TemplatedParent is GridViewColumnHeader header)
            {
                if (header.Column.ActualWidth < 50)
                {
                    header.Column.Width = 50;
                }
                else if (header.Column.ActualWidth > 250)
                {
                    header.Column.Width = 250;
                }
            }
        }

        private void OnProjectChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            (DataContext as ContentBrowser)?.Dispose();
            DataContext = null;
            if (e.NewValue is Project project)
            {
                Debug.Assert(e.NewValue == Project.Current);
                var contentBrowser = new ContentBrowser(project);
                contentBrowser.PropertyChanged += OnSelectedFolderChanged;
                DataContext = contentBrowser;
            }
        }

        private void OnSelectedFolderChanged(object sender, PropertyChangedEventArgs e)
        {
            var vm = sender as ContentBrowser;
            if (e.PropertyName == nameof(vm.SelectedFolderPath) && !string.IsNullOrEmpty(vm.SelectedFolderPath))
            {
                GeneratePathStackButtons();
            }
        }

        private void GeneratePathStackButtons()
        {
            var vm = DataContext as ContentBrowser;
            var path = Directory.GetParent(Path.TrimEndingDirectorySeparator(vm.SelectedFolderPath)).FullName;
            var contentPath = Path.TrimEndingDirectorySeparator(vm.ContentFolderPath);

            pathStack.Children.RemoveRange(1, pathStack.Children.Count - 1);
            if (vm.SelectedFolderPath == vm.ContentFolderPath)
                return;

            string[] paths = new string[3];
            string[] labels = new string[3];

            int i;
            for (i = 0; i < 3; i++)
            {
                paths[i] = path;
                labels[i] = path[(path.LastIndexOf(Path.DirectorySeparatorChar) + 1)..];
                if (path == contentPath)
                    break;

                path = path.Substring(0, path.LastIndexOf(Path.DirectorySeparatorChar));
            }

            if (i == 3)
                i = 2;

            for (; i >= 0; i--)
            {
                var button = new Button()
                {
                    DataContext = paths[i],
                    Content = new TextBlock() { Text = labels[i], TextTrimming = TextTrimming.CharacterEllipsis }
                };
                pathStack.Children.Add(button);
                if (i > 0)
                    pathStack.Children.Add(new System.Windows.Shapes.Path());
            }
        }

        private void OnGridViewColumnHeader_Click(object sender, RoutedEventArgs e)
        {
            var column = sender as GridViewColumnHeader;
            var sortBy = column.Tag.ToString();

            folderListView.Items.SortDescriptions.Clear();

            var newDir = ListSortDirection.Ascending;
            if (_sortProperty == sortBy && _sortDirection == newDir)
            {
                newDir = ListSortDirection.Descending;
            }

            _sortProperty = sortBy;
            _sortDirection = newDir;

            folderListView.Items.SortDescriptions.Add(new SortDescription(sortBy, newDir));
        }

        private void ExecuteSelection(ContentInfo info)
        {
            if (info == null)
                return;

            if (info.IsDirectory)
            {
                var vm = DataContext as ContentBrowser;
                vm.SelectedFolderPath = info.FullPath;
            }
        }

        private void OnContentItem_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            var info = (sender as FrameworkElement).DataContext as ContentInfo;
            ExecuteSelection(info);
        }

        private void OnContentItem_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                var info = (sender as FrameworkElement).DataContext as ContentInfo;
                ExecuteSelection(info);
            }
        }

        private void OnPathStackButton_Click(object sender, RoutedEventArgs e)
        {
            var vm = DataContext as ContentBrowser;
            vm.SelectedFolderPath = (sender as Button).DataContext as string;
        }

        private void OnFolderContent_ListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var item = folderListView.SelectedItem as ContentInfo;
            SelectedItem = item?.IsDirectory == true ? null : item;
        }

        public void Dispose()
        {
            if (Application.Current?.MainWindow != null)
            {
                Application.Current.MainWindow.DataContextChanged -= OnProjectChanged;
            }

            (DataContext as ContentBrowser)?.Dispose();
            DataContext = null;
        }
    }
}
