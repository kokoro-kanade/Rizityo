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
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Editor.GameProject
{
    /// <summary>
    /// ProjectBrowserDialog.xaml の相互作用ロジック
    /// </summary>
    public partial class ProjectBrowserDialog : Window
    {
        public ProjectBrowserDialog()
        {
            InitializeComponent();
            Loaded += OnProjectBroswerDialogLoaded;
        }

        public static bool MoveNewProjectTab { get; set; }

        private void OnProjectBroswerDialogLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnProjectBroswerDialogLoaded;
            if (!OpenProject.Projects.Any() || MoveNewProjectTab)
            {
                if (!MoveNewProjectTab)
                {
                    openProjectButton.IsEnabled = false;
                    openProjectView.Visibility = Visibility.Hidden;
                }
                
                OnToggleButton_Click(createProjectButton, new RoutedEventArgs());
            }

            MoveNewProjectTab = false;
        }

        //private void AnimateToCreateProject()
        //{
        //    var highlightAnimation = new DoubleAnimation(200, 400, new Duration(TimeSpan.FromSeconds(0.2)));
        //    highlightAnimation.Completed += (s, e) =>
        //    {
        //        var animation = new ThicknessAnimation(new Thickness(-800, 0, 0, 0), new Thickness(0), new Duration(TimeSpan.FromSeconds(0.5)));
        //        browserContent.BeginAnimation(MarginProperty, animation);
        //    };
        //    highlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        //}

        //private void AnimateToOpenProject()
        //{
        //    var highlightAnimation = new DoubleAnimation(400, 200, new Duration(TimeSpan.FromSeconds(0.2)));
        //    highlightAnimation.Completed += (s, e) =>
        //    {
        //        var animation = new ThicknessAnimation(new Thickness(0), new Thickness(-800, 0, 0, 0), new Duration(TimeSpan.FromSeconds(0.5)));
        //        browserContent.BeginAnimation(MarginProperty, animation);
        //    };
        //    highlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        //}

        private void OnToggleButton_Click(object sender, RoutedEventArgs e)
        {
            if(sender == openProjectButton)
            {
                if(createProjectButton.IsChecked == true)
                {
                    createProjectButton.IsChecked = false;
                    //AnimateToOpenProject();
                    openProjectView.IsEnabled = true;
                    newProjectView.IsEnabled = false;
                    browserContent.Margin = new Thickness(0);
                }
                openProjectButton.IsChecked = true;
            }
            else
            {
                if(openProjectButton.IsChecked == true)
                {
                    openProjectButton.IsChecked = false;
                    //AnimateToCreateProject();
                    openProjectView.IsEnabled = false;
                    newProjectView.IsEnabled = true;
                    browserContent.Margin = new Thickness(-800, 0, 0, 0);
                }
                createProjectButton.IsChecked = true;
            }
        }

        
    }
}
