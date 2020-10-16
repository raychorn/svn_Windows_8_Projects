//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using SDKTemplate;

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Media;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.Storage.Streams;

namespace MediaTransportControls
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario1
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;
        Windows.Media.SystemMediaTransportControls smtc = null;
        IReadOnlyList<StorageFile> playlist = null;
        int currentIndex = 0;
        bool isPlaying = false;
        bool isChangingMedia = false;

        public Scenario1()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            smtc = Windows.Media.SystemMediaTransportControls.GetForCurrentView();
            smtc.PlaybackStatus = Windows.Media.MediaPlaybackStatus.Closed;
            smtc.ButtonPressed += smtc_ButtonPressed;
            smtc.IsPlayEnabled = false;
            smtc.IsPauseEnabled = false;
            smtc.IsStopEnabled = false;

            MyMediaElement.CurrentStateChanged += MyMediaElement_CurrentStateChanged;
            MyMediaElement.MediaOpened += MyMediaElement_MediaOpened;
            MyMediaElement.MediaEnded += MyMediaElement_MediaEnded;
            MyMediaElement.MediaFailed += MyMediaElement_MediaFailed;

            rootPage.NotifyUser("events registered", NotifyType.StatusMessage);        
        }

        // supported audio and video formats for Windows Store apps:
        // http://msdn.microsoft.com/en-us/library/windows/apps/hh986969.aspx
        private static string[] supportedAudioFormats = new string[]{
            ".3g2", ".3gp2", ".3gp", ".3gpp", ".m4a", ".asf", ".wma", ".aac", ".adt", ".adts", ".mp3", ".wav", ".ac3", ".ec3",
        };

        private static string[] supportedVideoFormats = new string[]{
            ".3g2", ".3gp2", ".3gp", ".3gpp", ".m4v", ".mp4v", ".mp4", ".mov", ".m2ts", ".asf", ".wmv", ".avi",
        };

        private async void SelectFilesButton_Click(object sender, RoutedEventArgs e)
        {
            FileOpenPicker filePicker = new FileOpenPicker();
            filePicker.ViewMode = PickerViewMode.List;
            filePicker.SuggestedStartLocation = PickerLocationId.MusicLibrary;
            foreach (string extension in supportedAudioFormats)
            {
                filePicker.FileTypeFilter.Add(extension);
            }
            foreach (string extension in supportedVideoFormats)
            {
                filePicker.FileTypeFilter.Add(extension);
            }

            IReadOnlyList<StorageFile> newPlaylist = await filePicker.PickMultipleFilesAsync();
            if (newPlaylist.Count > 0)
            {
                rootPage.NotifyUser(String.Format("{0} file(s) selected", newPlaylist.Count), NotifyType.StatusMessage);
                currentIndex = 0;
                playlist = newPlaylist;
                isPlaying = true;
                await SetCurrentPlayingAsync(currentIndex);
            }
            else
            {
                // user canceled the file picker
                rootPage.NotifyUser("no files selected", NotifyType.StatusMessage);
            }
        }

        private void PlayPauseButton_Click(object sender, RoutedEventArgs e)
        {
            if (MyMediaElement.CurrentState == MediaElementState.Playing)
            {
                MyMediaElement.Pause();
            }
            else
            {
                MyMediaElement.Play();
            }
        }

        private void MyMediaElement_MediaOpened(object sender, RoutedEventArgs e)
        {
            isChangingMedia = false;
            if (isPlaying)
            {
                MyMediaElement.Play();
            }
        }

        private async void MyMediaElement_MediaEnded(object sender, RoutedEventArgs e)
        {
            if (currentIndex < playlist.Count - 1)
            {
                isPlaying = true;
                currentIndex++;
                await SetCurrentPlayingAsync(currentIndex);
            }
        }

        private void MyMediaElement_MediaFailed(object sender, RoutedEventArgs e)
        {
            isChangingMedia = false;
            string errorMessage = String.Format("Cannot play file {0}. Press Next or Previous to continue, or select new files to play.",
                                                playlist[currentIndex].Name);
            rootPage.NotifyUser(errorMessage, NotifyType.ErrorMessage);
        }

        private void MyMediaElement_CurrentStateChanged(object sender, RoutedEventArgs e)
        {
            if (MyMediaElement.CurrentState == MediaElementState.Playing)
            {
                isPlaying = true;
                PlayPauseButton.Content = "Pause";
            }
            else if (!isChangingMedia &&
                     (MyMediaElement.CurrentState == MediaElementState.Stopped || MyMediaElement.CurrentState == MediaElementState.Paused))
            {
                isPlaying = false;
                PlayPauseButton.Content = "Play";
            }

            switch (MyMediaElement.CurrentState)
            {
                case MediaElementState.Closed:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Closed;
                    break;

                case MediaElementState.Opening:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Changing;
                    break;

                case MediaElementState.Buffering:
                    // no changes in MediaPlaybackStatus
                    break;

                case MediaElementState.Paused:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Paused;
                    break;

                case MediaElementState.Playing:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Playing;
                    break;

                case MediaElementState.Stopped:
                    smtc.PlaybackStatus = MediaPlaybackStatus.Stopped;
                    break;
            }
        }

        private async void smtc_ButtonPressed(Windows.Media.SystemMediaTransportControls sender, Windows.Media.SystemMediaTransportControlsButtonPressedEventArgs args)
        {   
            switch (args.Button)
            {
                case SystemMediaTransportControlsButton.Next:
                    await DispatchNotifyUser("Next pressed", NotifyType.StatusMessage);
                    currentIndex++;
                    await SetCurrentPlayingAsync(currentIndex);
                    break;
                case SystemMediaTransportControlsButton.Previous:
                    await DispatchNotifyUser("Previous pressed", NotifyType.StatusMessage);
                    currentIndex--;
                    await SetCurrentPlayingAsync(currentIndex);
                    break;
                case SystemMediaTransportControlsButton.Play:
                    await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                    {
                        rootPage.NotifyUser("Play pressed", NotifyType.StatusMessage);
                        MyMediaElement.Play();
                    });
                    break;
                case SystemMediaTransportControlsButton.Pause:
                    await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                    {
                        rootPage.NotifyUser("Pause pressed", NotifyType.StatusMessage);
                        MyMediaElement.Pause();
                    });
                    break;
                case SystemMediaTransportControlsButton.Stop:
                    await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                    {
                        rootPage.NotifyUser("Stop pressed", NotifyType.StatusMessage);
                        MyMediaElement.Stop();
                    });
                    break;

                // Insert additional case statements for other buttons you want to handle in your app.
                // Remember that you also need to first enable those buttons via the corresponding
                // IsXXXXEnabled property on the SystemMediaTransportControls object.
            }
        }

        private Windows.Media.MediaPlaybackType GetMediaTypeFromFileContentType(StorageFile file)
        {
            Windows.Media.MediaPlaybackType mediaPlaybackType = MediaPlaybackType.Unknown;
            string fileMimeType = file.ContentType.ToLowerInvariant();

            if (fileMimeType.StartsWith("audio/"))
            {
                mediaPlaybackType = MediaPlaybackType.Music;
            }
            else if (fileMimeType.StartsWith("video/"))
            {
                mediaPlaybackType = MediaPlaybackType.Video;
            }
            else if (fileMimeType.StartsWith("image/"))
            {
                mediaPlaybackType = MediaPlaybackType.Image;
            }

            return mediaPlaybackType;
        }

        private async Task SetCurrentPlayingAsync(int playlistIndex)
        {
            if (playlistIndex <= 0)
            {
                smtc.IsPreviousEnabled = false;
                playlistIndex = 0;
            }
            else
            {
                smtc.IsPreviousEnabled = true;
            }

            if (playlistIndex >= playlist.Count - 1)
            {
                smtc.IsNextEnabled = false;
                playlistIndex = playlist.Count - 1;
            }
            else
            {
                smtc.IsNextEnabled = true;
            }

            string errorMessage = null;
            StorageFile mediaFile = playlist[playlistIndex];
            Windows.Storage.Streams.IRandomAccessStream stream = null;
            isChangingMedia = true;
            try
            {
                stream = await mediaFile.OpenAsync(Windows.Storage.FileAccessMode.Read);
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => 
                {
                    MyMediaElement.SetSource(stream, mediaFile.ContentType);
                    PlayPauseButton.IsEnabled = true;
                    smtc.IsPlayEnabled = true;
                    smtc.IsPauseEnabled = true;
                    smtc.IsStopEnabled = true;
                });
            }
            catch (Exception e)
            {
                errorMessage = e.Message;
                isChangingMedia = false;
            }

            if (await smtc.DisplayUpdater.CopyFromFileAsync(GetMediaTypeFromFileContentType(mediaFile), mediaFile))
            {
                // updates UI with the new values set in DisplayUpdater
                smtc.DisplayUpdater.Update();
            }
            else
            {
                // ClearAll() updates the UI immediately
                smtc.DisplayUpdater.ClearAll();
            }
            
            if (errorMessage != null)
            {
                await DispatchNotifyUser(errorMessage, NotifyType.ErrorMessage);
            }
        }

        private async Task DispatchNotifyUser(string message, NotifyType notifyType)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                rootPage.NotifyUser(message, notifyType);        
            });
        }
    }
}
