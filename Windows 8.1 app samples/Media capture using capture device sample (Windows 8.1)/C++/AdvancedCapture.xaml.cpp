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
// AdvancedCapture.xaml.cpp
// Implementation of the AdvancedCapture class
//

#include "pch.h"
#include "AdvancedCapture.xaml.h"

using namespace SDKSample::MediaCapture;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Platform;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Storage;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Storage::Streams;
using namespace Windows::System;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Media;
using namespace Windows::Media::Capture;

ref class ReencodeState sealed
{
public:
	ReencodeState()
	{
	}

	virtual ~ReencodeState()
	{
		if (InputStream != nullptr)
		{
			delete InputStream;
		}
		if (OutputStream != nullptr)
		{
			delete OutputStream;
		}
	}

internal:
	Windows::Storage::Streams::IRandomAccessStream ^InputStream;
	Windows::Storage::Streams::IRandomAccessStream ^OutputStream;
	Windows::Storage::StorageFile ^PhotoStorage;
	Windows::Graphics::Imaging::BitmapDecoder ^Decoder;
	Windows::Graphics::Imaging::BitmapEncoder ^Encoder;
};

AdvancedCapture::AdvancedCapture()
{
	InitializeComponent();
	ScenarioInit();
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void AdvancedCapture::OnNavigatedTo(NavigationEventArgs^ e)
{
    // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
    // as NotifyUser()
    rootPage = MainPage::Current;
	SystemMediaTransportControls^ systemMediaControls = SystemMediaTransportControls::GetForCurrentView();
	m_eventRegistrationToken = systemMediaControls->PropertyChanged += ref new TypedEventHandler<SystemMediaTransportControls^, SystemMediaTransportControlsPropertyChangedEventArgs^>(this, &AdvancedCapture::SystemMediaControlsPropertyChanged);

    Windows::Graphics::Display::DisplayInformation^ displayInfo =
        Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
    m_displayOrientation = displayInfo->CurrentOrientation;
    m_orientationChangedEventToken = displayInfo->OrientationChanged += ref new TypedEventHandler<Windows::Graphics::Display::DisplayInformation^, Platform::Object^>(this, &AdvancedCapture::DisplayInfo_OrientationChanged);
}

void AdvancedCapture::OnNavigatedFrom(NavigationEventArgs^ e)
{
	SystemMediaTransportControls^ systemMediaControls = SystemMediaTransportControls::GetForCurrentView();
	systemMediaControls->PropertyChanged -= m_eventRegistrationToken;
    Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->OrientationChanged  -= m_orientationChangedEventToken;
    ScenarioClose();
}

void  AdvancedCapture::ScenarioInit()
{
    rootPage = MainPage::Current;
    btnStartDevice2->IsEnabled = true;
    btnStartPreview2->IsEnabled = false;
    btnStartStopRecord2->IsEnabled = false;
    btnStartStopRecord2->Content = "StartRecord";
    btnTakePhoto2->IsEnabled = false;
    btnTakePhoto2->Content = "TakePhoto";

    m_bRecording = false;
    m_bPreviewing = false;
    m_bEffectAdded = false;
    m_bSuspended = false;
    m_bLowLagPrepared = false;
    chkAddRemoveEffect->IsChecked = false;
    chkAddRemoveEffect->IsEnabled = false;
    radTakePhoto->IsEnabled = false;
    radRecord->IsEnabled = false;

    previewElement2->Source = nullptr;
    playbackElement2->Source = nullptr;
    imageElement2->Source= nullptr;

    ShowStatusMessage("");

    previewCanvas2->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    EnumerateWebcamsAsync();
    EnumerateMicrophonesAsync();

    SceneModeList2->SelectedIndex = -1;
    SceneModeList2->Items->Clear();
}

void AdvancedCapture::ScenarioClose()
{
    //TBD when bug 252131 fix check in, replace this with only delete(m_mediaCaptureMgr.Get()
    if (m_bLowLagPrepared)
    {
        ShowStatusMessage("Stopping LowLagPhoto");
        m_bLowLagPrepared = false;
        create_task(m_lowLagPhoto->FinishAsync()).then([this](task<void> Task)
        {
            if (m_bRecording)
            {
                ShowStatusMessage("Stopping LowLagRecord");
                m_bRecording = false;
                try
                {
                    create_task(m_lowLagRecord->FinishAsync()).then([this](task<void> Task)
                    {
                        m_lowLagRecord  = nullptr;
                        ShowStatusMessage("Stopping Preview and Camera");
                        previewElement2->Source = nullptr;
                        delete(m_mediaCaptureMgr.Get());
                        m_bPreviewing = false;
                    });
                    if (safe_cast<String^>(btnStartStopRecord2->Content) == "StartRecord")
                    {
                        m_recordStorageFile->DeleteAsync();
                    }
                }
                catch (Exception ^e)
                {
                    ShowExceptionMessage(e);
                }
            }else // (!m_bRecording)
            {
                ShowStatusMessage("Stopping Preview and Camera");
                previewElement2->Source = nullptr;
                delete(m_mediaCaptureMgr.Get());
                m_bPreviewing = false;
            }
        });
    }else //!m_bLowLagPrepared
    {
        if (m_bRecording)
        {
            ShowStatusMessage("Stopping LowLagRecord");
            m_bRecording = false;
            try
            {
                create_task(m_lowLagRecord->FinishAsync()).then([this](task<void> Task)
                {
                    m_lowLagRecord  = nullptr;
                    ShowStatusMessage("Stopping Preview and Camera");
                    previewElement2->Source = nullptr;
                    delete(m_mediaCaptureMgr.Get());
                    m_bPreviewing = false;
                });
                if (safe_cast<String^>(btnStartStopRecord2->Content) == "StartRecord")
                {
                    m_recordStorageFile->DeleteAsync();
                }
            }
            catch (Exception ^e)
            {
                ShowExceptionMessage(e);
            }
        }else //!m_bRecording
        {
            ShowStatusMessage("Stopping Preview and Camera");
            previewElement2->Source = nullptr;
            if (m_mediaCaptureMgr.Get() != nullptr)
            {
                delete(m_mediaCaptureMgr.Get());
                m_bPreviewing = false;
            }
        }
    }
}

void AdvancedCapture::SystemMediaControlsPropertyChanged(SystemMediaTransportControls^ sender, SystemMediaTransportControlsPropertyChangedEventArgs^ e)
{
	switch (e->Property)
	{
	case SystemMediaTransportControlsProperty::SoundLevel:
		create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this, sender]()
		{

			if (sender->SoundLevel != SoundLevel::Muted)
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


void AdvancedCapture::RecordLimitationExceeded(Windows::Media::Capture::MediaCapture ^currentCaptureObject)
{
    try
    {
        if (safe_cast<String^>(btnStartStopRecord2->Content) == "StopRecord")
        {
            create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this]()
            {
                try
                {
                    ShowStatusMessage("Stopping Record on exceeding max record duration");
                    btnStartStopRecord2->IsEnabled = false;
                    create_task(m_lowLagRecord->FinishAsync()).then([this](task<void> recordTask)
                    {
                        try
                        {
                            recordTask.get();
                            m_bRecording = false;
                            ShowStatusMessage("Stopped record on exceeding max record duration:" + m_recordStorageFile->Path);

                            if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                            {
                                //if camera does not support lowlag record and lowlag photo at the same time
                                //enable the checkbox
                                radTakePhoto->IsEnabled = true;
                                radRecord->IsEnabled = true;
                            }

                            //Prepare lowlag record for next round;
                            PrepareLowLagRecordAsync();

                        }
                        catch (Exception ^e)
                        {
                            ShowExceptionMessage(e);
                        }
                    });
                }
                catch (Exception ^e)
                {
                    ShowExceptionMessage(e);
                }
            })));
        }
    }
    catch (Exception ^e)
    {
        ShowExceptionMessage(e);
    }
}

