//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// LoggingScenario.cpp
// Implementation of the LoggingScenario class
//

#include "pch.h"
#include "LoggingScenario.h"

using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Search;
using namespace Platform::Collections;
using namespace SDKSample;
using namespace SDKSample::LoggingSession;

LoggingScenario^ LoggingScenario::loggingScenario;

LoggingScenario::LoggingScenario()
{
	_logFileGeneratedCount = 0;
	_logMessageCount = 0;
    _logMessageApproximateByteCount = 0;
    _isBusy = false;

    SDKSample::App::Current->Suspending += ref new Windows::UI::Xaml::SuspendingEventHandler(this, &LoggingScenario::OnAppSuspending);
    SDKSample::App::Current->Resuming += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &LoggingScenario::OnAppResuming);

    // If the app is being launched (not resumed), the 
    // following call will activate logging if it had been 
    // activated during the last suspend. 
    ResumeLoggingIfApplicable();
}

LoggingScenario::~LoggingScenario()
{

}

void LoggingScenario::OnAppSuspending(Platform::Object ^sender, Windows::ApplicationModel::SuspendingEventArgs ^e)
{
    (void) sender;    // Unused parameter
    auto deferral = e->SuspendingOperation->GetDeferral();
    PrepareToSuspend();
    deferral->Complete();
}

void LoggingScenario::OnAppResuming(Platform::Object ^sender, Platform::Object ^args)
{
    // If logging was active at the last suspend,
    // ResumeLoggingIfApplicable will re-activate 
    // logging.
    ResumeLoggingIfApplicable();
}

void LoggingScenario::OnChannelLoggingEnabled(ILoggingChannel^ sender, Platform::Object^ args)
{
	_isChannelEnabled = sender->Enabled;
	_channelLoggingLevel = sender->Level;
}

task<StorageFolder^> LoggingScenario::GetLogRepositoryFolderAsync()
{
	return create_task(ApplicationData::Current->LocalFolder->CreateFolderAsync(LOG_FILE_REPOSITORY_FOLDER_NAME, CreationCollisionOption::OpenIfExists));
}

task<IVectorView<StorageFile^>^> LoggingScenario::GetAllLogRepositoryFileNamesAsync()
{
	return GetLogRepositoryFolderAsync().then([this](StorageFolder^ logRepositoryFolder)
	{
		QueryOptions^ queryOptions = ref new QueryOptions(CommonFileQuery::OrderBySearchRank, nullptr);
		queryOptions->ApplicationSearchFilter = "System.FileName:~\"" + LOG_FILE_BASE_FILE_NAME + "-????.etl\"";
		StorageFileQueryResult^ queryResult = logRepositoryFolder->CreateFileQueryWithOptions(queryOptions);
		return create_task(queryResult->GetFilesAsync());
	});
}

task<int> LoggingScenario::GetNextLogFileNumberAsync()
{
	return GetAllLogRepositoryFileNamesAsync().then([=](IVectorView<StorageFile^>^ files)
	{
		Platform::String^ extractNumberRegExPattern = LOG_FILE_BASE_FILE_NAME + "-(\\d{4})\\.etl";
		std::wregex extractNumberRegEx(extractNumberRegExPattern->Data());
		int lastNumber = 0;
		std::for_each(begin(files), end(files), [=,&lastNumber](StorageFile^ file)
		{
			std::wstring fileName(file->Name->Data());
			std::wsmatch m;
			if (std::regex_search(fileName, m, extractNumberRegEx))
			{
				int tempNumber = std::stoi(m[1].str());
				if (tempNumber > lastNumber)
				{
					lastNumber = tempNumber;
				}
			}
		});

		return lastNumber + 1;
	});
}

task<Platform::String^> LoggingScenario::GetNextLogFileNameAsync()
{
	return GetNextLogFileNumberAsync().then([=](int nextLogFileNumber)
	{
		std::wostringstream nextLogFileName;
        nextLogFileName 
            << std::wstring(LOG_FILE_BASE_FILE_NAME->Data())
            << L'-'
            << std::setfill(L'0')
            << std::setw(4)
            << nextLogFileNumber
            << L".etl";
        return ref new Platform::String(nextLogFileName.str().c_str());
	});
}

task<Platform::String^> LoggingScenario::SaveLogInMemoryToFileAsync(Windows::Foundation::Diagnostics::LoggingSession^ sessionToSnapshot)
{
    struct ContinuationData
    {
        Windows::Foundation::Diagnostics::LoggingSession^ sessionToSnapshot;
        StorageFolder^ logRepositoryFolder;
        Platform::String^ nextLogFileName;
        Platform::String^ finalAppLogFileFullPath;
    };
    auto data = make_shared<ContinuationData>();
    data->sessionToSnapshot = sessionToSnapshot;

    return GetLogRepositoryFolderAsync().then([this, data](StorageFolder^ logRepositoryFolder)
    {
        _lockForMoveFile.lock();
        data->logRepositoryFolder = logRepositoryFolder;
        return GetNextLogFileNameAsync();
    }).then([this, data](Platform::String^ nextLogFileName)
    {
        data->nextLogFileName = nextLogFileName;
        return create_task(data->sessionToSnapshot->SaveToFileAsync(data->logRepositoryFolder, nextLogFileName));
    }).then([this, data](Windows::Storage::StorageFile^ savedLogFile)
    {
        data->finalAppLogFileFullPath = "";
        data->finalAppLogFileFullPath = savedLogFile->Path;
    }).then([this, data](task<void> previousTask)
    {
        _lockForMoveFile.unlock();
        return data->finalAppLogFileFullPath;
    });    
}

