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
// Globalization.xaml.cpp
// Implementation of the Globalization class
//

#include "pch.h"
#include "Globalization.xaml.h"

using namespace SDKSample::Tiles;

using namespace Platform;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Notifications;
using namespace NotificationsExtensions::TileContent;
using namespace Windows::ApplicationModel::Resources::Core;

Globalization::Globalization()
{
    InitializeComponent();
}

void Globalization::OnNavigatedTo(NavigationEventArgs^ e)
{
    rootPage = MainPage::Current;
}

void Globalization::ViewCurrentResources_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{	
	ResourceContext^ defaultContextForCurrentView = ResourceContext::GetForCurrentView();

	String^ language = defaultContextForCurrentView->QualifierValues->Lookup("Language");

	String^ scale = defaultContextForCurrentView->QualifierValues->Lookup("Scale");

	String^ contrast = defaultContextForCurrentView->QualifierValues->Lookup("Contrast");

	OutputTextBlock->Text = "Your system is currently set to the following values: Application Language: " + language + ", Scale: " + scale + ", Contrast: " + contrast + ". If using web images and addImageQuery, the following query string would be appened to the URL: ?ms-lang=" + language + "&ms-scale=" + scale + "&ms-contrast=" + contrast;
}

void Globalization::SendTileNotificationWithQueryStrings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{	
	auto wide310x150TileContent = TileContentFactory::CreateTileWide310x150ImageAndText01();
	wide310x150TileContent->TextCaptionWrap->Text = "This tile notification uses query strings for the image src.";

	wide310x150TileContent->Image->Src = ImageUrl->Text;
	wide310x150TileContent->Image->Alt = "Web image";

	// enable AddImageQuery on the notification
    wide310x150TileContent->AddImageQuery = true;

	auto square150x150TileContent = TileContentFactory::CreateTileSquare150x150Image();

	square150x150TileContent->Image->Src = ImageUrl->Text;
	square150x150TileContent->Image->Alt = "Web image";

	// include the square template.
	wide310x150TileContent->Square150x150Content = square150x150TileContent;

	// Send the notification to the app's application tile
	TileUpdateManager::CreateTileUpdaterForApplication()->Update(wide310x150TileContent->CreateNotification());

	OutputTextBlock->Text = wide310x150TileContent->GetContent();
}

void Globalization::SendScaledImageTileNotification_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{	
	String^ scale = ResourceContext::GetForCurrentView()->QualifierValues->Lookup("Scale");

	auto wide310x150TileContent = TileContentFactory::CreateTileWide310x150SmallImageAndText03(); 

	wide310x150TileContent->TextBodyWrap->Text = "blueWide310x150.png in the xml is actually blueWide310x150.scale-" + scale + ".png";
	wide310x150TileContent->Image->Src = "ms-appx:///blueWide310x150.png";
	wide310x150TileContent->Image->Alt = "Blue wide";

	auto square150x150TileContent = TileContentFactory::CreateTileSquare150x150Image();
	square150x150TileContent->Image->Src = "ms-appx:///graySquare150x150.png";
	square150x150TileContent->Image->Alt = "Gray square";
	wide310x150TileContent->Square150x150Content = square150x150TileContent;

	TileUpdateManager::CreateTileUpdaterForApplication()->Update(wide310x150TileContent->CreateNotification());

	OutputTextBlock->Text = wide310x150TileContent->GetContent();
}

void Globalization::SendTextResourceTileNotification_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{	
	auto wide310x150TileContent = TileContentFactory::CreateTileWide310x150Text03();
	// check out /en-US/resources.resw to understand where this string will come from
	wide310x150TileContent->TextHeadingWrap->Text = "ms-resource:greeting";

	auto square150x150TileContent = TileContentFactory::CreateTileSquare150x150Text04();
	// check out /en-US/resources.resw to understand where this string will come from 
	square150x150TileContent->TextBodyWrap->Text = "ms-resource:greeting";
	wide310x150TileContent->Square150x150Content = square150x150TileContent;

	TileUpdateManager::CreateTileUpdaterForApplication()->Update(wide310x150TileContent->CreateNotification());

	OutputTextBlock->Text = wide310x150TileContent->GetContent();
}