void AdvancedCapture::Failed(Windows::Media::Capture::MediaCapture ^currentCaptureObject, Windows::Media::Capture::MediaCaptureFailedEventArgs^ currentFailure)
{
	String ^message = "Fatal error" + currentFailure->Message;
	create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
		ref new Windows::UI::Core::DispatchedHandler([this, message]()
	{
		ShowStatusMessage(message);
	})));
}

void AdvancedCapture::PrepareLowLagRecordAsync()
{
    PrepareForVideoRecording();

    task<StorageFile^>(KnownFolders::VideosLibrary->CreateFileAsync(VIDEO_FILE_NAME, Windows::Storage::CreationCollisionOption::GenerateUniqueName)).then(
        [this](task<StorageFile^> fileTask)
    {
        this->m_recordStorageFile = fileTask.get();
        ShowStatusMessage("Create record file successful");

		MediaEncodingProfile^ recordProfile = nullptr;
		recordProfile = MediaEncodingProfile::CreateMp4(Windows::Media::MediaProperties::VideoEncodingQuality::Auto);

        create_task(m_mediaCaptureMgr->PrepareLowLagRecordToStorageFileAsync(recordProfile, m_recordStorageFile)).then(
            [this](LowLagMediaRecording ^lowLagRecord)
        {
            m_lowLagRecord = lowLagRecord;
            btnStartStopRecord2->IsEnabled = true;
            m_bRecording = true;
            btnStartStopRecord2->Content = "StartRecord";
        });
    });
}

void AdvancedCapture::btnStartDevice_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        btnStartDevice2->IsEnabled = false;
        ShowStatusMessage("Starting device");
        auto mediaCapture = ref new Windows::Media::Capture::MediaCapture();
        m_mediaCaptureMgr = mediaCapture;
        auto settings = ref new Windows::Media::Capture::MediaCaptureInitializationSettings();
        auto chosenDevInfo = m_devInfoCollection->GetAt(EnumedDeviceList2->SelectedIndex);
        settings->VideoDeviceId = chosenDevInfo->Id;

		if (EnumedMicrophonesList2->SelectedIndex >= 0 && m_microPhoneInfoCollection->Size > 0)
		{
			auto chosenMicrophoneInfo = m_microPhoneInfoCollection->GetAt(EnumedMicrophonesList2->SelectedIndex);
			settings->AudioDeviceId = chosenMicrophoneInfo->Id;
		}

		if (chosenDevInfo->EnclosureLocation != nullptr && chosenDevInfo->EnclosureLocation->Panel == Windows::Devices::Enumeration::Panel::Back)
		{
			m_bRotateVideoOnOrientationChange = true;
			m_bReversePreviewRotation = false;
		}
		else if (chosenDevInfo->EnclosureLocation != nullptr && chosenDevInfo->EnclosureLocation->Panel == Windows::Devices::Enumeration::Panel::Front)
		{
			m_bRotateVideoOnOrientationChange = true;
			m_bReversePreviewRotation = true;
		}
		else
		{
			m_bRotateVideoOnOrientationChange = false;
		}

		create_task(mediaCapture->InitializeAsync(settings)).then([this](task<void> initTask)
		{
			try
			{
				initTask.get();

				auto mediaCapture = m_mediaCaptureMgr.Get();

                OrientationChanged();

                btnStartPreview2->IsEnabled = true;
                ShowStatusMessage("Device initialized successful");
                chkAddRemoveEffect->IsEnabled = true;
                mediaCapture->RecordLimitationExceeded += ref new Windows::Media::Capture::RecordLimitationExceededEventHandler(this, &AdvancedCapture::RecordLimitationExceeded);
                mediaCapture->Failed += ref new Windows::Media::Capture::MediaCaptureFailedEventHandler(this, &AdvancedCapture::Failed);

                if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                {
                    radTakePhoto->IsEnabled = true;
                    radRecord->IsEnabled = true;
                    //choose TakePhoto Mode as default
                    radTakePhoto->IsChecked = true;
                }
                else
                {
                    //prepare lowlag photo, then prepare lowlag record
                    create_task(m_mediaCaptureMgr->PrepareLowLagPhotoCaptureAsync(ImageEncodingProperties::CreateJpeg())).then([this](LowLagPhotoCapture ^photoCapture)
                    {
                        m_lowLagPhoto = photoCapture;
                        btnTakePhoto2->IsEnabled = true;
                        m_bLowLagPrepared = true;
                        PrepareLowLagRecordAsync();
                    });
                    //disable check options
                    radTakePhoto->IsEnabled = false;
                    radRecord->IsEnabled = false;
                }

				EnumerateSceneModeAsync();
			}
			catch (Exception ^ e)
			{
				ShowExceptionMessage(e);
			}
		});
	}
	catch (Platform::Exception^ e)
	{
		ShowExceptionMessage(e);
	}
}

