using Editor.Components;
using Editor.GameProject;
using Editor.Utility;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor.Editors
{
    /// <summary>
    /// TransformView.xaml の相互作用ロジック
    /// </summary>
    public partial class TransformView : UserControl
    {
        private Action _undoAction = null;
        private bool _propertyChanged = false;
        public TransformView()
        {
            InitializeComponent();
            Loaded += OnTransformViewLoaded;
        }

        private void OnTransformViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnTransformViewLoaded;
            (DataContext as MultiSelectedTransform).PropertyChanged += (s, e) => _propertyChanged = true;
        }

        private Action GetAction(Func<Transform, (Transform transform, Vector3)> selecter,
            Action<(Transform transform, Vector3)> forEachAction)
        {
            if (!(DataContext is MultiSelectedTransform vm))
            {
                _undoAction = null;
                _propertyChanged = false;
                return null;
            }

            var selection = vm.SelectedComponents.Select(t => selecter(t)).ToList();
            return new Action(() =>
            {
                selection.ForEach(p => forEachAction(p));
                (GameEntityView.Instance.DataContext as MultiSelectedEntity)?.GetMultiSelectedComponent<MultiSelectedTransform>().Refresh();
            });
        }

        private Action GetPositionAction() => GetAction(t => (t, t.Position), p => p.transform.Position = p.Item2);
        private Action GetRotationAction() => GetAction(t => (t, t.Rotation), p => p.transform.Rotation = p.Item2);
        private Action GetScaleAction() => GetAction(t => (t, t.Scale), p => p.transform.Scale = p.Item2);

        private void RecordAction(Action redoAction, string actionName)
        {
            if (_propertyChanged)
            {
                Debug.Assert(_undoAction != null);
                _propertyChanged = false;
                Project.UndoRedo.Add(new UndoRedoAction(_undoAction, redoAction, actionName));
            }
        }

        private void OnPositionVectorTextBox_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetPositionAction();
        }

        private void OnPositionVectorTextBox_PreviewMouse_LBU(object sender, MouseButtonEventArgs e)
        {
            RecordAction(GetPositionAction(), "位置を変更");
        }

        private void OnPositionVectorTextBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnPositionVectorTextBox_PreviewMouse_LBU(sender, null);
            }
        }

        private void OnRotationVectorTextBox_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetRotationAction();
        }

        private void OnRotationVectorTextBox_PreviewMouse_LBU(object sender, MouseButtonEventArgs e)
        {
            RecordAction(GetRotationAction(), "回転角を変更");
        }

        private void OnRotationVectorTextBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnRotationVectorTextBox_PreviewMouse_LBU(sender, null);
            }
        }

        private void OnScaleVectorTextBox_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetScaleAction();
        }

        private void OnScaleVectorTextBox_PreviewMouse_LBU(object sender, MouseButtonEventArgs e)
        {
            RecordAction(GetScaleAction(), "スケールを変更");
        }

        private void OnScaleVectorTextBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnScaleVectorTextBox_PreviewMouse_LBU(sender, null);
            }
        }
    }
}
