using Editor.Components;
using Editor.GameProject;
using Editor.Utility;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace Editor.Editors
{
    /// <summary>
    /// ProjectView.xaml の相互作用ロジック
    /// </summary>
    public partial class ProjectView : UserControl
    {
        public ProjectView()
        {
            InitializeComponent();
        }

        private void OnAddGameEntityButton_Click(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            var vm = button.DataContext as Level;
            vm.AddGameEntityCommand.Execute(new GameEntity(vm) { Name = "Game Entity"});
        }

        private void OnGameEntitiesListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            GameEntityView.Instance.DataContext = null;
            var listBox = sender as ListBox;

            var nowSelectedItems = listBox.SelectedItems.Cast<GameEntity>().ToList();
            var prevSelectedItems = nowSelectedItems.Except(e.AddedItems.Cast<GameEntity>()).Concat(e.RemovedItems.Cast<GameEntity>()).ToList(); // 前の選択 = 今も選択されている + 選択されなくなった

            Project.UndoRedo.Add(new UndoRedoAction(
                () =>
                {
                    listBox.UnselectAll();
                    prevSelectedItems.ForEach(x => (listBox.ItemContainerGenerator.ContainerFromItem(x) as ListBoxItem).IsSelected = true);
                },
                () =>
                {
                    listBox.UnselectAll();
                    nowSelectedItems.ForEach(x => (listBox.ItemContainerGenerator.ContainerFromItem(x) as ListBoxItem).IsSelected = true);
                },
                "Selection Changed")
            );

            MultiSelectedGameEntity msEntity = null;
            if (nowSelectedItems.Any())
            {
                msEntity = new MultiSelectedGameEntity(nowSelectedItems);
            }
            GameEntityView.Instance.DataContext = msEntity;
        }
    }
}
