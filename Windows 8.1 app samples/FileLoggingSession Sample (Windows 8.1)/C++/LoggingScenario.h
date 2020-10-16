//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// LoggingScenario.h
// Declaration of the LoggingScenario class
//

#include <agents.h>
#include <ppltasks.h>

#pragma once
using namespace std;
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation::Diagnostics;

namespace SDKSample
{
	namespace FileLoggingSession
	{
        template<typename T> T^ InterlockedExchangeRefValues(T^* target, T^ value)
        {
            T^ result;
            *((PVOID*) &result) = InterlockedExchangePointer((PVOID*) target, (PVOID) value);
            return result;
        }

        inline task<void> complete_after(unsigned int timeout)
        {
            // A task completion event that is set when a timer fires.
            task_completion_event<void> tce;

            // Create a non-repeating timer.
            auto fire_once = new timer<int>(timeout, 0, nullptr, false);
            // Create a call object that sets the completion event after the timer fires.
            auto callback = new call<int>([tce](int)
            {
                tce.set();
            });

            // Connect the timer to the callback and start the timer.
            fire_once->link_target(callback);
            fire_once->start();

            // Create a task that completes after the completion event is set.
            task<void> event_set(tce);

            // Create a continuation task that cleans up resources and 
            // and return that continuation task. 
            return event_set.then([callback, fire_once]()
            {
                delete callback;
                delete fire_once;
            });
        }

        enum LoggingScenarioEventType
        {
            BusyStatusChanged,
            LogFileGenerated,
            LogFileGeneratedAtDisable,
            LogFileGeneratedAtSuspend,
            LoggingEnabledDisabled,
            LogMessageCountUpdate
        };

        ref class LoggingScenarioEventArgs sealed
        {
        private:

            LoggingScenarioEventType _type;
            Platform::String^  _logFilePath;
            bool _enabled;

        internal:

            LoggingScenarioEventArgs(LoggingScenarioEventType type)
            {
                _type = type;
            }

            LoggingScenarioEventArgs(LoggingScenarioEventType type, Platform::String^ logFilePath)
            {
                _type = type;
                _logFilePath = logFilePath;
            }

            LoggingScenarioEventArgs(bool enabled)
            {
                _type = LoggingEnabledDisabled;
                _enabled = enabled;
            }

            property LoggingScenarioEventType Type
            {
                LoggingScenarioEventType get()
                {
                    return _type;
                }
            }

            property Platform::String^ LogFilePath
            {
                Platform::String^ get()
                {
                    return _logFilePath;
                }
            }

            property bool Enabled
            {
                bool get()
                {
                    return _enabled;
                }
            }
        };

        ref class LoggingScenario;
        delegate void StatusChangedHandler(LoggingScenario^ sender, LoggingScenarioEventArgs^ args);


