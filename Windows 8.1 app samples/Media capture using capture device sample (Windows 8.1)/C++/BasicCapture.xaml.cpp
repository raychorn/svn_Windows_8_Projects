//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// BasicCapture.xaml.cpp
// Implementation of the BasicCapture class
//

#include "pch.h"
#include "BasicCapture.xaml.h"
#include "ppl.h"

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Platform;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Storage;
using namespace Windows::Media;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Media::Capture;
using namespace Windows::Storage::Streams;
using namespace Windows::System;
using namespace Windows::UI::Xaml::Media::Imaging;

using namespace SDKSample::MediaCapture;
using namespace concurrency;


BasicCapture::BasicCapture()
{
    InitializeComponent();
    ScenarioInit();
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void BasicCapture::OnNavigatedTo(NavigationEventArgs^ e)
{
	// A pointer back to the main page.  This is needed if you want to call methods in MainPage such
	// as NotifyUser()
	rootPage = MainPage::Current;
	SystemMediaTransportControls^ systemMediaControls = SystemMediaTransportControls::GetForCurrentView();
	m_eventRegistrationToken = systemMediaControls->PropertyChanged += ref new TypedEventHandler<SystemMediaTransportControls^, SystemMediaTransportControlsPropertyChangedEventArgs^>(this, &BasicCapture::SystemMediaControlsPropertyChanged);
}

void BasicCapture::OnNavigatedFrom(NavigationEventArgs^ e)
{
	SystemMediaTransportControls^ systemMediaControls = SystemMediaTransportControls::GetForCurrentView();
	systemMediaControls->PropertyChanged -= m_eventRegistrationToken;

    ScenarioClose();

}


void  BasicCapture::ScenarioInit()
{

    btnStartDevice1->IsEnabled = true;
    btnStartPreview1->IsEnabled = false;
    btnStartStopRecord1->IsEnabled = false;
    btnStartStopRecord1->Content = "StartRecord";
    btnTakePhoto1->IsEnabled = false;
    btnTakePhoto1->Content = "TakePhoto";

    m_bRecording = false;
    m_bPreviewing = false;
    m_bSuspended = false;

    previewElement1->Source = nullptr;
    playbackElement1->Source = nullptr;
    imageElement1->Source= nullptr;
    sldBrightness->IsEnabled = false;
    sldContrast->IsEnabled = false;

    previewCanvas1->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void BasicCapture::ScenarioClose()
{
    //TBD
    if (m_bRecording)
    {
        ShowStatusMessage("Stopping Record, Preview and Camera");
        m_bRecording = false;
        create_task(m_mediaCaptureMgr->StopRecordAsync()).then([this](task<void> recordTask)
        {        
            try
            {
                previewElement1->Source = nullptr;
                delete(m_mediaCaptureMgr.Get());
                m_bPreviewing = false;
            }
            catch (Exception ^e)
            {
                ShowExceptionMessage(e);
            }
        });
    }else //!(m_bRecording)
    {
        previewElement1->Source = nullptr;
        if (m_mediaCaptureMgr.Get() != nullptr)
        {
            delete(m_mediaCaptureMgr.Get());
            m_bPreviewing = false;
        }
    }

}

void BasicCapture::SystemMediaControlsPropertyChanged(SystemMediaTransportControls^ sender, SystemMediaTransportControlsPropertyChangedEventArgs^ e)
{
	switch (e->Property)
	{
	case SystemMediaTransportControlsProperty::SoundLevel:
		create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this, sender]()
		{

			if(sender->SoundLevel != SoundLevel::Muted)
			{
				ScenarioInit();
			}
			else
			{
				ScenarioClose();
			}
		})));
		break;

	default:
		break;
	}
}

