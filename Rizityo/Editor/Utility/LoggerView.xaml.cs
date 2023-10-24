using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor.Utility
{
    /// <summary>
    /// LoggerView.xaml の相互作用ロジック
    /// </summary>
    public partial class LoggerView : UserControl
    {
        public LoggerView()
        {
            InitializeComponent();
        }

        private void OnClearButton_Click(object sender, RoutedEventArgs e)
        {
            Logger.Clear();
        }

        private void OnFilterButton_Click(object sender, RoutedEventArgs e)
        {
            var filter = 0x0;
            if (toggleDisplay.IsChecked == true)
                filter |= (int)Verbosity.Display;
            if (toggleWarning.IsChecked == true)
                filter |= (int)Verbosity.Warning;
            if (toggleError.IsChecked == true)
                filter |= (int)Verbosity.Error;
            Logger.SetFilter(filter);
        }
    }
}