void AdvancedCapture::btnStartPreview_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    m_bPreviewing = false;
    try
    {
        ShowStatusMessage("Starting preview");
        btnStartPreview2->IsEnabled = false;

        auto mediaCapture = m_mediaCaptureMgr.Get();
        previewCanvas2->Visibility = Windows::UI::Xaml::Visibility::Visible;
        previewElement2->Source = mediaCapture;
        create_task(mediaCapture->StartPreviewAsync()).then([this](task<void> previewTask)
        {
            try
            {
                previewTask.get();
                m_bPreviewing = true;
                ShowStatusMessage("Start preview successful");
            }
            catch (Exception ^e)
            {
                ShowExceptionMessage(e);
            }
        });
    }
    catch (Platform::Exception^ e)
    {
        m_bPreviewing = false;
        previewElement2->Source = nullptr;
        btnStartPreview2->IsEnabled = true;
        ShowExceptionMessage(e);
    }
}

void AdvancedCapture::btnTakePhoto_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        ShowStatusMessage("Taking photo");
        btnTakePhoto2->IsEnabled = false;

        if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
        {
            //disable check box while taking photo
            radTakePhoto->IsEnabled = false;
            radRecord->IsEnabled = false;
        }

		create_task(m_lowLagPhoto->CaptureAsync()).then([this](CapturedPhoto ^photo)
		{

			auto currentRotation = GetCurrentPhotoRotation();

			ReencodePhotoAsync(photo->Frame->CloneStream(), currentRotation).then([this] (task<StorageFile^> reencodeImageTask)
			{
				try
				{
					auto photoStorageFile = reencodeImageTask.get();

                    btnTakePhoto2->IsEnabled = true;
                    ShowStatusMessage("Photo taken");

					task<IRandomAccessStream^>(photoStorageFile->OpenAsync(FileAccessMode::Read)).then([this](task<IRandomAccessStream^> getStreamTask)
					{
						try
						{
							auto photoStream = getStreamTask.get();
							ShowStatusMessage("File open successful");
							auto bmpimg = ref new BitmapImage();

                            bmpimg->SetSource(photoStream);
                            imageElement2->Source = bmpimg;

                            if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                            {
                                //reset check options
                                radTakePhoto->IsEnabled = true;
                                radTakePhoto->IsChecked = true;
                                radRecord->IsEnabled = true;
                            }
                        }
                        catch (Exception^ e)
                        {
                            ShowExceptionMessage(e);
                            btnTakePhoto2->IsEnabled = true;
                            if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                            {
                                //enable check box at exception
                                radTakePhoto->IsEnabled = true;
                                radRecord->IsEnabled = true;
                            }
                        }
                    });
                }
                catch (Platform::Exception ^ e)
                {
                    ShowExceptionMessage(e);
                    btnTakePhoto2->IsEnabled = true;
                    if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                    {
                        //enable check box at exception
                        radTakePhoto->IsEnabled = true;
                        radRecord->IsEnabled = true;
                    }
                }
            });
        });
    }
    catch (Platform::Exception^ e)
    {
        ShowExceptionMessage(e);
        btnTakePhoto2->IsEnabled = true;
        if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
        {
            //enable check box at exception
            radTakePhoto->IsEnabled = true;
            radRecord->IsEnabled = true;
        }
    }

}

void AdvancedCapture::btnStartStopRecord_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        if (safe_cast<String^>(btnStartStopRecord2->Content) == "StartRecord")
        {
            if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
            {
                //disable check box while recording
                radTakePhoto->IsEnabled = false;
                radRecord->IsEnabled = false;
            }

            btnStartStopRecord2->IsEnabled = false;
            ShowStatusMessage("Starting Record");
            create_task(m_lowLagRecord->StartAsync()).then([this]()
            {
                btnStartStopRecord2->Content = "StopRecord";
                btnStartStopRecord2->IsEnabled = true;
                playbackElement2->Source = nullptr;
            });
        }
        else
        {
            ShowStatusMessage("Stopping Record");
            btnStartStopRecord2->IsEnabled = false;
            create_task(m_lowLagRecord->FinishAsync()).then([this](task<void> recordTask)
            {
                try
                {
                    recordTask.get();
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
                                playbackElement2->AutoPlay = true;
                                playbackElement2->SetSource(stream, this->m_recordStorageFile->FileType);
                                playbackElement2->Play();
                            }
                            catch (Exception ^e)
                            {
                                btnStartStopRecord2->Content = "StartRecord";
                                btnStartStopRecord2->IsEnabled = true;
                                ShowExceptionMessage(e);
                                if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                                {
                                    //enable check box at exception
                                    radTakePhoto->IsEnabled = true;
                                    radRecord->IsEnabled = true;
                                }
                            }
                        });
                    }

                    if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                    {
                        //reset check options
                        radTakePhoto->IsEnabled = true;
                        radRecord->IsEnabled = true;
                        radRecord->IsChecked = true;
                    }
                    //prepare lowlag record for next round
                    m_bRecording = false;
                    PrepareLowLagRecordAsync();

                }
                catch (Exception ^e)
                {
                    btnStartStopRecord2->Content = "StartRecord";
                    btnStartStopRecord2->IsEnabled = true;
                    ShowExceptionMessage(e);
                    if (!m_mediaCaptureMgr->MediaCaptureSettings->ConcurrentRecordAndPhotoSupported)
                    {
                        //enable check box at exception
                        radTakePhoto->IsEnabled = true;
                        radRecord->IsEnabled = true;
                    }
                }
            });
        }
    }
    catch (Platform::Exception^ e)
    {
        ShowExceptionMessage(e);
    }
}
void AdvancedCapture::lstEnumedDevices_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	ScenarioClose();

    btnStartDevice2->IsEnabled = true;
    btnStartPreview2->IsEnabled = false;
    btnStartStopRecord2->IsEnabled = false;
    btnStartStopRecord2->Content = "StartRecord";
    btnTakePhoto2->IsEnabled = false;

    m_bRecording = false;
    m_bPreviewing = false;
    m_bEffectAdded = false;
    m_bSuspended = false;
    m_bLowLagPrepared = false;

    chkAddRemoveEffect->IsEnabled = false;
    chkAddRemoveEffect->IsChecked = false;
    radTakePhoto->IsEnabled = false;
    radRecord->IsEnabled = false;

    previewElement2->Source = nullptr;
    playbackElement2->Source = nullptr;
    imageElement2->Source= nullptr;

    m_bEffectAddedToRecord = false;
    m_bEffectAddedToPhoto = false;
    SceneModeList2->SelectedIndex = -1;
    SceneModeList2->Items->Clear();
    ShowStatusMessage("Device changed, Initialize");
}

