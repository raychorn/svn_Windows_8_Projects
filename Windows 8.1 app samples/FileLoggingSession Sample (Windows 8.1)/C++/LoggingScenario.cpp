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
using namespace SDKSample::FileLoggingSession;

LoggingScenario^ LoggingScenario::loggingScenario;


LoggingScenario::LoggingScenario()
{
	_logFileGeneratedCount = 0;
	_logMessageCount = 0;
    _logMessageApproximateByteCount = 0;
    _isSuspending = false;
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

    // Get a deferral before performing any async operations
    // to avoid suspension prior to LoggingScenario completing 
    // PrepareToSuspendAsync().
    auto deferral = e->SuspendingOperation->GetDeferral();
    PrepareToSuspendAsync().then([=](task<void> previousTask)
    {
        // From LoggingScenario's perspective, it's now okay to 
        // suspend, so release the deferral. 
        deferral->Complete();
    });
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

void LoggingScenario::OnLogFileGenerated(IFileLoggingSession^ sender, LogFileGeneratedEventArgs^ args)
{
    InterlockedIncrement(&_logFileGeneratedCount);
    create_task(MoveFileToRepositoryAsync(args->File)).then([this](Platform::String^ finalLogFileFullPath)
    {
        if (_isSuspending == false)
        {
            StatusChanged(this, ref new LoggingScenarioEventArgs(LoggingScenarioEventType::LogFileGenerated, finalLogFileFullPath));
        }
    });
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

task<Platform::String^> LoggingScenario::MoveFileToRepositoryAsync(StorageFile^ sourceFile)
{
    struct ContinuationData
    {
        StorageFolder^ logRepositoryFolder;
        Platform::String^ nextLogFileName;
    };
    auto data = make_shared<ContinuationData>();

    return GetLogRepositoryFolderAsync().then([this, data](StorageFolder^ logRepositoryFolder)
	{
        _lockForMoveFile.lock();
        data->logRepositoryFolder = logRepositoryFolder;
        return GetNextLogFileNameAsync();
    }).then([this, sourceFile, data](Platform::String^ nextLogFileName)
    {
        data->nextLogFileName = nextLogFileName;
        return sourceFile->MoveAsync(data->logRepositoryFolder, nextLogFileName, Windows::Storage::NameCollisionOption::FailIfExists);
    }).then([this, data](task<void> previousTask)
    {
        _lockForMoveFile.unlock();
        return data->logRepositoryFolder->Path + "\\" + data->nextLogFileName;
    });
}

task<Platform::String^> LoggingScenario::SaveFinalLogFileAsync(Windows::Foundation::Diagnostics::FileLoggingSession^ sessionToCloseAndSave)
{
    return create_task(sessionToCloseAndSave->CloseAndSaveToFileAsync()).then([this](StorageFile^ finalFileBeforeSuspend)
    {
        if (finalFileBeforeSuspend != nullptr)
        {
            return create_task(MoveFileToRepositoryAsync(finalFileBeforeSuspend));
        }

        return task<Platform::String^>([]() -> Platform::String^
        {
            return nullptr;
        });     
    }).then([this](Platform::String^ finalAppLogFileFullPath)
    {
        return finalAppLogFileFullPath;
    });
}

task<bool> LoggingScenario::ToggleLoggingEnabledDisabledAsync()
{
    IsBusy = true;
    try
    {
        Windows::Foundation::Diagnostics::FileLoggingSession^ sessionCopy;
        sessionCopy = InterlockedExchangeRefValues(&_session, (Windows::Foundation::Diagnostics::FileLoggingSession^)nullptr);
        if (sessionCopy != nullptr)
        {
            return create_task(SaveFinalLogFileAsync(sessionCopy)).then([this, sessionCopy](Platform::String^ finalLogFilePath)
            {
                delete sessionCopy;
                StatusChanged(this, ref new LoggingScenarioEventArgs(LoggingScenarioEventType::LogFileGeneratedAtDisable, finalLogFilePath));
                SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, false);
                StatusChanged(this, ref new LoggingScenarioEventArgs(false));
                IsBusy = false;
                return false;
            });
        }
        else
        {
            StartLogging();
            SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, true);
            StatusChanged(this, ref new LoggingScenarioEventArgs(true));
            IsBusy = false;
            return task<bool>([]() -> bool
            {
                return true;
            });
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
    // StartLogging called in two places: resume and when logging is 
    // enabled. It is assumed both cannot occur at the same time so no
    // interlocked or synchronization is used when checking 'session' here. 
    if (_session == nullptr)
    {
        _session = ref new Windows::Foundation::Diagnostics::FileLoggingSession(DEFAULT_SESSION_NAME);
        _session->LogFileGenerated += ref new Windows::Foundation::TypedEventHandler<IFileLoggingSession ^, LogFileGeneratedEventArgs ^>(this, &LoggingScenario::OnLogFileGenerated);
    }

    if (_channel == nullptr)
    {
        _channel = ref new Windows::Foundation::Diagnostics::LoggingChannel(DEFAULT_CHANNEL_NAME);
        _channel->LoggingEnabled += ref new Windows::Foundation::TypedEventHandler<Windows::Foundation::Diagnostics::ILoggingChannel ^, Platform::Object ^>(this, &LoggingScenario::OnChannelLoggingEnabled);
    }

    // This sample adds the channel at level "warning" to 
    // demonstrated how messages logged at more verbose levels
    // are ignored by the session. 
    _session->AddLoggingChannel(_channel, LoggingLevel::Warning);
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

task<void> LoggingScenario::PrepareToSuspendAsync()
{
    Windows::Foundation::Diagnostics::FileLoggingSession^ sessionCopy;
    sessionCopy = InterlockedExchangeRefValues(&_session, (Windows::Foundation::Diagnostics::FileLoggingSession^)nullptr);
    if (sessionCopy != nullptr)
    {
        _isSuspending = true;
        return create_task(SaveFinalLogFileAsync(sessionCopy)).then([this, sessionCopy](Platform::String^ finalLogFilePath)
        {
            delete sessionCopy;
            SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, true);
            SetAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME, finalLogFilePath);          
        }).then([this](task<void> previousTask)
        {
            _isSuspending = false;
        });
    }
    else
    {
        SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, false);
        SetAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME, nullptr);
        return task<void>([]() -> void {});
    }
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

    // When the sample suspends, it retains state as to whether or not it had
    // generated a new log file at the last suspension. This allows any
    // UI to be updated on resume to reflect that fact. 
    if (IsAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME))
    {
        Platform::String^ logFileGeneratedBeforeSuspend = GetAppLocalSettingsValueAsString(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME);
        if (logFileGeneratedBeforeSuspend != nullptr)
        {
            StatusChanged(this, ref new LoggingScenarioEventArgs(LoggingScenarioEventType::LogFileGeneratedAtSuspend, logFileGeneratedBeforeSuspend));
        }
        SetAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME, nullptr);
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
                LogMessage("Value #" + (++messageIndex).ToString() + "  " + value.ToString(), LoggingLevel::Critical); // value is logged as 14 byte wide character string.
                LogValuePair("Value #" + (++messageIndex).ToString(), value, LoggingLevel::Critical); // value is logged as a 4-byte integer.

                //
                // Pause every once in a while to simulate application 
                // activity outside of logging.
                //

                if (messageIndex % 50 == 0)
                {                   
                    complete_after(5).wait();
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
