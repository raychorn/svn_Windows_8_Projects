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
using Windows.UI.Notifications;
using NotificationsExtensions.TileContent;

namespace Tiles
{
    public sealed partial class SendWebImageTile : SDKTemplate.Common.LayoutAwarePage
    {
        #region TemplateCode
        MainPage rootPage = MainPage.Current;

        public SendWebImageTile()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }
        #endregion TemplateCode

        void UpdateTileWithWebImage_Click(object sender, RoutedEventArgs e)
        {
            // Note: This sample contains an additional project, NotificationsExtensions.
            // NotificationsExtensions exposes an object model for creating notifications, but you can also 
            // modify the strings directly. See UpdateTileWithWebImageWithStringManipulation_Click for an example.

            // !Important!
            // The Internet (Client) capability must be checked in the manifest in the Capabilities tab
            // to display web images in tiles (either the http:// or https:// protocols)

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
            tileContent.AddImageQuery = true;
            tileContent.Image.Src = ImageUrl.Text;
            tileContent.Image.Alt = "Web Image";

            // create the notification for a wide310x150 template.
            ITileWide310x150ImageAndText01 wide310x150Content = TileContentFactory.CreateTileWide310x150ImageAndText01();
            wide310x150Content.TextCaptionWrap.Text = "This tile notification uses web images.";
            wide310x150Content.Image.Src = ImageUrl.Text;
            wide310x150Content.Image.Alt = "Web image";

            // create the square150x150 template and attach it to the wide310x150 template.
            ITileSquare150x150Image square150x150Content = TileContentFactory.CreateTileSquare150x150Image();
            square150x150Content.Image.Src = ImageUrl.Text;
            square150x150Content.Image.Alt = "Web image";

            // add the square150x150 template to the wide310x150 template.
            wide310x150Content.Square150x150Content = square150x150Content;

            // add the wide310x150 to the Square310x310 template.
            tileContent.Wide310x150Content = wide310x150Content;

            // send the notification to the app's application tile.
            TileUpdateManager.CreateTileUpdaterForApplication().Update(tileContent.CreateNotification());

            OutputTextBlock.Text = MainPage.PrettyPrint(tileContent.GetContent());
        }

        void UpdateTileWithWebImageWithStringManipulation_Click(object sender, RoutedEventArgs e)
        {
            // create a string with the tile template xml
            string tileXmlString = "<tile>"
                             + "<visual version='2' addImageQuery='true'>"
                             + "<binding template='TileSquare150x150Image' fallback='TileSquareImage'>"
                             + "<image id='1' src='" + ImageUrl.Text + "' alt='Web image'/>"
                             + "</binding>"
                             + "<binding template='TileWide310x150ImageAndText01' fallback='TileWideImageAndText01'>"
                             + "<image id='1' src='" + ImageUrl.Text + "' alt='Web image'/>"
                             + "<text id='1'>This tile notification uses web images.</text>"
                             + "</binding>"
                             + "<binding template='TileSquare310x310Image'>"
                             + "<image id='1' src='" + ImageUrl.Text + "' alt='Web image'/>"
                             + "</binding>"
                             + "</visual>"
                             + "</tile>";

            // create a DOM
            Windows.Data.Xml.Dom.XmlDocument tileDOM = new Windows.Data.Xml.Dom.XmlDocument();
            try
            {
                // load the xml string into the DOM, catching any invalid xml characters 
                tileDOM.LoadXml(tileXmlString);

                // create a tile notification
                TileNotification tile = new TileNotification(tileDOM);

                // send the notification to the app's application tile
                TileUpdateManager.CreateTileUpdaterForApplication().Update(tile);

                OutputTextBlock.Text = MainPage.PrettyPrint(tileDOM.GetXml());
            }
            catch (Exception)
            {
                OutputTextBlock.Text = "Error loading the xml, check for invalid characters in the input";
            }
        }

        private void ClearTile_Click(object sender, RoutedEventArgs e)
        {
            TileUpdateManager.CreateTileUpdaterForApplication().Clear();
            OutputTextBlock.Text = "Tile cleared";
        }
    }
}