//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// App.xaml.h
// Declaration of the App.xaml class.
//

#pragma once

#include "pch.h"
#include "App.g.h"
#include "MainPage.g.h"

namespace SDKSample
{
    ref class App
    {
    internal:
        App();
        virtual void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ pArgs);
        Concurrency::task<Windows::UI::Core::CoreDispatcher^> CreateNewView();

    protected:
        virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ pArgs) override;
        virtual void OnWindowCreated(Windows::UI::Xaml::WindowCreatedEventArgs^ pArgs) override;

    private:
        Concurrency::concurrent_queue<std::shared_ptr<Concurrency::task_completion_event<Windows::UI::Core::CoreDispatcher^>>> viewCreationTasks;
    };
}
