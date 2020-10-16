//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// Scenario1.xaml.h
// Declaration of the Scenario1 class
//

#pragma once
#include "Scenario1.g.h"

namespace SDKSample
{
    namespace MediaTransportControls
    {
        /// <summary>
        /// An empty page that can be used on its own or navigated to within a Frame.
        /// </summary>
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class Scenario1 sealed
        {
        public:
            Scenario1();

        protected:
            virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

        private:
            void SelectFilesButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
            void PlayPauseButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

            void MyMediaElement_MediaOpened(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
            void MyMediaElement_MediaEnded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
            void MyMediaElement_MediaFailed(Platform::Object^ sender, Windows::UI::Xaml::ExceptionRoutedEventArgs^ e);
            void MyMediaElement_CurrentStateChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

            void smtc_ButtonPressed(
                Windows::Media::SystemMediaTransportControls^ sender,
                Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^ args
                );
            Windows::Media::MediaPlaybackType GetMediaTypeFromFileContentType(Windows::Storage::StorageFile^ file);
            void SetCurrentPlayingAsync(int playlistIndex);

            void DispatchNotifyUser(Platform::String^ message, SDKSample::NotifyType notifyType);

            MainPage^ rootPage;
            bool isInitialized;
            Windows::Media::SystemMediaTransportControls^ smtc;
            Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFile^>^ playlist;
            unsigned int currentIndex;
            bool isPlaying;
            bool isChangingMedia;
        };
    }
}
