// // Copyright (c) Microsoft. All rights reserved.
// // Licensed under the MIT license. See LICENSE file in the project root for full license information.

using System.Windows;
using System.Windows.Media;

namespace FontDialog
{
    /// <summary>
    ///     Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void OnFontButtonClick(object sender, RoutedEventArgs e)
        {
            ShowFontDialog();
        }

        private Typeface typeface;

        private void ShowFontDialog()
        {
            var fontChooser = new FontChooser {Owner = this};

            fontChooser.SetPropertiesFromObject(textBox);
            fontChooser.PreviewSampleText = textBox.SelectedText;

            var showDialog = fontChooser.ShowDialog();
            
            if (showDialog != null && showDialog.Value)
            {
                fontChooser.ApplyPropertiesToObject(textBox);

                typeface = new Typeface(fontChooser.SelectedFontFamily, fontChooser.SelectedFontStyle, fontChooser.SelectedFontWeight, fontChooser.SelectedFontStretch);
                
            }
        }
    }
}