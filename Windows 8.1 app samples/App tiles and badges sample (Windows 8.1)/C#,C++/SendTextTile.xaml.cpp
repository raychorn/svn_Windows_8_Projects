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
// SendTextTile.xaml.cpp
// Implementation of the SendTextTile class
//

#include "pch.h"
#include "SendTextTile.xaml.h"

using namespace SDKSample::Tiles;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Notifications;
using namespace NotificationsExtensions::TileContent;

SendTextTile::SendTextTile()
{
	InitializeComponent();
}

void SendTextTile::OnNavigatedTo(NavigationEventArgs^ e)
{
	rootPage = MainPage::Current;
}

void SendTextTile::UpdateTileWithText_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// Note: This sample contains an additional project, NotificationsExtensions.
	// NotificationsExtensions exposes an object model for creating notifications, but you can also 
	// modify the strings directly. See UpdateTileWithTextWithStringManipulation_Click for an example.

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
	auto tileContent = TileContentFactory::CreateTileSquare310x310Text05();
	tileContent->TextHeading->Text = "Hello World! My very own tile notification";

	// create the notification for a wide310x150 template.
	auto wide310x150Content = TileContentFactory::CreateTileWide310x150Text03();
	wide310x150Content->TextHeadingWrap->Text = "Hello World! My very own tile notification";

	// create the square150x150 template and attach it to the wide310x150 template.
	auto square150x150Content = TileContentFactory::CreateTileSquare150x150Text04();
	square150x150Content->TextBodyWrap->Text = "Hello World! My very own tile notification";
	wide310x150Content->Square150x150Content = square150x150Content;

	// attach the wide310x150 template to the square310x310 template.
	tileContent->Wide310x150Content = wide310x150Content;

	// Send the notification to the app's application tile
	TileUpdateManager::CreateTileUpdaterForApplication()->Update(tileContent->CreateNotification());

	OutputTextBlock->Text = tileContent->GetContent();
}

void SendTextTile::UpdateTileWithTextWithStringManipulation_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// create a string with the tile template xml
	auto tileXmlString = "<tile>"
		+ "<visual version='2'>"
		+ "<binding template='TileSquare150x150Text04' fallback='TileSquareText04'>"
		+ "<text id='1'>Hello World! My very own tile notification</text>"
		+ "</binding>"
		+ "<binding template='TileWide310x150Text03' fallback='TileWideText03'>"
		+ "<text id='1'>Hello World! My very own tile notification</text>"
		+ "</binding>"
		+ "<binding template='TileSquare310x310Text05'>"
		+ "<text id='1'>Hello World! My very own tile notification</text>"
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

void SendTextTile::ClearTile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// A TileUpdater is also used to clear the tile        
	TileUpdateManager::CreateTileUpdaterForApplication()->Clear();
	OutputTextBlock->Text = "Tile cleared";
}