void AdvancedCapture::EnumerateWebcamsAsync()
{
	try
	{
		ShowStatusMessage("Enumerating Webcams...");
		m_devInfoCollection = nullptr;

		EnumedDeviceList2->Items->Clear();

		task<DeviceInformationCollection^>(DeviceInformation::FindAllAsync(DeviceClass::VideoCapture)).then([this](task<DeviceInformationCollection^> findTask)
			//task<DeviceInformationCollection^>(Windows::Media::Capture::MediaCaptureInitializationSettings::GetVideoDevices()).then([this](task<DeviceInformationCollection^> findTask)
		{
			try
			{
				m_devInfoCollection = findTask.get();
				if (m_devInfoCollection == nullptr || m_devInfoCollection->Size == 0)
				{
					ShowStatusMessage("No WebCams found.");
				}
				else
				{
					for (unsigned int i = 0; i < m_devInfoCollection->Size; i++)
					{
						auto devInfo = m_devInfoCollection->GetAt(i);
						auto location = devInfo->EnclosureLocation;

						if (location != nullptr)
						{

							if (location->Panel == Windows::Devices::Enumeration::Panel(1))
							{
								EnumedDeviceList2->Items->Append(devInfo->Name + "-Front");
							}
							else if (location->Panel == Windows::Devices::Enumeration::Panel(1))
							{
								EnumedDeviceList2->Items->Append(devInfo->Name + "-Back");
							}
							else{
								EnumedDeviceList2->Items->Append(devInfo->Name);
							}
						}
						else{
							EnumedDeviceList2->Items->Append(devInfo->Name);
						}
					}
					EnumedDeviceList2->SelectedIndex = 0;
					ShowStatusMessage("Enumerating Webcams completed successfully.");
					btnStartDevice2->IsEnabled = true;
				}
			}
			catch (Exception ^e)
			{
				ShowExceptionMessage(e);
			}
		});
	}
	catch (Platform::Exception^ e)
	{
		ShowExceptionMessage(e);
	}
}

void AdvancedCapture::EnumerateMicrophonesAsync()
{
	try
	{
		ShowStatusMessage("Enumerating Microphones...");
		m_microPhoneInfoCollection = nullptr;

		EnumedMicrophonesList2->Items->Clear();

		task<DeviceInformationCollection^>(DeviceInformation::FindAllAsync(DeviceClass::AudioCapture)).then([this](task<DeviceInformationCollection^> findTask)
		{
			try
			{
				m_microPhoneInfoCollection = findTask.get();
				if (m_microPhoneInfoCollection == nullptr || m_microPhoneInfoCollection->Size == 0)
				{
					ShowStatusMessage("No Microphones found.");
				}
				else
				{
					for (unsigned int i = 0; i < m_microPhoneInfoCollection->Size; i++)
					{
						auto devInfo = m_microPhoneInfoCollection->GetAt(i);
						auto location = devInfo->EnclosureLocation;
						if (location != nullptr)
						{
							if (location->Panel == Windows::Devices::Enumeration::Panel(1))
							{
								EnumedMicrophonesList2->Items->Append(devInfo->Name + "-Front");
							}
							else if (location->Panel == Windows::Devices::Enumeration::Panel(2))
							{
								EnumedMicrophonesList2->Items->Append(devInfo->Name + "-Back");

							}
							else{
								EnumedMicrophonesList2->Items->Append(devInfo->Name);
							}

						}
						else
						{
							EnumedMicrophonesList2->Items->Append(devInfo->Name);
						}
					}
					EnumedMicrophonesList2->SelectedIndex = 0;
					ShowStatusMessage("Enumerating Microphones completed successfully.");
				}
			}
			catch (Exception ^e)
			{
				ShowExceptionMessage(e);
			}
		});
	}
	catch (Platform::Exception^ e)
	{
		ShowExceptionMessage(e);
	}
}