bool LoggingScenario::ToggleLoggingEnabledDisabled()
{
    IsBusy = true;
    try
    {
        Windows::Foundation::Diagnostics::LoggingSession^ sessionCopy;
        sessionCopy = InterlockedExchangeRefValues(&_session, (Windows::Foundation::Diagnostics::LoggingSession^)nullptr);
        if (sessionCopy != nullptr)
        {
            delete sessionCopy;
            SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, false);
            StatusChanged(this, ref new LoggingScenarioEventArgs(false));
            IsBusy = false;
            return false;
        }
        else
        {
            StartLogging();
            SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, true);
            StatusChanged(this, ref new LoggingScenarioEventArgs(true));
            IsBusy = false;
            return true;
        }
    }
    catch (...)
    {
        IsBusy = false;
        throw;
    }
}

void LoggingScenario::StartLogging()
{
    bool sessionJustCreated = false;
    bool channelJustCreated = false;

    // StartLogging called in two places: resume and when logging is 
    // enabled. It is assumed both cannot occur at the same time so no
    // interlocked or synchronization is used when checking 'session' here. 
    if (_session == nullptr)
    {
        _session = ref new Windows::Foundation::Diagnostics::LoggingSession(DEFAULT_SESSION_NAME);
        sessionJustCreated = true;
    }

    if (_channel == nullptr)
    {
        _channel = ref new Windows::Foundation::Diagnostics::LoggingChannel(DEFAULT_CHANNEL_NAME);
        _channel->LoggingEnabled += ref new Windows::Foundation::TypedEventHandler<Windows::Foundation::Diagnostics::ILoggingChannel ^, Platform::Object ^>(this, &LoggingScenario::OnChannelLoggingEnabled);
        channelJustCreated = true;
    }

    if (sessionJustCreated || channelJustCreated)
    {
        // This sample adds the channel at level "warning" to 
        // demonstrated how messages logged at more verbose levels
        // are ignored by the session. 
        _session->AddLoggingChannel(_channel, LoggingLevel::Warning);
    }
}

void LoggingScenario::LogMessage(Platform::String^ message, Windows::Foundation::Diagnostics::LoggingLevel level)
{
    if (IsLoggingChannelEnabledForLevel(level))
    {
        _channel->LogMessage(message, level);

        // Total bytes is roughly 2 bytes per wide character.
        // This non-critical calculation is for sample UI feedback purposes.
        LogMessageApproximateByteCount += message->Length() * 2;
        LogMessageCount++;
    }
}

void LoggingScenario::LogValuePair(Platform::String^ value1, int value2, Windows::Foundation::Diagnostics::LoggingLevel level)
{
    if (IsLoggingChannelEnabledForLevel(level))
    {
        _channel->LogValuePair(value1, value2, level);

        // Total bytes is roughly 2 bytes per wide character, plus 4 bytes for the integer.
        // This non-critical calculation is for sample UI feedback purposes.
        LogMessageApproximateByteCount += (value1->Length() * 2) + 4; 
        LogMessageCount++;
    }
}


bool LoggingScenario::IsLoggingChannelEnabledForLevel(Windows::Foundation::Diagnostics::LoggingLevel level)
{
    if (_channel == nullptr)
    {
        //
        // No channel, so "false" logging is not enabled for the specified level.
        //

        return false;
    }

    if (_isChannelEnabled && level >= _channelLoggingLevel)
    {
        //
        // The channel is enabled, and the caller's level is equal to or higher
        // than the aggregate level of all the channels' listeners.
        //

        return true;
    }

    //
    // No sessions consuming events from this channel, so 'false' for "not enabled."
    //

    return false;
}

void LoggingScenario::PrepareToSuspend()
{
    SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, _session != nullptr);
}

void LoggingScenario::ResumeLoggingIfApplicable()
{
    bool loggingEnabled;
    if (IsAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME))
    {
        loggingEnabled = GetAppLocalSettingsValueAsBool(LOGGING_ENABLED_SETTING_KEY_NAME);
    }
    else
    {         
        SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, true);
        loggingEnabled = true;
    }

    if (loggingEnabled)
    {
        StartLogging();
    }
}

