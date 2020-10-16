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

#define DISABLE_MAIN_VIEW_KEY "DisableShowingMainViewOnActivation"

namespace SDKSample
{
    ref class App
    {
    internal:
        App();
        virtual void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ pArgs);

        property Windows::Foundation::Collections::IVector<SDKSample::MultipleViews::ViewData^>^ SecondaryViews
        {
            Windows::Foundation::Collections::IVector<SDKSample::MultipleViews::ViewData^>^ get();
        };

        void UpdateTitle(Platform::String^ newTitle, int viewId);
        property Windows::UI::Core::CoreDispatcher^ MainDispatcher
        {
            Windows::UI::Core::CoreDispatcher^ get();
        };
        property int MainViewId
        {
            int get();
        }

        Concurrency::task<Windows::UI::Core::CoreDispatcher^> CreateNewView();

    protected:
        virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ pArgs) override;
        virtual void OnActivated(Windows::ApplicationModel::Activation::IActivatedEventArgs^ pArgs) override;
        virtual void OnWindowCreated(Windows::UI::Xaml::WindowCreatedEventArgs^ pArgs) override;
    private:
        bool TryFindViewDataForViewId(int viewId, SDKSample::MultipleViews::ViewData^* foundData);
        Concurrency::task<void> InitializeMainPage(Windows::ApplicationModel::Activation::ApplicationExecutionState previousExecutionState,
                                Platform::String^ arguments);
        Platform::Collections::Vector<SDKSample::MultipleViews::ViewData^>^ secondaryViews;
        Windows::UI::Core::CoreDispatcher^ mainDispatcher;
        int mainViewId;
        Concurrency::concurrent_queue<std::shared_ptr<Concurrency::task_completion_event<Windows::UI::Core::CoreDispatcher^>>> viewCreationTasks;
    };
}