void AdvancedCapture::EnumerateSceneModeAsync()
{
	try
	{
		ShowStatusMessage("Enumerating SceneMode...");

		SceneModeList2->Items->Clear();

		auto sceneModes = m_mediaCaptureMgr->VideoDeviceController->SceneModeControl->SupportedModes;

		for (auto mode : sceneModes)
		{
			String^ modeName = nullptr;

			switch (mode)
			{
			case Windows::Media::Devices::CaptureSceneMode::Auto:
				modeName = ref new String(L"Auto");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Macro:
				modeName = ref new String(L"Macro");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Portrait:
				modeName = ref new String(L"Portrait");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Sport:
				modeName = ref new String(L"Sport");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Snow:
				modeName = ref new String(L"Snow");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Night:
				modeName = ref new String(L"Night");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Beach:
				modeName = ref new String(L"Beach");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Sunset:
				modeName = ref new String(L"Sunset");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Candlelight:
				modeName = ref new String(L"Candlelight");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Landscape:
				modeName = ref new String(L"Landscape");
				break;
			case Windows::Media::Devices::CaptureSceneMode::NightPortrait:
				modeName = ref new String(L"Night portrait");
				break;
			case Windows::Media::Devices::CaptureSceneMode::Backlit:
				modeName = ref new String(L"Backlit");
				break;
			}
			if (modeName != nullptr)
			{
				SceneModeList2->Items->Append(modeName);
			}
		}

		if (sceneModes->Size > 0)
		{
			SceneModeList2->SelectedIndex = 0;
		}

	}
	catch (Exception^ e)
	{
		ShowExceptionMessage(e);
	}

}

void AdvancedCapture::SceneMode_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (SceneModeList2->SelectedIndex > -1)
	{
		auto modeName = safe_cast<String^>(SceneModeList2->Items->GetAt(SceneModeList2->SelectedIndex));

		auto mode = Windows::Media::Devices::CaptureSceneMode::Auto;

		if (modeName == L"Macro")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Macro;
		}
		else if (modeName == L"Portrait")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Portrait;
		}
		else if (modeName == L"Sport")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Sport;
		}
		else if (modeName == L"Snow")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Snow;
		}
		else if (modeName == L"Night")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Night;
		}
		else if (modeName == L"Beach")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Beach;
		}
		else if (modeName == L"Sunset")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Sunset;
		}
		else if (modeName == L"Candlelight")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Candlelight;
		}
		else if (modeName == L"Landscape")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Landscape;
		}
		else if (modeName == L"Night portrait")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::NightPortrait;
		}
		else if (modeName == L"Backlight")
		{
			mode = Windows::Media::Devices::CaptureSceneMode::Backlit;
		}

		create_task(m_mediaCaptureMgr->VideoDeviceController->SceneModeControl->SetValueAsync(mode)).then([this, modeName]()
		{
			auto message = "SceneMode is set to " + modeName;
			ShowStatusMessage(message);
		});
	}

}

void AdvancedCapture::AddEffectToImageStream()
{
	auto mediaCapture = m_mediaCaptureMgr.Get();
	Windows::Media::Capture::VideoDeviceCharacteristic charecteristic = mediaCapture->MediaCaptureSettings->VideoDeviceCharacteristic;

	if ((charecteristic != Windows::Media::Capture::VideoDeviceCharacteristic::AllStreamsIdentical) &&
		(charecteristic != Windows::Media::Capture::VideoDeviceCharacteristic::PreviewPhotoStreamsIdentical) &&
		(charecteristic != Windows::Media::Capture::VideoDeviceCharacteristic::RecordPhotoStreamsIdentical))
	{
		Windows::Media::MediaProperties::IMediaEncodingProperties ^props = mediaCapture->VideoDeviceController->GetMediaStreamProperties(Windows::Media::Capture::MediaStreamType::Photo);
		if (props->Type->Equals("Image"))
		{
			//Switch to a video media type instead since we cant add an effect to a image media type
			Windows::Foundation::Collections::IVectorView<Windows::Media::MediaProperties::IMediaEncodingProperties^>^ supportedPropsList = mediaCapture->VideoDeviceController->GetAvailableMediaStreamProperties(Windows::Media::Capture::MediaStreamType::Photo);
			{
				unsigned int i = 0;
				while (i < supportedPropsList->Size)
				{
					Windows::Media::MediaProperties::IMediaEncodingProperties^ props = supportedPropsList->GetAt(i);

					String^ s = props->Type;
					if (props->Type->Equals("Video"))
					{
						task<void>(mediaCapture->VideoDeviceController->SetMediaStreamPropertiesAsync(Windows::Media::Capture::MediaStreamType::Photo, props)).then([this](task<void> changeTypeTask)
						{
							try
							{
								changeTypeTask.get();
								ShowStatusMessage("Change type on photo stream successful");
								//Now add the effect on the image pin
								task<void>(m_mediaCaptureMgr->AddEffectAsync(Windows::Media::Capture::MediaStreamType::Photo, "GrayscaleTransform.GrayscaleEffect", nullptr)).then([this](task<void> effectTask3)
								{
									try
									{
										effectTask3.get();
										m_bEffectAddedToPhoto = true;
										ShowStatusMessage("Adding effect to photo stream successful");
										chkAddRemoveEffect->IsEnabled = true;

									}
									catch (Exception ^e)
									{
										ShowExceptionMessage(e);
										chkAddRemoveEffect->IsEnabled = true;
										chkAddRemoveEffect->IsChecked = false;
									}
								});

							}
							catch (Exception ^e)
							{
								ShowExceptionMessage(e);
								chkAddRemoveEffect->IsEnabled = true;
								chkAddRemoveEffect->IsChecked = false;

							}

						});
						break;

					}
					i++;
				}
			}
		}
		else
		{
			//Add the effect to the image pin if the type is already "Video"
			task<void>(mediaCapture->AddEffectAsync(Windows::Media::Capture::MediaStreamType::Photo, "GrayscaleTransform.GrayscaleEffect", nullptr)).then([this](task<void> effectTask3)
			{
				try
				{
					effectTask3.get();
					m_bEffectAddedToPhoto = true;
					ShowStatusMessage("Adding effect to photo stream successful");
					chkAddRemoveEffect->IsEnabled = true;

				}
				catch (Exception ^e)
				{
					ShowExceptionMessage(e);
					chkAddRemoveEffect->IsEnabled = true;
					chkAddRemoveEffect->IsChecked = false;
				}
			});
		}
	}
}



