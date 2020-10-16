﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// ShareDelayRenderedFiles.xaml.h
// Declaration of the ShareDelayRenderedFiles class
//

#pragma once

#include "pch.h"
#include "SharePage.h"
#include "ShareDelayRenderedFiles.g.h"
#include "MainPage.xaml.h"

namespace SDKSample
{
    namespace ShareSource
    {
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class ShareDelayRenderedFiles sealed
        {
        public:
            ShareDelayRenderedFiles();
    
        protected:
            virtual bool GetShareContent(Windows::ApplicationModel::DataTransfer::DataRequest^ request) override;
    
        private:
            MainPage^ rootPage;
            Windows::Storage::StorageFile^ imageFile;
    
            void OnDeferredImageRequestedHandler(Windows::ApplicationModel::DataTransfer::DataProviderRequest^ request);
            void SelectImageButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        };
    }
}
