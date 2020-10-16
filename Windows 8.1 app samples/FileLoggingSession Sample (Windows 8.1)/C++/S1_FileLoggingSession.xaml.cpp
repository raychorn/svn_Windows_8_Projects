//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S1_FileLoggingSession.xaml.cpp
// Implementation of the S1_FileLoggingSession class
//

#include "pch.h"
#include "S1_FileLoggingSession.xaml.h"
#include "MainPage.xaml.h"
#include "LoggingScenario.h"

using namespace SDKSample;
using namespace SDKSample::FileLoggingSession;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

S1_FileLoggingSession::S1_FileLoggingSession()
{
    LoggingScenario->StatusChanged += ref new StatusChangedHandler(this, &S1_FileLoggingSession::OnStatusChanged);
    InitializeComponent();
}

void S1_FileLoggingSession::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
    UpdateControls();
}

Platform::String^ GetJustFileName(Platform::String^ path)
{
    Platform::String^ extractJustFileNameString = L"[\\\\\\/]([^\\\\\\/]+)$";
    std::wregex extractJustFileNameRegEx(extractJustFileNameString->Data());
    std::wsmatch m;
    std::wstring pathStr(path->Data());
    if (std::regex_search(pathStr, m, extractJustFileNameRegEx))
    {
        return ref new Platform::String(m[1].str().c_str());
    }
    return ref new Platform::String();
}

Platform::String^ GetJustDirectoryName(Platform::String^ path)
{
    Platform::String^ extractJustFileNameString = L"(.*)[\\\\\\/]([^\\\\\\/]+)$";
    std::wregex extractJustFileNameRegEx(extractJustFileNameString->Data());
    std::wsmatch m;
    std::wstring pathStr(path->Data());
    if (std::regex_search(pathStr, m, extractJustFileNameRegEx))
    {
        return ref new Platform::String(m[1].str().c_str());
    }
    return ref new Platform::String();
}


Windows::UI::Xaml::Controls::ScrollViewer^ S1_FileLoggingSession::FindScrollViewer(DependencyObject^ depObject)
{
    if (depObject == nullptr)
    {
        return nullptr;
    }

    int countThisLevel = Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(depObject);
    if (countThisLevel <= 0)
    {
        return nullptr;
    }

    for (int childIndex = 0; childIndex < countThisLevel; childIndex++)
    {
        DependencyObject^ childDepObject = Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(depObject, childIndex);
        ScrollViewer^ sv = dynamic_cast<ScrollViewer^>(childDepObject);
        if (sv != nullptr)
        {
            return sv;
        }

        sv = FindScrollViewer(childDepObject);
        if (sv != nullptr)
        {
            return sv;
        }
    }

    return nullptr;
}


void S1_FileLoggingSession::AddMessage(Platform::String^ message)
{
    Platform::String^ messageToAdd = "";
    messageToAdd += message;
    messageToAdd += "\r\n";
    StatusMessageList->Text += messageToAdd;
    StatusMessageList->Select(StatusMessageList->Text->Length(), 0);

    ScrollViewer^ sv = FindScrollViewer(StatusMessageList);
    if (sv != nullptr)
    {
        sv->ChangeView(nullptr, StatusMessageList->ActualHeight, nullptr);
    }
    
}

task<void> S1_FileLoggingSession::AddMessageDispatch(Platform::String^ message)
{
    return create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=](){
        AddMessage(message);
    })));
}

task<void> S1_FileLoggingSession::AddLogFileMessageDispatch(Platform::String^ message, Platform::String^ path)
{
    return create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=](){
        Platform::String^ finalMessage;
        if (path != nullptr && path->IsEmpty() == false)
        {
            finalMessage = message + ": " + GetJustFileName(path);
            AppLogFolderLabel->Visibility = Windows::UI::Xaml::Visibility::Visible;
            AppLogFolder->Visibility = Windows::UI::Xaml::Visibility::Visible;
            AppLogFolder->Text = GetJustDirectoryName(path);

            ViewLogInfo->Visibility = Windows::UI::Xaml::Visibility::Visible;
            ViewLogInfo->Text = "To view the contents of the ETL files:\r\n" +
                                "Using tracerpt to create an XML file: tracerpt.exe \"" +
                                path +
                                "\" -of XML -o LogFile.xml\r\n" +
                                "Using the Windows Performance Toolkit (WPT): wpa.exe \"" +
                                path + "\"";
        }
        else
        {
            finalMessage = message + ": none, nothing logged since saving the last file.";
        }
        AddMessage(finalMessage);
    })));
}