void AdvancedCapture::chkAddRemoveEffect_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	try
	{
		chkAddRemoveEffect->IsEnabled = false;
		m_bEffectAdded = true;
		create_task(m_mediaCaptureMgr->AddEffectAsync(Windows::Media::Capture::MediaStreamType::VideoPreview, "GrayscaleTransform.GrayscaleEffect", nullptr)).then([this](task<void> effectTask)
		{
			try
			{
				effectTask.get();

				auto mediaCapture = m_mediaCaptureMgr.Get();
				Windows::Media::Capture::VideoDeviceCharacteristic charecteristic = mediaCapture->MediaCaptureSettings->VideoDeviceCharacteristic;

				ShowStatusMessage("Add effect successful to preview stream successful");
				if ((charecteristic != Windows::Media::Capture::VideoDeviceCharacteristic::AllStreamsIdentical) &&
					(charecteristic != Windows::Media::Capture::VideoDeviceCharacteristic::PreviewRecordStreamsIdentical))
				{
					Windows::Media::MediaProperties::IMediaEncodingProperties ^props = mediaCapture->VideoDeviceController->GetMediaStreamProperties(Windows::Media::Capture::MediaStreamType::VideoRecord);
					Windows::Media::MediaProperties::VideoEncodingProperties ^videoEncodingProperties = static_cast<Windows::Media::MediaProperties::VideoEncodingProperties ^>(props) ;
					if (!videoEncodingProperties->Subtype->Equals("H264")) //Cant add an effect to an H264 stream
					{
						task<void>(mediaCapture->AddEffectAsync(Windows::Media::Capture::MediaStreamType::VideoRecord, "GrayscaleTransform.GrayscaleEffect", nullptr)).then([this](task<void> effectTask2)
						{
							try
							{
								effectTask2.get();
								ShowStatusMessage("Add effect successful to record stream successful");
								m_bEffectAddedToRecord = true;
								AddEffectToImageStream();
								chkAddRemoveEffect->IsEnabled = true;
							}
							catch (Exception ^e)
							{
								ShowExceptionMessage(e);
								chkAddRemoveEffect->IsEnabled = true;
								chkAddRemoveEffect->IsChecked = false;
							}
						});
					}
					else
					{
						AddEffectToImageStream();
						chkAddRemoveEffect->IsEnabled = true;
					}

				}
				else
				{
					AddEffectToImageStream();
					chkAddRemoveEffect->IsEnabled = true;
				}
			}
			catch (Exception ^e)
			{
				ShowExceptionMessage(e);
				chkAddRemoveEffect->IsEnabled = true;
				chkAddRemoveEffect->IsChecked = false;
			}
		});
	}
	catch (Platform::Exception ^e)
	{
		ShowExceptionMessage(e);
		chkAddRemoveEffect->IsEnabled = true;
		chkAddRemoveEffect->IsChecked = false;
	}
}

void AdvancedCapture::chkAddRemoveEffect_Unchecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	try
	{
		chkAddRemoveEffect->IsEnabled = false;
		m_bEffectAdded = false;
		create_task(m_mediaCaptureMgr->ClearEffectsAsync(Windows::Media::Capture::MediaStreamType::VideoPreview)).then([this](task<void> effectTask)
		{
			try
			{
				effectTask.get();
				ShowStatusMessage("Remove effect from video preview stream successful");
				if (m_bEffectAddedToRecord)
				{
					task<void>(m_mediaCaptureMgr->ClearEffectsAsync(Windows::Media::Capture::MediaStreamType::VideoRecord)).then([this](task<void> effectTask)
					{
						try
						{
							effectTask.get();
							ShowStatusMessage("Remove effect from video record stream successful");
							m_bEffectAddedToRecord = false;
							if (m_bEffectAddedToPhoto)
							{
								task<void>(m_mediaCaptureMgr->ClearEffectsAsync(Windows::Media::Capture::MediaStreamType::Photo)).then([this](task<void> effectTask)
								{
									try
									{
										effectTask.get();
										ShowStatusMessage("Remove effect from Photo stream successful");
										m_bEffectAddedToPhoto = false;

									}
									catch (Exception ^e)
									{
										ShowExceptionMessage(e);
										chkAddRemoveEffect->IsEnabled = true;
										chkAddRemoveEffect->IsChecked = true;
									}

								});
							}
							else
							{
							}
							chkAddRemoveEffect->IsEnabled = true;
						}
						catch (Exception ^e)
						{
							ShowExceptionMessage(e);
							chkAddRemoveEffect->IsEnabled = true;
							chkAddRemoveEffect->IsChecked = true;

						}

					});

				}
				else if (m_bEffectAddedToPhoto)
				{
					task<void>(m_mediaCaptureMgr->ClearEffectsAsync(Windows::Media::Capture::MediaStreamType::Photo)).then([this](task<void> effectTask)
					{
						try
						{
							effectTask.get();
							ShowStatusMessage("Remove effect from Photo stream successful");
							m_bEffectAddedToPhoto = false;

						}
						catch (Exception ^e)
						{
							ShowExceptionMessage(e);
							chkAddRemoveEffect->IsEnabled = true;
							chkAddRemoveEffect->IsChecked = true;
						}

					});
				}
				else
				{
					chkAddRemoveEffect->IsEnabled = true;

				}
			}
			catch (Exception ^e)
			{
				ShowExceptionMessage(e);
				chkAddRemoveEffect->IsEnabled = true;
				chkAddRemoveEffect->IsChecked = true;
			}
		});
	}
	catch (Platform::Exception ^e)
	{
		ShowExceptionMessage(e);
		chkAddRemoveEffect->IsEnabled = true;
		chkAddRemoveEffect->IsChecked = true;
	}
}

