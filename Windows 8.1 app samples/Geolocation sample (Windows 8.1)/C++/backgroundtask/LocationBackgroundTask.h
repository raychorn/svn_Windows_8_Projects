//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "pch.h"

namespace BackgroundTask
{
    enum GeofenceReason
    {
        Add,
        Entered,
        Exited,
        Expired,
        Used
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class LocationBackgroundTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
        LocationBackgroundTask();

        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);

    private:
        void OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);
        ~LocationBackgroundTask();

        Concurrency::cancellation_token_source geopositionTaskTokenSource;
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class GeofenceBackgroundTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
        GeofenceBackgroundTask();

        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);
        void Invoke(Windows::Foundation::IAsyncAction^ asyncInfo, Windows::Foundation::AsyncStatus asyncStatus);

    private:
        void OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);
        void WipeGeofenceDataFromAppdata();
        void WriteStatusToAppdata(Platform::String^ status);
        Concurrency::task<void> GetGeofenceStateChangedReportsAsync();
        ~GeofenceBackgroundTask();

        Concurrency::cancellation_token_source geopositionTaskTokenSource;
    };
}