task<void> S1_FileLoggingSession::UpdateLogMessageCountDispatchAsync()
{
    int messageCount = LoggingScenario->LogMessageCount;
    long approximateByteCount = LoggingScenario->LogMessageApproximateByteCount;

    return create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=](){
        LoggingStatisticsTextBlock->Text =
            "Total messages logged: " + messageCount.ToString() + "  Approximate bytes logged: " + approximateByteCount.ToString();
    })));
}


void S1_FileLoggingSession::OnStatusChanged(SDKSample::FileLoggingSession::LoggingScenario ^sender, SDKSample::FileLoggingSession::LoggingScenarioEventArgs ^e)
{
    if (e->Type == LoggingScenarioEventType::BusyStatusChanged)
    {
        create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=](){
            UpdateControls();
        })));
    }
    else if (e->Type == LoggingScenarioEventType::LogFileGenerated)
    {
        AddLogFileMessageDispatch("LogFileGenerated", e->LogFilePath).then([](){
        });
    }
    else if (e->Type == LoggingScenarioEventType::LogFileGeneratedAtSuspend)
    {
        AddLogFileMessageDispatch("Log file at suspend", e->LogFilePath).then([](){
        });
    }
    else if (e->Type == LoggingScenarioEventType::LogFileGeneratedAtDisable)
    {
        AddLogFileMessageDispatch("Log file at disable", e->LogFilePath).then([](){
        });
    }
    else if (e->Type == LoggingScenarioEventType::LoggingEnabledDisabled)
    {
        Platform::String^ message = "Logging has been " + (e->Enabled ? "enabled" : "disabled") + ".";
        AddMessageDispatch(message);
    }
    else if (e->Type == LoggingScenarioEventType::LogMessageCountUpdate)
    {
        UpdateLogMessageCountDispatchAsync().then([](){
        });
    }
}

void S1_FileLoggingSession::UpdateControls()
{
    if (LoggingScenario->LoggingEnabled)
    {
        InputTextBlock1->Text = "Logging is enabled. Click 'Disable Logging' to disable logging. With logging enabled, you can click 'Log Messages' to use the logging API to generate log files.";
        EnableDisableLoggingButton->Content = "Disable Logging";
        DoScenarioButton->IsEnabled = true;
    }
    else
    {
        InputTextBlock1->Text = "Logging is disabled. Click 'Enable Logging' to enable logging. After you enable logging you can click 'Log Messages' to use the logging API to generate log files.";
        EnableDisableLoggingButton->Content = "Enable Logging";
        DoScenarioButton->IsEnabled = false;
    }

    if (LoggingScenario->IsBusy)
    {
        EnableDisableLoggingButton->IsEnabled = false;
        DoScenarioButton->IsEnabled = false;
    }
    else
    {
        EnableDisableLoggingButton->IsEnabled = true;
    }
}

void S1_FileLoggingSession::EnableDisableLogging(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    LoggingScenario->ToggleLoggingEnabledDisabledAsync().then([this](bool isEnabled) {
        UpdateControls();
    }, task_continuation_context::use_current());
}

void S1_FileLoggingSession::DoScenario(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    DoScenarioButton->IsEnabled = false;
    create_task(DoScenarioButton->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([=](){}))).then([this]()
    {
        return LoggingScenario->DoScenarioAsync();
    }).then([this](task<void> previousTask){
        DoScenarioButton->IsEnabled = true;
    });
}