void AdvancedCapture::radTakePhoto_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if (!m_bLowLagPrepared)
    {
        //if camera does not support lowlag record and lowlag photo at the same time
        //disable all buttons while preparing lowlag photo
        btnStartStopRecord2 ->IsEnabled = false;
        btnTakePhoto2->IsEnabled = false;
        //uncheck record Mode
        radRecord->IsChecked = false;
        //disable check option while preparing lowlag photo
        radTakePhoto->IsEnabled = false;
        radRecord->IsEnabled = false;

        if (m_bRecording)
        {
            //if camera does not support lowlag record and lowlag photo at the same time
            //but lowlag record is already prepared, un-prepare lowlag record first, before preparing lowlag photo
            create_task(m_lowLagRecord->FinishAsync()).then([this](task<void> recordTask)
            {
                try
                {
                    recordTask.get();
                    m_bRecording = false;
                    ShowStatusMessage("Stopped record on preparing lowlag Photo:" + m_recordStorageFile->Path);

                    create_task(m_mediaCaptureMgr->PrepareLowLagPhotoCaptureAsync(ImageEncodingProperties::CreateJpeg())).then([this](LowLagPhotoCapture ^photoCapture)
                    {
                        m_lowLagPhoto = photoCapture;
                        btnTakePhoto2->IsEnabled = true;
                        m_bLowLagPrepared = true;
                        //re-enable check option, after lowlag record finish preparing
                        radTakePhoto->IsEnabled = true;
                        radRecord->IsEnabled = true;
                    });
                }
                catch (Exception ^e)
                {
                    ShowExceptionMessage(e);
                    radTakePhoto->IsEnabled = true;
                    radRecord->IsEnabled = true;
                }
            });
        }else //(!m_bRecording)
        {
            //if camera does not support lowlag record and lowlag photo at the same time
            //lowlag record is not prepared, go ahead to prepare lowlag photo
            create_task(m_mediaCaptureMgr->PrepareLowLagPhotoCaptureAsync(ImageEncodingProperties::CreateJpeg())).then([this](LowLagPhotoCapture ^photoCapture)
            {
                m_lowLagPhoto = photoCapture;
                btnTakePhoto2->IsEnabled = true;
                m_bLowLagPrepared = true;
                //re-enable check option, after lowlag record finish preparing
                radTakePhoto->IsEnabled = true;
                radRecord->IsEnabled = true;
            });
        }
    }
}

void AdvancedCapture::radRecord_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if (!m_bRecording)
    {
        //if camera does not support lowlag record and lowlag photo at the same time
        //disable all buttons while preparing lowlag record
        btnTakePhoto2->IsEnabled = false;
        btnStartStopRecord2->IsEnabled = false;
        //uncheck TakePhoto Mode
        radTakePhoto->IsChecked = false;
        //disable check option while preparing lowlag record
        radTakePhoto->IsEnabled = false;
        radRecord->IsEnabled = false;

        if (m_bLowLagPrepared)
        {
            //if camera does not support lowlag record and lowlag photo at the same time
            //but lowlag photo is already prepared, un-prepare lowlag photo first, before preparing lowlag record
            create_task(m_lowLagPhoto->FinishAsync()).then([this](task<void> Task)
            {
                m_bLowLagPrepared = false;
                //prepare lowlag record
                PrepareForVideoRecording();

                task<StorageFile^>(KnownFolders::VideosLibrary->CreateFileAsync(VIDEO_FILE_NAME, Windows::Storage::CreationCollisionOption::GenerateUniqueName)).then(
                    [this](task<StorageFile^> fileTask)
                {
                    this->m_recordStorageFile = fileTask.get();
                    ShowStatusMessage("Create record file successful");

                    MediaEncodingProfile^ recordProfile= nullptr;
                    recordProfile = MediaEncodingProfile::CreateMp4(Windows::Media::MediaProperties::VideoEncodingQuality::Auto);

                    create_task(m_mediaCaptureMgr->PrepareLowLagRecordToStorageFileAsync(recordProfile, m_recordStorageFile)).then(
                        [this](LowLagMediaRecording ^lowLagRecord)
                    {
                        m_lowLagRecord = lowLagRecord;
                        btnStartStopRecord2->IsEnabled = true;
                        m_bRecording = true;
                        btnStartStopRecord2->Content = "StartRecord";
                        //re-enable check option, after lowlag record finish preparing
                        radTakePhoto->IsEnabled = true;
                        radRecord->IsEnabled = true;
                    });
                });
            });

        }else //(!m_bLowLagPrepared)
        {
            //if camera does not support lowlag record and lowlag photo at the same time
            //lowlag photo is not prepared, go ahead to prepare lowlag record
            PrepareForVideoRecording();

            task<StorageFile^>(KnownFolders::VideosLibrary->CreateFileAsync(VIDEO_FILE_NAME, Windows::Storage::CreationCollisionOption::GenerateUniqueName)).then(
                [this](task<StorageFile^> fileTask)
            {
                this->m_recordStorageFile = fileTask.get();
                ShowStatusMessage("Create record file successful");

                MediaEncodingProfile^ recordProfile= nullptr;
                recordProfile = MediaEncodingProfile::CreateMp4(Windows::Media::MediaProperties::VideoEncodingQuality::Auto);

                create_task(m_mediaCaptureMgr->PrepareLowLagRecordToStorageFileAsync(recordProfile, m_recordStorageFile)).then(
                    [this](LowLagMediaRecording ^lowLagRecord)
                {
                    m_lowLagRecord = lowLagRecord;
                    btnStartStopRecord2->IsEnabled = true;
                    m_bRecording = true;
                    btnStartStopRecord2->Content = "StartRecord";
                    //re-enable check option, after lowlag record finish preparing
                    radTakePhoto->IsEnabled = true;
                    radRecord->IsEnabled = true;
                });
            });
        }
    }
}

void AdvancedCapture::ShowStatusMessage(Platform::String^ text)
{
	rootPage->NotifyUser(text, NotifyType::StatusMessage);
}

void AdvancedCapture::ShowExceptionMessage(Platform::Exception^ ex)
{
	rootPage->NotifyUser(ex->Message, NotifyType::ErrorMessage);
}

