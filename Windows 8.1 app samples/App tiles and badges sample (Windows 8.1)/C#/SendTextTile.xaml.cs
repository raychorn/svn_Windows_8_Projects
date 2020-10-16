//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using SDKTemplate;
using System;
using NotificationsExtensions.TileContent;
using Windows.UI.Notifications;
using System.Xml;
using System.Text;
using Windows.Data.Xml.Dom;
using System.IO;
using System.Xml.Linq;

namespace Tiles
{
    public sealed partial class SendTextTile : SDKTemplate.Common.LayoutAwarePage
    {
        #region TemplateCode
        MainPage rootPage = MainPage.Current;

        public SendTextTile()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }
        #endregion TemplateCode

        private void UpdateTileWithText_Click(object sender, RoutedEventArgs e)
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

            // Create notification TileSquare310x310Text05 content based on a visual template.
            ITileSquare310x310Text05 tileContent = TileContentFactory.CreateTileSquare310x310Text05();
            tileContent.TextHeading.Text = "Hello World! My very own tile notification";

            // create the notification for a wide310x150 template.
            ITileWide310x150Text03 wide310x150Content = TileContentFactory.CreateTileWide310x150Text03();
            wide310x150Content.TextHeadingWrap.Text = "Hello World! My very own tile notification";

            // create the square150x150 template and attach it to the wide310x150 template.
            ITileSquare150x150Text04 square150x150Content = TileContentFactory.CreateTileSquare150x150Text04();
            square150x150Content.TextBodyWrap.Text = "Hello World! My very own tile notification";
            wide310x150Content.Square150x150Content = square150x150Content;

            // attach the wide310x150 template to the square310x310 template.
            tileContent.Wide310x150Content = wide310x150Content;

            // send the notification
            TileUpdateManager.CreateTileUpdaterForApplication().Update(tileContent.CreateNotification());

            OutputTextBlock.Text = MainPage.PrettyPrint(tileContent.GetContent());
        }

        private void UpdateTileWithTextWithStringManipulation_Click(object sender, RoutedEventArgs e)
        {
            // create a string with the tile template xml
            string tileXmlString = "<tile>"
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
            Windows.Data.Xml.Dom.XmlDocument tileDOM = new Windows.Data.Xml.Dom.XmlDocument();
            // load the xml string into the DOM, catching any invalid xml characters 
            tileDOM.LoadXml(tileXmlString);

            // create a tile notification
            TileNotification tile = new TileNotification(tileDOM);

            // send the notification to the app's application tile
            TileUpdateManager.CreateTileUpdaterForApplication().Update(tile);

            OutputTextBlock.Text = MainPage.PrettyPrint(tileDOM.GetXml());
        }

        private void ClearTile_Click(object sender, RoutedEventArgs e)
        {
            TileUpdateManager.CreateTileUpdaterForApplication().Clear();
            OutputTextBlock.Text = "Tile cleared";
        }
    }
}