//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using NotificationsExtensions.TileContent;
using SDKTemplate;
using Windows.UI.Notifications;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Navigation;

namespace Tiles
{
    public sealed partial class SendLocalImageTile : SDKTemplate.Common.LayoutAwarePage
    {
        #region TemplateCode
        MainPage rootPage = MainPage.Current;

        public SendLocalImageTile()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }
        #endregion TemplateCode

        void UpdateTileWithImage_Click(object sender, RoutedEventArgs e)
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
            ITileSquare310x310Image tileContent = TileContentFactory.CreateTileSquare310x310Image();
            tileContent.Image.Src = "ms-appx:///images/purpleSquare310x310.png";
            tileContent.Image.Alt = "Purple image";

            // create the notification for a wide310x150 template.
            ITileWide310x150ImageAndText01 wide310x150Content = TileContentFactory.CreateTileWide310x150ImageAndText01();
            wide310x150Content.TextCaptionWrap.Text = "This tile notification uses ms-appx images";
            wide310x150Content.Image.Src = "ms-appx:///images/redWide310x150.png";
            wide310x150Content.Image.Alt = "Red image";

            // create the square150x150 template and attach it to the wide310x150 template.
            ITileSquare150x150Image square150x150Content = TileContentFactory.CreateTileSquare150x150Image();
            square150x150Content.Image.Src = "ms-appx:///images/graySquare150x150.png";
            square150x150Content.Image.Alt = "Gray image";

            // add the square150x150 template to the wide310x150 template.
            wide310x150Content.Square150x150Content = square150x150Content;

            // add the wide310x150 to the Square310x310 template.
            tileContent.Wide310x150Content = wide310x150Content;

            // send the notification to the app's application tile.
            TileUpdateManager.CreateTileUpdaterForApplication().Update(tileContent.CreateNotification());

            OutputTextBlock.Text = MainPage.PrettyPrint(tileContent.GetContent());
        }

        void UpdateTileWithImageWithStringManipulation_Click(object sender, RoutedEventArgs e)
        {
            // create a string with the tile template xml
            string tileXmlString = "<tile>"
                              + "<visual version='2'>"
                              + "<binding template='TileSquare150x150Image' fallback='TileSquareImage'>"
                              + "<image id='1' src='ms-appx:///images/graySquare150x150.png' alt='Gray image'/>"
                              + "</binding>"
                              + "<binding template='TileWide310x150ImageAndText01' fallback='TileWideImageAndText01'>"
                              + "<image id='1' src='ms-appx:///images/redWide310x150.png' alt='Red image'/>"
                              + "<text id='1'>This tile notification uses ms-appx images</text>"
                              + "</binding>"
                              + "<binding template='TileSquare310x310Image'>"
                              + "<image id='1' src='ms-appx:///images/purpleSquare310x310.png' alt='Purple image'/>"
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