task<Windows::Storage::StorageFile^> AdvancedCapture::ReencodePhotoAsync(
	Windows::Storage::Streams::IRandomAccessStream ^stream,
	Windows::Storage::FileProperties::PhotoOrientation photoRotation)
{
	ReencodeState ^state = ref new ReencodeState();

	state->InputStream = stream;

	return create_task(Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(state->InputStream)).then([state](Windows::Graphics::Imaging::BitmapDecoder ^decoder)
	{
		state->Decoder = decoder;
		return Windows::Storage::KnownFolders::PicturesLibrary->CreateFileAsync(PHOTO_FILE_NAME, Windows::Storage::CreationCollisionOption::GenerateUniqueName);
	}).then([state](Windows::Storage::StorageFile ^storageFile)
	{
		state->PhotoStorage = storageFile;
		return state->PhotoStorage->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite);
	}).then([state](Windows::Storage::Streams::IRandomAccessStream ^stream)
	{
		state->OutputStream = stream;
		state->OutputStream->Size = 0;
		return Windows::Graphics::Imaging::BitmapEncoder::CreateForTranscodingAsync(state->OutputStream, state->Decoder);
	}).then([state, photoRotation](Windows::Graphics::Imaging::BitmapEncoder ^encoder)
	{
		state->Encoder = encoder;
		auto properties = ref new Windows::Graphics::Imaging::BitmapPropertySet();
		properties->Insert("System.Photo.Orientation",
			ref new Windows::Graphics::Imaging::BitmapTypedValue((unsigned short) photoRotation, Windows::Foundation::PropertyType::UInt16));
		return create_task(state->Encoder->BitmapProperties->SetPropertiesAsync(properties));
	}).then([state]()
	{
		return state->Encoder->FlushAsync();
	}).then([state](task<void> previousTask)
	{
		auto result = state->PhotoStorage;
		delete state;

		previousTask.get();

		return result;
	});
}

Windows::Storage::FileProperties::PhotoOrientation AdvancedCapture::GetCurrentPhotoRotation()
{
	bool counterclockwiseRotation = m_bReversePreviewRotation;

    if (m_bRotateVideoOnOrientationChange)
    {
        return PhotoRotationLookup(m_displayOrientation, counterclockwiseRotation);
    }
    else
    {
        return Windows::Storage::FileProperties::PhotoOrientation::Normal;
    }
}

void AdvancedCapture::PrepareForVideoRecording()
{
	Windows::Media::Capture::MediaCapture ^mediaCapture = m_mediaCaptureMgr.Get();
	if (mediaCapture == nullptr)
	{
		return;
	}

	bool counterclockwiseRotation = m_bReversePreviewRotation;

    if (m_bRotateVideoOnOrientationChange)
    {
        mediaCapture->SetRecordRotation(VideoRotationLookup(m_displayOrientation, counterclockwiseRotation));
    }
    else
    {
        mediaCapture->SetRecordRotation(Windows::Media::Capture::VideoRotation::None);
    }
}

void AdvancedCapture::OrientationChanged()
{
	Windows::Media::Capture::MediaCapture ^mediaCapture = m_mediaCaptureMgr.Get();
	if (mediaCapture == nullptr)
	{
		return;
	}

	bool previewMirroring = mediaCapture->GetPreviewMirroring();
	bool counterclockwiseRotation = (previewMirroring && !m_bReversePreviewRotation) ||
		(!previewMirroring && m_bReversePreviewRotation);

    if (m_bRotateVideoOnOrientationChange)
    {
        mediaCapture->SetPreviewRotation(VideoRotationLookup(m_displayOrientation, counterclockwiseRotation));
    }
    else
    {
        mediaCapture->SetPreviewRotation(Windows::Media::Capture::VideoRotation::None);
    }
}

void AdvancedCapture::DisplayInfo_OrientationChanged(
    _In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args)
{
    m_displayOrientation = sender->CurrentOrientation;
    OrientationChanged();
}

Windows::Storage::FileProperties::PhotoOrientation AdvancedCapture::PhotoRotationLookup(
	Windows::Graphics::Display::DisplayOrientations displayOrientation, bool counterclockwise)
{
	switch (displayOrientation)
	{
	case Windows::Graphics::Display::DisplayOrientations::Landscape:
		return Windows::Storage::FileProperties::PhotoOrientation::Normal;

	case Windows::Graphics::Display::DisplayOrientations::Portrait:
		return (counterclockwise) ? Windows::Storage::FileProperties::PhotoOrientation::Rotate270 :
			Windows::Storage::FileProperties::PhotoOrientation::Rotate90;

	case Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped:
		return Windows::Storage::FileProperties::PhotoOrientation::Rotate180;

	case Windows::Graphics::Display::DisplayOrientations::PortraitFlipped:
		return (counterclockwise) ? Windows::Storage::FileProperties::PhotoOrientation::Rotate90 :
			Windows::Storage::FileProperties::PhotoOrientation::Rotate270;

	default:
		return Windows::Storage::FileProperties::PhotoOrientation::Unspecified;
	}
}

Windows::Media::Capture::VideoRotation AdvancedCapture::VideoRotationLookup(
	Windows::Graphics::Display::DisplayOrientations displayOrientation, bool counterclockwise)
{
	switch (displayOrientation)
	{
	case Windows::Graphics::Display::DisplayOrientations::Landscape:
		return Windows::Media::Capture::VideoRotation::None;

	case Windows::Graphics::Display::DisplayOrientations::Portrait:
		return (counterclockwise) ? Windows::Media::Capture::VideoRotation::Clockwise270Degrees :
			Windows::Media::Capture::VideoRotation::Clockwise90Degrees;

	case Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped:
		return Windows::Media::Capture::VideoRotation::Clockwise180Degrees;

	case Windows::Graphics::Display::DisplayOrientations::PortraitFlipped:
		return (counterclockwise) ? Windows::Media::Capture::VideoRotation::Clockwise90Degrees :
			Windows::Media::Capture::VideoRotation::Clockwise270Degrees;

	default:
		return Windows::Media::Capture::VideoRotation::None;
	}
}
