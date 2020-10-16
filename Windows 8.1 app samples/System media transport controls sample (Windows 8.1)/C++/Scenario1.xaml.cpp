//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// Scenario1.xaml.cpp
// Implementation of the Scenario1 class
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Scenario1.xaml.h"

using namespace SDKSample;
using namespace SDKSample::MediaTransportControls;

using namespace Platform;
using namespace Concurrency;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Media;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;

Scenario1::Scenario1()
{
    InitializeComponent();
    isInitialized = false;
    isPlaying = false;
    isChangingMedia = false;
    currentIndex = 0;
    smtc = nullptr;
    playlist = nullptr;
}

void Scenario1::OnNavigatedTo(NavigationEventArgs^ e)
{
    rootPage = MainPage::Current;

    if (isInitialized)
    {
        return;
    }

    smtc = Windows::Media::SystemMediaTransportControls::GetForCurrentView();
    smtc->PlaybackStatus = Windows::Media::MediaPlaybackStatus::Closed;
    smtc->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls^,SystemMediaTransportControlsButtonPressedEventArgs^>(
        this,
        &Scenario1::smtc_ButtonPressed
        );
    smtc->IsPlayEnabled = false;
    smtc->IsPauseEnabled = false;
    smtc->IsStopEnabled = false;

    MyMediaElement->CurrentStateChanged += ref new RoutedEventHandler(this, &Scenario1::MyMediaElement_CurrentStateChanged);
    MyMediaElement->MediaOpened += ref new RoutedEventHandler(this, &Scenario1::MyMediaElement_MediaOpened);
    MyMediaElement->MediaEnded += ref new RoutedEventHandler(this, &Scenario1::MyMediaElement_MediaEnded);
    MyMediaElement->MediaFailed += ref new ExceptionRoutedEventHandler(this, &Scenario1::MyMediaElement_MediaFailed);

    rootPage->NotifyUser("events registered", NotifyType::StatusMessage);
    isInitialized = true;
}

void Scenario1::SelectFilesButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    // supported audio and video formats for Windows Store apps:
    // http://msdn.microsoft.com/en-us/library/windows/apps/hh986969.aspx
    static const char16* const supportedAudioFormats [] =
    {
        L".3g2", L".3gp2", L".3gp", L".3gpp", L".m4a", L".asf", L".wma", L".aac", L".adt", L".adts", L".mp3", L".wav", L".ac3", L".ec3",
    };
    static const char16* const supportedVideoFormats [] =
    {
        L".3g2", L".3gp2", L".3gp", L".3gpp", L".m4v", L".mp4v", L".mp4", L".mov", L".m2ts", L".asf", L".wmv", L".avi",
    };

    FileOpenPicker^ filePicker = ref new FileOpenPicker();
    filePicker->ViewMode = PickerViewMode::List;
    filePicker->SuggestedStartLocation = PickerLocationId::MusicLibrary;
    for (const auto &fileExtension : supportedAudioFormats)
    {
        filePicker->FileTypeFilter->Append(Platform::StringReference(fileExtension));
    }
    for (const auto &fileExtension : supportedVideoFormats)
    {
        filePicker->FileTypeFilter->Append(Platform::StringReference(fileExtension));
    }

    create_task(filePicker->PickMultipleFilesAsync()).then([this](IVectorView<StorageFile^>^ selectedFiles)
    {
        if (selectedFiles->Size > 0)
        {
            rootPage->NotifyUser(selectedFiles->Size + " file(s) selected", NotifyType::StatusMessage);
            currentIndex = 0;
            playlist = selectedFiles;
            isPlaying = true;
            SetCurrentPlayingAsync(currentIndex);
        }
        else
        {
            // user canceled the file picker
            rootPage->NotifyUser("no files selected", NotifyType::StatusMessage);
        }
    });
}

void Scenario1::PlayPauseButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    if (MyMediaElement->CurrentState == MediaElementState::Playing)
    {
        MyMediaElement->Pause();
    }
    else
    {
        MyMediaElement->Play();
    }
}

void Scenario1::MyMediaElement_MediaOpened(Object^ sender, RoutedEventArgs^ e)
{
    isChangingMedia = false;
    if (isPlaying)
    {
        MyMediaElement->Play();
    }
}

void Scenario1::MyMediaElement_MediaEnded(Object^ sender, RoutedEventArgs^ e)
{
    if (currentIndex < int(playlist->Size - 1))
    {
        isPlaying = true;
        currentIndex++;
        SetCurrentPlayingAsync(currentIndex);
    }
}

void Scenario1::MyMediaElement_MediaFailed(Object^ sender, ExceptionRoutedEventArgs^ e)
{
    isChangingMedia = false;
    Platform::String^ errorMessage = "Cannot play " + playlist->GetAt(currentIndex)->Name +
        ". Press Next or Previous to continue, or select new files to play.";
    rootPage->NotifyUser(errorMessage, NotifyType::ErrorMessage);
}

