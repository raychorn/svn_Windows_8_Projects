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
// SendLocalImageTile.xaml.cpp
// Implementation of the SendLocalImageTile class
//

#include "pch.h"
#include "SendLocalImageTile.xaml.h"

using namespace SDKSample::Tiles;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Notifications;
using namespace NotificationsExtensions::TileContent;

SendLocalImageTile::SendLocalImageTile()
{
	InitializeComponent();
}

void SendLocalImageTile::OnNavigatedTo(NavigationEventArgs^ e)
{
	rootPage = MainPage::Current;
}

void SendLocalImageTile::UpdateTileWithImage_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// Note: This sample contains an additional project, NotificationsExtensions.
	// NotificationsExtensions exposes an object model for creating notifications, but you can also 
	// modify the strings directly. See UpdateTileWithImageWithStringManipulation_Click for an example

	// Users can resize tiles to large (Square310x310), wide (Wide310x150), medium (Square150x150) or small (Square70x70).
	// Apps can choose not to support all tile sizes (i.e. the app's tile can prevent being resized to wide or medium)
	// Supporting a large (Square310x310) tile requires supporting wide (Wide310x150) tile.

	// This sample application supports a Square150x150, Wide310x150 and Square310x310 Start tile. 
	// The user may have selected any of those sizes for their custom Start screen layout, so each 
	// notification should include template bindings for each supported tile size. (The Square70x70 tile
	// size does not support receiving live tile notifications, so we don't need a binding for that size.)
	// We assemble one notification with three template bindings by including the content for each smaller
	// tile in the next size up. Square310x310 includes Wide310x150, which includes Square150x150.
	// If we leave off the content for a tile size which the application supports, the user will not see the
	// notification if the tile is set to that size.

	// Create notification square310x310 content based on a visual template.
	auto tileContent = TileContentFactory::CreateTileSquare310x310Image();
	tileContent->Image->Src = "ms-appx:///purpleSquare310x310.png";
	tileContent->Image->Alt = "Purple image";

	// create the notification for a wide310x150 template.
	auto wide310x150Content = TileContentFactory::CreateTileWide310x150ImageAndText01();
	wide310x150Content->TextCaptionWrap->Text = "This tile notification uses ms-appx images";
	wide310x150Content->Image->Src = "ms-appx:///redWide310x150.png";
	wide310x150Content->Image->Alt = "Red image";

	// create the square150x150 template and attach it to the wide310x150 template.
	auto square150x150Content = TileContentFactory::CreateTileSquare150x150Image();
	square150x150Content->Image->Src = "ms-appx:///graySquare150x150.png";
	square150x150Content->Image->Alt = "Gray image";
	wide310x150Content->Square150x150Content = square150x150Content;

	// add the wide310x150 to the Square310x310 template.
	tileContent->Wide310x150Content = wide310x150Content;

	// Send the notification to the app's application tile
	TileUpdateManager::CreateTileUpdaterForApplication()->Update(tileContent->CreateNotification());

	OutputTextBlock->Text = tileContent->GetContent();
}

void SendLocalImageTile::UpdateTileWithImageWithStringManipulation_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// create a string with the tile template xml
	auto tileXmlString = "<tile>"
		+ "<visual version='2'>"
		+ "<binding template='TileSquare150x150Image' fallback='TileSquareImage'>"
		+ "<image id='1' src='ms-appx:///graySquare150x150.png' alt='Gray image'/>"
		+ "</binding>"
		+ "<binding template='TileWide310x150ImageAndText01' fallback='TileWideImageAndText01'>"
		+ "<image id='1' src='ms-appx:///redWide310x150.png' alt='Red image'/>"
		+ "<text id='1'>This tile notification uses ms-appx images</text>"
		+ "</binding>"
		+ "<binding template='TileSquare310x310Image'>"
		+ "<image id='1' src='ms-appx:///purpleSquare310x310.png' alt='Purple image'/>"
		+ "</binding>"
		+ "</visual>"
		+ "</tile>";

	// create a DOM
	auto tileDOM = ref new Windows::Data::Xml::Dom::XmlDocument();

	// load the xml string into the DOM, catching any invalid xml characters 
	tileDOM->LoadXml(tileXmlString);

	// create a tile notification
	auto tile = ref new TileNotification(tileDOM);

	// Send the notification to the app's application tile
	TileUpdateManager::CreateTileUpdaterForApplication()->Update(tile);

	OutputTextBlock->Text = tileDOM->GetXml();
}

void SendLocalImageTile::ClearTile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// A TileUpdater is also used to clear the tile        
	TileUpdateManager::CreateTileUpdaterForApplication()->Clear();
	OutputTextBlock->Text = "Tile cleared";
}