void BasicCapture::RecordLimitationExceeded(Windows::Media::Capture::MediaCapture ^currentCaptureObject)
{
	try
	{
		if (m_bRecording)
        {
            create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this](){
                try
                {
                    ShowStatusMessage("Stopping Record on exceeding max record duration");
                    btnStartStopRecord1->IsEnabled = false;
                    create_task(m_mediaCaptureMgr->StopRecordAsync()).then([this](task<void> recordTask)
                    {
                        try
                        {
                            recordTask.get();
                            m_bRecording = false;
                            btnStartStopRecord1->Content = "StartRecord";
                            btnStartStopRecord1->IsEnabled = true;
                            ShowStatusMessage("Stopped record on exceeding max record duration:" + m_recordStorageFile->Path);

                            if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                            {
                                //if camera does not support record and Takephoto at the same time
                                //enable TakePhoto button again, after record finished
                                btnTakePhoto1->Content = "TakePhoto";
                                btnTakePhoto1->IsEnabled = true;
                            }

                        }
                        catch (Exception ^e)
                        {
                            ShowExceptionMessage(e);
                            m_bRecording = false;
                            btnStartStopRecord1->Content = "StartRecord";
                            btnStartStopRecord1->IsEnabled = true;
                        }
                    });

                }
                catch (Exception ^e)
                {
                    m_bRecording = false;
                    btnStartStopRecord1->Content = "StartRecord";
                    btnStartStopRecord1->IsEnabled = true;
                    ShowExceptionMessage(e);
                }

            })));
        }
    }
    catch (Exception ^e)
    {
        m_bRecording = false;
        btnStartStopRecord1->Content = "StartRecord";
        btnStartStopRecord1->IsEnabled = true;
        ShowExceptionMessage(e);
    }
}

void BasicCapture::Failed(Windows::Media::Capture::MediaCapture ^currentCaptureObject, Windows::Media::Capture::MediaCaptureFailedEventArgs^ currentFailure)
{
    String ^message = "Fatal error: " + currentFailure->Message;
    create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
        ref new Windows::UI::Core::DispatchedHandler([this, message]()
    {
        ShowStatusMessage(message);
    })));
}

void BasicCapture::btnStartDevice_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        btnStartDevice1->IsEnabled = false;
        ShowStatusMessage("Starting device");
        auto mediaCapture = ref new Windows::Media::Capture::MediaCapture();
        m_mediaCaptureMgr = mediaCapture;
        create_task(mediaCapture->InitializeAsync()).then([this](task<void> initTask)
        {
            try
            {
                initTask.get();

                auto mediaCapture = m_mediaCaptureMgr.Get();
                btnStartPreview1->IsEnabled = true;
                btnStartStopRecord1->IsEnabled = true;
                btnTakePhoto1->IsEnabled = true;

                ShowStatusMessage("Device initialized successful");

                mediaCapture->RecordLimitationExceeded += ref new Windows::Media::Capture::RecordLimitationExceededEventHandler(this, &BasicCapture::RecordLimitationExceeded);
                mediaCapture->Failed += ref new Windows::Media::Capture::MediaCaptureFailedEventHandler(this, &BasicCapture::Failed);

            }
            catch (Exception ^ e)
            {
                ShowExceptionMessage(e);
            }
        }
        );
    }
    catch (Platform::Exception^ e)
    {
        ShowExceptionMessage(e);
    }
}

void BasicCapture::btnStartPreview_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    m_bPreviewing = false;
    try
    {
        ShowStatusMessage("Starting preview");
        btnStartPreview1->IsEnabled = false;
        auto mediaCapture = m_mediaCaptureMgr.Get();

        previewCanvas1->Visibility = Windows::UI::Xaml::Visibility::Visible;
        previewElement1->Source = mediaCapture;
        create_task(mediaCapture->StartPreviewAsync()).then([this](task<void> previewTask)
        {
            try
            {
                previewTask.get();
                auto mediaCapture = m_mediaCaptureMgr.Get();
                m_bPreviewing = true;
                ShowStatusMessage("Start preview successful");
                if(mediaCapture->VideoDeviceController->Brightness)
                {
                    SetupVideoDeviceControl(mediaCapture->VideoDeviceController->Brightness, sldBrightness);
                }
                if(mediaCapture->VideoDeviceController->Contrast)
                {                
                    SetupVideoDeviceControl(mediaCapture->VideoDeviceController->Contrast, sldContrast);
                }

            }catch (Exception ^e)
            {
                ShowExceptionMessage(e);
            }
        });
    }
    catch (Platform::Exception^ e)
    {
        m_bPreviewing = false;
        previewElement1->Source = nullptr;
        btnStartPreview1->IsEnabled = true;
        ShowExceptionMessage(e);
    }
}

