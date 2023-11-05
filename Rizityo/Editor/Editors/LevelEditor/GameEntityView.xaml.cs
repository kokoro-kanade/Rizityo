using Editor.Components;
using Editor.GameProject;
using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Input;


namespace Editor.Editors
{
    public class NullableBoolToBoolConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is bool b && b == true;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is bool b && b == true;
        }
    }
    /// <summary>
    /// GameEntityView.xaml の相互作用ロジック
    /// </summary>
    public partial class GameEntityView : UserControl
    {
        private Action _undoAction;
        private string _propertyName;
        public static GameEntityView Instance { get; private set; }
        public GameEntityView()
        {
            InitializeComponent();
            DataContext = null;
            Instance = this;
            DataContextChanged += (_, __) =>
            {
                if (DataContext != null)
                {
                    (DataContext as MultiSelectedEntity).PropertyChanged += (s, e) => _propertyName = e.PropertyName;
                }
            };
        }

        private Action GetRenameAction()
        {
            var vm = DataContext as MultiSelectedEntity;
            var selectedEntitiesNameList = vm.SelectedEntities.Select(entity => (entity, entity.Name)).ToList();
            return new Action(() =>
            {
                selectedEntitiesNameList.ForEach(x => x.entity.Name = x.Name);
                (DataContext as MultiSelectedEntity).Refresh();
            });
        }

        // フォーカスされた時点での古いプロパティ値でUndo
        private void OnNameTextBox_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            _propertyName = string.Empty;
            _undoAction = GetRenameAction();
        }

        // フォーカスが外れた時点での新しいプロパティ値でRedo
        private void OnNameTextBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyName == nameof(MultiSelectedEntity.Name) && _undoAction != null)
            {

                var redoAction = GetRenameAction();
                Project.UndoRedo.Add(new UndoRedoAction(_undoAction, redoAction, "ゲームエンティティの名前を変更"));
                _propertyName = null;
            }
            _undoAction = null;
        }

        private Action GetIsEnabledAction()
        {
            var vm = DataContext as MultiSelectedEntity;
            var selectedEntitiesNameList = vm.SelectedEntities.Select(entity => (entity, entity.IsEnabled)).ToList();
            return new Action(() =>
            {
                selectedEntitiesNameList.ForEach(x => x.entity.IsEnabled = x.IsEnabled);
                (DataContext as MultiSelectedEntity).Refresh();
            });
        }
        private void OnIsEnabledCheckBox_Click(object sender, RoutedEventArgs e)
        {
            var undoAction = GetIsEnabledAction();
            var vm = DataContext as MultiSelectedEntity;
            vm.IsEnabled = (sender as CheckBox).IsChecked == true;
            var redoAction = GetIsEnabledAction();
            Project.UndoRedo.Add(new UndoRedoAction(undoAction, redoAction,
                vm.IsEnabled == true ? "ゲームエンティティを有効化" : "ゲームエンティティを無効化"));
        }

        private void OnAddComponentButton_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            var menu = FindResource("addComponentMenu") as ContextMenu;
            var button = sender as ToggleButton;
            button.IsChecked = true;
            menu.Placement = PlacementMode.Bottom;
            menu.PlacementTarget = button;
            menu.MinWidth = button.ActualWidth;
            menu.IsOpen = true;
        }

        private void AddComponent(ComponentType componentType, object data)
        {
            var createFunc = ComponentsFactory.GetCreateFunc(componentType);
            var changedEntities = new List<(GameEntity entity, Component component)>();
            var vm = DataContext as MultiSelectedEntity;
            foreach (var entity in vm.SelectedEntities)
            {
                var component = createFunc(entity, data);
                if (entity.AddComponent(component))
                {
                    changedEntities.Add((entity, component));
                }
            }

            if (changedEntities.Any())
            {
                vm.Refresh();
                Project.UndoRedo.Add(new UndoRedoAction(
                    () =>
                    {
                        changedEntities.ForEach(x => x.entity.RemoveComponent(x.component));
                        (DataContext as MultiSelectedEntity).Refresh();
                    },
                    () =>
                    {
                        changedEntities.ForEach(x => x.entity.AddComponent(x.component));
                        (DataContext as MultiSelectedEntity).Refresh();
                    },
                    $"{componentType}コンポーネントを追加しました"));
            }
        }

        private void OnAddScriptComponent(object sender, RoutedEventArgs e)
        {
            AddComponent(ComponentType.Script, (sender as MenuItem).Header.ToString());
        }
    }
}