task<void> LoggingScenario::DoScenarioAsync()
{
    IsBusy = true;
    try
    {
        auto workItemDelegate = [this](Windows::Foundation::IAsyncAction^ workItem)
        {
            const int NUMBER_OF_LOG_FILES_TO_GENERATE = 3;
            int messageIndex = 0;
            int startFileCount = LogFileGeneratedCount;

            while (LogFileGeneratedCount - startFileCount < NUMBER_OF_LOG_FILES_TO_GENERATE)
            {
                try
                {

                    // Since the channel is added to the session at level Warning,
                    // the following is logged because it is logged at level LoggingLevel.Critical.
                    Platform::String^ messageToLog =
                        "Message=" +
                        (++messageIndex).ToString() +
                        ": Lorem ipsum dolor sit amet, consectetur adipiscing elit.In ligula nisi, vehicula nec eleifend vel, rutrum non dolor.Vestibulum ante ipsum " +
                        "primis in faucibus orci luctus et ultrices posuere cubilia Curae; Curabitur elementum scelerisque accumsan. In hac habitasse platea dictumst.";
                    LogMessage(messageToLog, LoggingLevel::Critical);

                    // Since the channel is added to the session at level Warning,
                    // the following is *not* logged because it is logged at LoggingLevel.Information.
                    messageToLog =
                        "Message=" +
                        (++messageIndex).ToString() +
                        ": Lorem ipsum dolor sit amet, consectetur adipiscing elit.In ligula nisi, vehicula nec eleifend vel, rutrum non dolor.Vestibulum ante ipsum " +
                        "primis in faucibus orci luctus et ultrices posuere cubilia Curae; Curabitur elementum scelerisque accumsan. In hac habitasse platea dictumst.";
                    LogMessage(messageToLog, LoggingLevel::Information);

                    int value = 1000000; // one million, 7 digits, 4-bytes as an int, 14 bytes as a wide character string.
                    messageIndex++;
                    LogMessage("Value #" + messageIndex.ToString() + ":" + value.ToString(), LoggingLevel::Critical); // 'value' is logged as 14 byte wide character string.
                    messageIndex++;
                    LogValuePair("Value", value, LoggingLevel::Critical); // 'value' is logged as a 4-byte integer.

                    //
                    // Pause every once in a while to simulate application 
                    // activity outside of logging.
                    //

                    if (messageIndex % 50 == 0)
                    {
                        complete_after(5).wait();
                    }

                    //
                    // Every once in a while, simulate an application error
                    // which causes the app to save the current snapshot
                    // of logging events in memory to a disk ETL file.
                    //

                    if (messageIndex != 0 && messageIndex % 25000 == 0)
                    {
                        throw exception("Some bad app error occurred.");
                    }
                }
                catch (exception e)
                {
                    ::std::wstring wideWhat;
                    if (e.what() != nullptr)
                    {
                        int convertResult = MultiByteToWideChar(CP_UTF8, 0, e.what(), strlen(e.what()), NULL, 0);
                        if (convertResult <= 0)
                        {
                            wideWhat = L"Exception occurred: Failure to convert its message text using MultiByteToWideChar: convertResult=";
                            wideWhat += convertResult.ToString()->Data();
                            wideWhat += L"  GetLastError()=";
                            wideWhat += GetLastError().ToString()->Data();
                        }
                        else
                        {
                            wideWhat.resize(convertResult + 10);
                            convertResult = MultiByteToWideChar(CP_UTF8, 0, e.what(), strlen(e.what()), &wideWhat[0], wideWhat.size());
                            if (convertResult <= 0)
                            {
                                wideWhat = L"Exception occurred: Failure to convert its message text using MultiByteToWideChar: convertResult=";
                                wideWhat += convertResult.ToString()->Data();
                                wideWhat += L"  GetLastError()=";
                                wideWhat += GetLastError().ToString()->Data();
                            }
                            else
                            {
                                wideWhat.insert(0, L"Exception occurrred: ");
                            }
                        }
                    }
                    else 
                    {
                        wideWhat = L"Exception occurred: Unknown.";
                    }

                    Platform::String^ errorMessage = ref new Platform::String(wideWhat.c_str());
                    // The session added the channel at level Warning. Log the message at 
                    // level Error which is above (more critical than) Warning, which 
                    // means it will actually get logged.
                    LogMessage(errorMessage, LoggingLevel::Error);
                    SaveLogInMemoryToFileAsync(_session).then([=](Platform::String^ logFileName) {
                        _logFileGeneratedCount++;
                        StatusChanged(this, ref new LoggingScenarioEventArgs(LoggingScenarioEventType::LogFileGenerated, logFileName));
                    }).wait();
                }

            }

            IsBusy = false;
        };

        auto workItemHandler = ref new Windows::System::Threading::WorkItemHandler(workItemDelegate);
        return create_task(Windows::System::Threading::ThreadPool::RunAsync(workItemHandler, Windows::System::Threading::WorkItemPriority::Normal)).then([](task<void> previousTask){

            try
            {
                previousTask.get();
            }
            catch (...)
            {
                OutputDebugString(L"Exception during DoScenario.");
                throw;
            }
        });
    }
    catch (...)
    {
        IsBusy = false;
        throw;
    }
}
