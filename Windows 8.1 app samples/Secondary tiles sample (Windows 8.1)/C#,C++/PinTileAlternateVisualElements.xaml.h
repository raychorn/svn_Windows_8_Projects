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
// PinTileAlternateVisualElements.xaml.h
// Declaration of the PinTileAlternateVisualElements class
//

#pragma once

#include "pch.h"
#include "PinTileAlternateVisualElements.g.h"
#include "MainPage.xaml.h"

namespace SecondaryTiles
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class PinTileAlternateVisualElements sealed
    {
    public:
        PinTileAlternateVisualElements();

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
        virtual void OnNavigatingFrom(Windows::UI::Xaml::Navigation::NavigatingCancelEventArgs^ e) override;
    private:
        MainPage^ rootPage;
        Windows::UI::Xaml::Controls::AppBar^ appBar;
        void PinButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void VisualElementsRequestedHandler(Windows::UI::StartScreen::SecondaryTile^ tile, Windows::UI::StartScreen::VisualElementsRequestedEventArgs^ args);
    };
}