void Scenario1::MyMediaElement_CurrentStateChanged(Object^ sender, RoutedEventArgs^ e)
{
    if (MyMediaElement->CurrentState == MediaElementState::Playing)
    {
        isPlaying = true;
        PlayPauseButton->Content = "Pause";
    }
    else if (!isChangingMedia &&
        (MyMediaElement->CurrentState == MediaElementState::Stopped || MyMediaElement->CurrentState == MediaElementState::Paused))
    {
        isPlaying = false;
        PlayPauseButton->Content = "Play";
    }

    switch (MyMediaElement->CurrentState)
    {
    case MediaElementState::Closed:
        smtc->PlaybackStatus = MediaPlaybackStatus::Closed;
        break;

    case MediaElementState::Opening:
        smtc->PlaybackStatus = MediaPlaybackStatus::Changing;
        break;

    case MediaElementState::Buffering:
        // no changes in MediaPlaybackStatus
        break;

    case MediaElementState::Paused:
        smtc->PlaybackStatus = MediaPlaybackStatus::Paused;
        break;

    case MediaElementState::Playing:
        smtc->PlaybackStatus = MediaPlaybackStatus::Playing;
        break;

    case MediaElementState::Stopped:
        smtc->PlaybackStatus = MediaPlaybackStatus::Stopped;
        break;
    }
}

void Scenario1::smtc_ButtonPressed(SystemMediaTransportControls^ sender, SystemMediaTransportControlsButtonPressedEventArgs^ args)
{
    switch (args->Button)
    {
    case SystemMediaTransportControlsButton::Next:
        DispatchNotifyUser("Next pressed", NotifyType::StatusMessage);
        currentIndex++;
        SetCurrentPlayingAsync(currentIndex);
        break;

    case SystemMediaTransportControlsButton::Previous:
        DispatchNotifyUser("Previous pressed", NotifyType::StatusMessage);
        currentIndex--;
        SetCurrentPlayingAsync(currentIndex);
        break;

    case SystemMediaTransportControlsButton::Play:
        Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
        {
            rootPage->NotifyUser("Play pressed", NotifyType::StatusMessage);
            MyMediaElement->Play();
        }));
        break;

    case SystemMediaTransportControlsButton::Pause:
        Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
        {
            rootPage->NotifyUser("Pause pressed", NotifyType::StatusMessage);
            MyMediaElement->Pause();
        }));
        break;

    case SystemMediaTransportControlsButton::Stop:
        Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
        {
            rootPage->NotifyUser("Stop pressed", NotifyType::StatusMessage);
            MyMediaElement->Stop();
        }));
        break;

        // Insert additional case statements for other buttons you want to handle in your app.
        // Remember that you also need to first enable those buttons via the corresponding
        // IsXXXXEnabled property on the SystemMediaTransportControls object.
    }
}

Windows::Media::MediaPlaybackType Scenario1::GetMediaTypeFromFileContentType(StorageFile^ file)
{
    Windows::Media::MediaPlaybackType mediaPlaybackType = MediaPlaybackType::Unknown;
    Platform::String^ fileMimeType = file->ContentType;
    Platform::String^ mimeTypeAudio = "audio/";
    Platform::String^ mimeTypeVideo = "video/";
    Platform::String^ mimeTypeImage = "image/";

    if (-1 != FindStringOrdinal(FIND_STARTSWITH, fileMimeType->Data(), fileMimeType->Length(),
        mimeTypeAudio->Data(), mimeTypeAudio->Length(), TRUE))
    {
        mediaPlaybackType = MediaPlaybackType::Music;
    }
    else if (-1 != FindStringOrdinal(FIND_STARTSWITH, fileMimeType->Data(), fileMimeType->Length(),
        mimeTypeVideo->Data(), mimeTypeVideo->Length(), TRUE))
    {
        mediaPlaybackType = MediaPlaybackType::Video;
    }
    else if (-1 != FindStringOrdinal(FIND_STARTSWITH, fileMimeType->Data(), fileMimeType->Length(),
        mimeTypeImage->Data(), mimeTypeImage->Length(), TRUE))
    {
        mediaPlaybackType = MediaPlaybackType::Image;
    }

    return mediaPlaybackType;
}

void Scenario1::SetCurrentPlayingAsync(int playlistIndex)
{
    if (playlistIndex <= 0)
    {
        smtc->IsPreviousEnabled = false;
        playlistIndex = 0;
    }
    else
    {
        smtc->IsPreviousEnabled = true;
    }

    if (playlistIndex >= int(playlist->Size - 1))
    {
        smtc->IsNextEnabled = false;
        playlistIndex = playlist->Size - 1;
    }
    else
    {
        smtc->IsNextEnabled = true;
    }

    try
    {
        StorageFile^ mediaFile = playlist->GetAt(playlistIndex);
        isChangingMedia = true;

        create_task(mediaFile->OpenAsync(FileAccessMode::Read)).then([this, mediaFile](Streams::IRandomAccessStream^ stream)
        {
            Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, mediaFile, stream]()
            {
                MyMediaElement->SetSource(stream, mediaFile->ContentType);
                PlayPauseButton->IsEnabled = true;
                smtc->IsPlayEnabled = true;
                smtc->IsPauseEnabled = true;
                smtc->IsStopEnabled = true;
            }));

            create_task(smtc->DisplayUpdater->CopyFromFileAsync(GetMediaTypeFromFileContentType(mediaFile), mediaFile)).then([this](bool success)
            {
                if (success)
                {
                    // updates UI with the new values set in DisplayUpdater
                    smtc->DisplayUpdater->Update();
                }
                else
                {
                    // ClearAll() updates the UI immediately
                    smtc->DisplayUpdater->ClearAll();
                }
            });
        });
    }
    catch (Exception^ e)
    {
        isChangingMedia = false;
        DispatchNotifyUser(e->Message, NotifyType::ErrorMessage);
    }
}

void Scenario1::DispatchNotifyUser(Platform::String^ message, NotifyType notifyType)
{
    Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]()
    {
        rootPage->NotifyUser(message, notifyType);
    }));
}
