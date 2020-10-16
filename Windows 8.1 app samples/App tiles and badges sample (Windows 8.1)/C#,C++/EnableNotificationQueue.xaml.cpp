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
// EnableNotificationQueue.xaml.cpp
// Implementation of the EnableNotificationQueue class
//

#include "pch.h"
#include "EnableNotificationQueue.xaml.h"

using namespace SDKSample::Tiles;

using namespace Platform;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Notifications;
using namespace NotificationsExtensions::TileContent;

EnableNotificationQueue::EnableNotificationQueue()
{
	InitializeComponent();
}

void EnableNotificationQueue::OnNavigatedTo(NavigationEventArgs^ e)
{
	rootPage = MainPage::Current;
}

void EnableNotificationQueue::EnableNotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueue(true);
	OutputTextBlock->Text = "Notification cycling enabled for all tile sizes.";
}

void EnableNotificationQueue::DisableNotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueue(false);
	OutputTextBlock->Text = "Notification cycling disabled for all tile sizes.";
}

void EnableNotificationQueue::EnableSquare150x150NotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueueForSquare150x150(true);
	OutputTextBlock->Text = "Notification cycling enabled for Square150x150 Tiles.";
}

void EnableNotificationQueue::DisableSquare150x150NotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueueForSquare150x150(false);
	OutputTextBlock->Text = "Notification cycling disabled for Square150x150 Tiles.";
}

void EnableNotificationQueue::EnableWide310x150NotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueueForWide310x150(true);
	OutputTextBlock->Text = "Notification cycling enabled for Wide310x150 Tiles.";
}

void EnableNotificationQueue::DisableWide310x150NotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueueForWide310x150(false);
	OutputTextBlock->Text = "Notification cycling disabled for Wide310x150 Tiles.";
}

void EnableNotificationQueue::EnableSquare310x310NotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueueForSquare310x310(true);
	OutputTextBlock->Text = "Notification cycling enabled for Square310x310 Tiles.";
}

void EnableNotificationQueue::DisableSquare310x310NotificationQueue_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->EnableNotificationQueueForSquare310x310(false);
	OutputTextBlock->Text = "Notification cycling disabled for Square310x310 Tiles.";
}

void EnableNotificationQueue::UpdateTile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	String^ text = "TestTag01";
	if (TextContent->Text->IsEmpty())
	{
		text = TextContent->Text;
	}

	auto wide310x150TileContent = TileContentFactory::CreateTileWide310x150Text03();
	wide310x150TileContent->TextHeadingWrap->Text = text;

	auto square150x150TileContent = TileContentFactory::CreateTileSquare150x150Text04();
	square150x150TileContent->TextBodyWrap->Text = text;
	wide310x150TileContent->Square150x150Content = square150x150TileContent;

	auto tileNotification = wide310x150TileContent->CreateNotification();

	String^ tag = "TestTag01";
	if (!Id->Text->IsEmpty())
	{
		tag = Id->Text;
	}

	// set the tag on the notification
	tileNotification->Tag = tag;
	TileUpdateManager::CreateTileUpdaterForApplication()->Update(tileNotification);

	OutputTextBlock->Text = "Tile notification sent. It is tagged with '" + tag + "'.";
}

void EnableNotificationQueue::ClearTile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TileUpdateManager::CreateTileUpdaterForApplication()->Clear();
	OutputTextBlock->Text = "Tile cleared";
}
