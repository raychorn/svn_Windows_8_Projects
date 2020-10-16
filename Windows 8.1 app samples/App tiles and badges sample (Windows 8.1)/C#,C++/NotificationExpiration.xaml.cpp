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
// NotificationExpiration.xaml.cpp
// Implementation of the NotificationExpiration class
//

#include "pch.h"
#include "NotificationExpiration.xaml.h"

using namespace SDKSample::Tiles;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Notifications;
using namespace NotificationsExtensions::TileContent;

NotificationExpiration::NotificationExpiration()
{
    InitializeComponent();
}

void NotificationExpiration::OnNavigatedTo(NavigationEventArgs^ e)
{
    rootPage = MainPage::Current;
}

void NotificationExpiration::UpdateTileExpiring_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	int seconds = _wtoi(Time->Text->Data());
	if (seconds == 0)
	{ 
		seconds = 10;
	}

	Windows::Globalization::Calendar^ cal = ref new Windows::Globalization::Calendar();
	cal->SetToNow();
	cal->AddSeconds(seconds);

	auto longTime = ref new Windows::Globalization::DateTimeFormatting::DateTimeFormatter("longtime");
	DateTime time = cal->GetDateTime();
	Platform::String^ result = longTime->Format(time);

	auto wide310x150TileContent = TileContentFactory::CreateTileWide310x150Text04();
	wide310x150TileContent->TextBodyWrap->Text = "This notification will expire at "+ result;

	auto square150x150TileContent = TileContentFactory::CreateTileSquare150x150Text04();
	square150x150TileContent->TextBodyWrap->Text = "This notification will expire at " + result;
	wide310x150TileContent->Square150x150Content = square150x150TileContent;

	auto tileNotification = wide310x150TileContent->CreateNotification();

	// set the expirationTime
	tileNotification->ExpirationTime = dynamic_cast<Platform::IBox<DateTime>^>(PropertyValue::CreateDateTime(time));

	TileUpdateManager::CreateTileUpdaterForApplication()->Update(tileNotification);
	OutputTextBlock->Text = "Tile notification sent. It will expire at " + result;
}