void BasicCapture::btnTakePhoto_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        ShowStatusMessage("Taking photo");
        btnTakePhoto1->IsEnabled = false;

        if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
        {
            //if camera does not support record and Takephoto at the same time
            //disable Record button when taking photo
            btnStartStopRecord1 ->IsEnabled = false;
        }

        task<StorageFile^>(KnownFolders::PicturesLibrary->CreateFileAsync(PHOTO_FILE_NAME, Windows::Storage::CreationCollisionOption::GenerateUniqueName)).then([this](task<StorageFile^> getFileTask) 
        {
            try
            {
                this->m_photoStorageFile = getFileTask.get();
                ShowStatusMessage("Create photo file successful");
                ImageEncodingProperties^ imageProperties = ImageEncodingProperties::CreateJpeg();

                create_task(m_mediaCaptureMgr->CapturePhotoToStorageFileAsync(imageProperties, this->m_photoStorageFile)).then([this](task<void> photoTask)
                {
                    try
                    {
                        photoTask.get();
                        btnTakePhoto1->IsEnabled = true;
                        ShowStatusMessage("Photo taken");

                        if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                        {
                            //if camera does not support lowlag record and lowlag photo at the same time
                            //enable Record button after taking photo
                            btnStartStopRecord1 ->IsEnabled = true;
                        }

                        task<IRandomAccessStream^>(this->m_photoStorageFile->OpenAsync(FileAccessMode::Read)).then([this](task<IRandomAccessStream^> getStreamTask)
                        {
                            try
                            {
                                auto photoStream = getStreamTask.get();
                                ShowStatusMessage("File open successful");
                                auto bmpimg = ref new BitmapImage();

                                bmpimg->SetSource(photoStream);
                                imageElement1->Source = bmpimg;
                            }
                            catch (Exception^ e)
                            {
                                ShowExceptionMessage(e);
                                btnTakePhoto1->IsEnabled = true;
                                if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                                {
                                    //if camera does not support record and Takephoto at the same time
                                    //enable Record button at exception
                                    btnStartStopRecord1 ->IsEnabled = true;
                                }
                            }
                        });
                    }
                    catch (Platform::Exception ^ e)
                    {
                        ShowExceptionMessage(e);
                        btnTakePhoto1->IsEnabled = true;
                        if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                        {
                            //if camera does not support record and Takephoto at the same time
                            //enable Record button at exception
                            btnStartStopRecord1 ->IsEnabled = true;
                        }
                    }
                });
            }
            catch (Exception^ e)
            {
                ShowExceptionMessage(e);
                btnTakePhoto1->IsEnabled = true;
                if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                {
                    //if camera does not support record and Takephoto at the same time
                    //enable Record button at exception
                    btnStartStopRecord1 ->IsEnabled = true;
                }
            }
        });
    }
    catch (Platform::Exception^ e)
    {
        ShowExceptionMessage(e);
        btnTakePhoto1->IsEnabled = true;
        if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
        {
            //if camera does not support record and Takephoto at the same time
            //enable Record button at exception
            btnStartStopRecord1 ->IsEnabled = true;
        }
    }
}

void BasicCapture::StartRecord()
{

    ShowStatusMessage("Starting Record");
    String ^fileName;
    fileName = VIDEO_FILE_NAME;

    task<StorageFile^>(KnownFolders::VideosLibrary->CreateFileAsync(fileName,Windows::Storage::CreationCollisionOption::GenerateUniqueName )).then([this](task<StorageFile^> fileTask)
    {
        try
        {
            this->m_recordStorageFile = fileTask.get();
            ShowStatusMessage("Create record file successful");

            MediaEncodingProfile^ recordProfile= nullptr;
            recordProfile = MediaEncodingProfile::CreateMp4(Windows::Media::MediaProperties::VideoEncodingQuality::Auto);

            create_task(m_mediaCaptureMgr->StartRecordToStorageFileAsync(recordProfile, this->m_recordStorageFile)).then([this](task<void> recordTask)
            {
                try
                {
                    recordTask.get();
                    m_bRecording = true;
                    btnStartStopRecord1->Content = "StopRecord";
                    btnStartStopRecord1->IsEnabled = true;

                    ShowStatusMessage("Start Record successful");
                }
                catch (Exception ^e)
                {
                    ShowExceptionMessage(e);
                }
            });
        }
        catch (Exception ^e)
        {
            m_bRecording = false;
            btnStartStopRecord1->Content = "StopRecord";
            btnStartStopRecord1->IsEnabled = true;
            ShowExceptionMessage(e);
        }
    }
    );
}

