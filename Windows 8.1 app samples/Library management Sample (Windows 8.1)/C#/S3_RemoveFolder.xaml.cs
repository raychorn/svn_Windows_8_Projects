//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using SDKTemplate;
using System;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace LibraryManagement
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario3 : SDKTemplate.Common.LayoutAwarePage
    {
        StorageLibrary picturesLibrary;

        public Scenario3()
        {
            this.InitializeComponent();
            this.GetPicturesLibrary();
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }

        /// <summary>
        /// Requests the user's permission to remove the selected location from the Pictures library.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void RemoveFolderButton_Click(object sender, RoutedEventArgs e)
        {
            var folderToRemove = (StorageFolder)FoldersComboBox.SelectedItem;
            if (await picturesLibrary.RequestRemoveFolderAsync(folderToRemove))
            {
                OutputTextBlock.Text = folderToRemove.DisplayName + " was removed from the Pictures library.";
            }
            else
            {
                OutputTextBlock.Text = "Operation canceled.";
            }
        }

        /// <summary>
        /// Gets the Pictures library and sets up the FoldersComboBox to list its folders.
        /// </summary>
        private async void GetPicturesLibrary()
        {
            picturesLibrary = await StorageLibrary.GetLibraryAsync(KnownLibraryId.Pictures);

            // Bind the ComboBox to the list of folders currently in the library
            FoldersComboBox.ItemsSource = picturesLibrary.Folders;

            // Update the states of our controls when the list of folders changes
            picturesLibrary.DefinitionChanged += async (StorageLibrary sender, object e) =>
                {
                    await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        UpdateControls();
                    });
                };

            if (picturesLibrary.Folders.Count > 0)
            {
                FoldersComboBox.SelectedIndex = 0;
            }
            UpdateControls();
        }

        /// <summary>
        /// Updates the Visibility and IsEnabled properties of UI controls that depend upon the Pictures library
        /// having at least one folder in its Folders list.
        /// </summary>
        private void UpdateControls()
        {
            bool libraryHasFolders = (picturesLibrary.Folders.Count > 0);
            FoldersComboBox.Visibility = libraryHasFolders ? Visibility.Visible : Visibility.Collapsed;
            LibraryEmptyTextBlock.Visibility = libraryHasFolders ? Visibility.Collapsed : Visibility.Visible;
            RemoveFolderButton.IsEnabled = libraryHasFolders;
        }
    }
}