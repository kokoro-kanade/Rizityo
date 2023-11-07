﻿using Editor.Common;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Security.AccessControl;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor.Utility
{
    /// <summary>
    /// RenderSurfaceView.xaml の相互作用ロジック
    /// </summary>
    public partial class RenderSurfaceView : UserControl, IDisposable
    {
        private enum Win32Msg
        {
            WM_SIZE = 0x0005,
            WM_SIZING = 0x0214,
            WM_ENTERSIZEMOVE = 0x0231,
            WM_EXITSIZEMOVE = 0x0232,
        }

        private RenderSurfaceHost _host = null;
        private bool _canResize = true;
        private bool _moved = false;

        public RenderSurfaceView()
        {
            InitializeComponent();
            Loaded += OnRenderSurfaceViewLoaded;
        }

        private void OnRenderSurfaceViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnRenderSurfaceViewLoaded;

            _host = new RenderSurfaceHost(ActualWidth, ActualHeight);
            _host.MessageHook += new HwndSourceHook(HostMsgFilter);
            Content = _host;

            var window = this.FindVisualAncestor<Window>();
            Debug.Assert(window != null);
            var helper = new WindowInteropHelper(window);
            if (helper.Handle != IntPtr.Zero)
            {
                HwndSource.FromHwnd(helper.Handle)?.AddHook(HwndMessageHook);
            }
        }

        private IntPtr HwndMessageHook(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch ((Win32Msg)msg)
            {
                case Win32Msg.WM_SIZING:
                    _canResize = false;
                    _moved = false;
                    break;
                case Win32Msg.WM_ENTERSIZEMOVE:
                    _moved = true;
                    break;
                case Win32Msg.WM_EXITSIZEMOVE:
                    _canResize = true;
                    if (!_moved)
                    {
                        _host.Resize();
                    }
                    break;
                default:
                    break;
            }

            return IntPtr.Zero;
        }

        private IntPtr HostMsgFilter(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch ((Win32Msg)msg)
            {
                case Win32Msg.WM_SIZE:
                    if (_canResize)
                    {
                        _host.Resize();
                    }
                    break;
                case Win32Msg.WM_SIZING:
                    break;
                case Win32Msg.WM_ENTERSIZEMOVE:
                    break;
                case Win32Msg.WM_EXITSIZEMOVE:
                    break;
                default:
                    break;
            }

            return IntPtr.Zero;
        }

        #region IDisposable
        private bool _disposedValue;

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposedValue)
            {
                if (disposing)
                {
                    _host.Dispose();
                }

                _disposedValue = true;
            }
        }

        public void Dispose()
        {
            // このコードを変更しないでください。クリーンアップ コードを 'Dispose(bool disposing)' メソッドに記述します
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}