void BasicCapture::btnStartStopRecord_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        btnStartStopRecord1->IsEnabled = false;
        playbackElement1->Source = nullptr;
        if (safe_cast<String^>(btnStartStopRecord1->Content) == "StartRecord")
        {
            if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
            {
                //if camera does not support record and Takephoto at the same time
                //disable TakePhoto button when recording
                btnTakePhoto1->IsEnabled = false;
            }
            StartRecord();
        }
        else //(safe_cast<String^>(btnStartStopRecord1->Content) == "StopRecord")
        {
            ShowStatusMessage("Stopping Record");
            create_task(m_mediaCaptureMgr->StopRecordAsync()).then([this](task<void> recordTask)
            {
                try
                {
                    recordTask.get();
                    m_bRecording = false;
                    btnStartStopRecord1->IsEnabled = true;
                    btnStartStopRecord1->Content = "StartRecord";

                    if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                    {
                        //if camera does not support lowlag record and lowlag photo at the same time
                        //enable TakePhoto button after recording
                        btnTakePhoto1->IsEnabled = true;
                    }

                    ShowStatusMessage("Stop record successful");
                    if (!m_bSuspended)
                    {
                        task<IRandomAccessStream^>(this->m_recordStorageFile->OpenAsync(FileAccessMode::Read)).then([this](task<IRandomAccessStream^> streamTask)
                        {
                            try
                            {
                                auto stream = streamTask.get();
                                ShowStatusMessage("Record file opened");
                                ShowStatusMessage(this->m_recordStorageFile->Path);
                                playbackElement1->AutoPlay = true;
                                playbackElement1->SetSource(stream, this->m_recordStorageFile->FileType);
                                playbackElement1->Play();
                            }
                            catch (Exception ^e)
                            {
                                ShowExceptionMessage(e);
                                m_bRecording = false;
                                btnStartStopRecord1->IsEnabled = true;
                                btnStartStopRecord1->Content = "StartRecord";
                                if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                                {
                                    //if camera does not support lowlag record and lowlag photo at the same time
                                    //enable TakePhoto button at exception
                                    btnTakePhoto1->IsEnabled = true;
                                }
                            }
                        });
                    }
                }
                catch (Exception ^e)
                {
                    m_bRecording = false;
                    btnStartStopRecord1->IsEnabled = true;
                    btnStartStopRecord1->Content = "StartRecord";
                    ShowExceptionMessage(e);
                    if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
                    {
                        //if camera does not support lowlag record and lowlag photo at the same time
                        //enable TakePhoto button at exception
                        btnTakePhoto1->IsEnabled = true;
                    }
                }
            });
        }
    }
    catch (Platform::Exception^ e)
    {
        ShowExceptionMessage(e);
        btnStartStopRecord1->IsEnabled = true;
        btnStartStopRecord1->Content = "StartRecord";
        m_bRecording = false;
        if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported) 
        {
            //if camera does not support lowlag record and lowlag photo at the same time
            //enable TakePhoto button at exception
            btnTakePhoto1->IsEnabled = true;
        }
    }
}

void BasicCapture::SetupVideoDeviceControl(Windows::Media::Devices::MediaDeviceControl^ videoDeviceControl, Slider^ slider)
{
    try
    {		
        if ((videoDeviceControl->Capabilities)->Supported)
        {
            slider->IsEnabled = true;
            slider->Maximum = videoDeviceControl->Capabilities->Max;
            slider->Minimum = videoDeviceControl->Capabilities->Min;
            slider->StepFrequency = videoDeviceControl->Capabilities->Step;
            double controlValue = 0;
            if (videoDeviceControl->TryGetValue(&controlValue))
            {
                slider->Value = controlValue;
            }
        }
        else
        {
            slider->IsEnabled = false;
        }
    }
    catch (Platform::Exception^ e)
    {
        ShowExceptionMessage(e);
    }
}

// VideoDeviceControllers
void BasicCapture::sldBrightness_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
    bool succeeded = m_mediaCaptureMgr->VideoDeviceController->Brightness->TrySetValue(sldBrightness->Value);
    if (!succeeded)
    {
        ShowStatusMessage("Set Brightness failed");
    }
}

void BasicCapture::sldContrast_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs ^e)
{
    bool succeeded = m_mediaCaptureMgr->VideoDeviceController->Contrast->TrySetValue(sldContrast->Value);
    if (!succeeded)
    {
        ShowStatusMessage("Set Contrast failed");
    }
}

void BasicCapture::ShowStatusMessage(Platform::String^ text)
{
    rootPage->NotifyUser(text, NotifyType::StatusMessage);
}

void BasicCapture::ShowExceptionMessage(Platform::Exception^ ex)
{
    rootPage->NotifyUser(ex->Message, NotifyType::ErrorMessage);
}




