﻿using System;
using System.Collections.Generic;
using System.Linq;
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
}