		/// <summary>
		/// An empty page that can be used on its own or navigated to within a Frame.
		/// </summary>
		ref class LoggingScenario sealed
		{

        internal:

            LoggingScenario();
            event StatusChangedHandler^ StatusChanged;
            //
            // Constants:
            //

            static property Platform::String^ DEFAULT_SESSION_NAME
            {
                Platform::String^ get()
                {
                    return ref new Platform::String(L"AppSession1");
                }
            }

            static property Platform::String^ DEFAULT_CHANNEL_NAME
            {
                Platform::String^ get()
                {
                    return ref new Platform::String(L"AppChannel1");
                }
            }

            static property Platform::String^ LOG_FILE_REPOSITORY_FOLDER_NAME
            {
                Platform::String^ get()
                {
                    return ref new Platform::String(L"OurAppLogFiles");
                }
            }

            static property Platform::String^ LOG_FILE_BASE_FILE_NAME
            {
                Platform::String^ get()
                {
                    return ref new Platform::String(L"OurAppLog");
                }
            }

            static property Platform::String^ LOGGING_ENABLED_SETTING_KEY_NAME
            {
                Platform::String^ get()
                {
                    return ref new Platform::String(L"LoggingEnabled");
                }
            }

            static property Platform::String^ LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME
            {
                Platform::String^ get()
                {
                    return ref new Platform::String(L"LogFileGeneratedBeforeSuspend");
                }
            }
           
            static void SetAppLocalSettingsValue(Platform::String^ key, Platform::Object^ value)
            {
                if (ApplicationData::Current->LocalSettings->Values->HasKey(key))
                {
                    ApplicationData::Current->LocalSettings->Values->Remove(key);
                }
                ApplicationData::Current->LocalSettings->Values->Insert(key, value);
            }

            static bool IsAppLocalSettingsValue(Platform::String^ key)
            {
                return ApplicationData::Current->LocalSettings->Values->HasKey(key);
            }

            static Platform::Object^ GetAppLocalSettingsValueAsObject(Platform::String^ key)
            {
                return ApplicationData::Current->LocalSettings->Values->Lookup(key);
            }

            static bool GetAppLocalSettingsValueAsBool(Platform::String^ key)
            {
                Platform::Object^ obj = GetAppLocalSettingsValueAsObject(key);
                return safe_cast<bool>(obj);
            }

            static Platform::String^ GetAppLocalSettingsValueAsString(Platform::String^ key)
            {
                Platform::Object^ obj = GetAppLocalSettingsValueAsObject(key);
                return safe_cast<Platform::String^>(obj);
            }

            static LoggingScenario^ GetLoggingScenarioSingleton()
            {
                if (loggingScenario == nullptr)
                {
                    loggingScenario = ref new LoggingScenario();
                }
                return loggingScenario;
            }

            property bool IsBusy
            {
                bool get() { return _isBusy; }
            private: 
                void set(bool value)
                {
                    _isBusy = value;
                    StatusChanged(this, ref new LoggingScenarioEventArgs(LoggingScenarioEventType::BusyStatusChanged));
                }
            }

            property int LogMessageCount
            {
                int get() { return _logMessageCount; }
            private:
                void set(int value) 
                { 
                    _logMessageCount = value; 
                    if (_logMessageCount % 500 == 0)
                    {
                        StatusChanged(this, ref new LoggingScenarioEventArgs(LoggingScenarioEventType::LogMessageCountUpdate));
                    }
                }
            }

            property int LogMessageApproximateByteCount
            {
                int get() { return _logMessageApproximateByteCount; }
            private:
                void set(int value) { _logMessageApproximateByteCount = value; }
            }

            property int LogFileGeneratedCount
            {
                int get() { return _logFileGeneratedCount; }
            }            

            property bool LoggingEnabled
            {
                bool get() { return _session != nullptr; }
            }

            task<bool> ToggleLoggingEnabledDisabledAsync();
            void StartLogging();
            void LogMessage(Platform::String^ message, Windows::Foundation::Diagnostics::LoggingLevel level);
            void LogValuePair(Platform::String^ value1, int value2, Windows::Foundation::Diagnostics::LoggingLevel level);
            bool IsLoggingChannelEnabledForLevel(Windows::Foundation::Diagnostics::LoggingLevel level);

            task<void> PrepareToSuspendAsync();
            void ResumeLoggingIfApplicable();

            task<void> DoScenarioAsync();

        private:

            ~LoggingScenario();

		private:

			static LoggingScenario^ loggingScenario;
			Windows::Foundation::Diagnostics::FileLoggingSession^ _session;
			Windows::Foundation::Diagnostics::LoggingChannel^ _channel;

			int _logMessageCount;
            long _logMessageApproximateByteCount;
            long _logFileGeneratedCount;
            bool _isSuspending;
            bool _isBusy;

			// The following are set when a LoggingChannel 
			// calls OnChannelLoggingEnabled:
			bool _isChannelEnabled;
			LoggingLevel _channelLoggingLevel;
			Windows::Foundation::EventRegistrationToken _onChannelLoggingEnabledToken;
            void OnChannelLoggingEnabled(ILoggingChannel ^sender, Platform::Object ^args);

			Windows::Foundation::EventRegistrationToken _onLogFileGeneratedToken;
			void OnLogFileGenerated(IFileLoggingSession^ sender, LogFileGeneratedEventArgs^ args);

            critical_section _lockForMoveFile;
            task<Platform::String^> MoveFileToRepositoryAsync(StorageFile^ sourceFile);
            task<Platform::String^> GetNextLogFileNameAsync();
            task<int> GetNextLogFileNumberAsync();
			task<IVectorView<StorageFile^>^> GetAllLogRepositoryFileNamesAsync();
			task<StorageFolder^> GetLogRepositoryFolderAsync();
            task<Platform::String^> SaveFinalLogFileAsync(Windows::Foundation::Diagnostics::FileLoggingSession^ sessionToCloseAndSave);
            void OnAppSuspending(Platform::Object ^sender, Windows::ApplicationModel::SuspendingEventArgs ^e);
            void OnAppResuming(Platform::Object ^sender, Platform::Object ^args);
};
	}
}
