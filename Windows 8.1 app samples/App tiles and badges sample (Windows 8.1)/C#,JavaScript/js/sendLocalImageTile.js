//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/sendLocalImageTile.html", {
        ready: function (element, options) {
            document.getElementById("sendTileLocalImageNotification").addEventListener("click", sendTileLocalImageNotification, false);
            document.getElementById("sendTileLocalImageNotificationWithStringManipulation").addEventListener("click", sendTileLocalImageNotificationWithStringManipulation, false);
            document.getElementById("sendTileLocalImageNotificationWithXmlManipulation").addEventListener("click", sendTileLocalImageNotificationWithXmlManipulation, false);
            document.getElementById("clearTileNotification").addEventListener("click", clearTileNotification, false);
        }
    });

    function clearTileNotification() {
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().clear();
        WinJS.log && WinJS.log("Tile cleared", "sample", "status");
    }

    function sendTileLocalImageNotification() {
        // Note: This sample contains an additional project, NotificationsExtensions.
        // NotificationsExtensions exposes an object model for creating notifications, but you can also modify the xml
        // of the notification directly. See the additional function sendTileLocalImageNotificationWithXml to see how
        // to do it by modifying Xml directly, or sendLocalImageNotificationWithStringManipulation to see how to do it
        // by modifying strings directly.

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
        var tileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare310x310Image();
        tileContent.image.src = "ms-appx:///images/purpleSquare310x310.png";
        tileContent.image.alt = "Purple image";

        // create the notification for a wide310x150 template.
        var wide310x150Content = NotificationsExtensions.TileContent.TileContentFactory.createTileWide310x150ImageAndText01();
        wide310x150Content.textCaptionWrap.text = "This tile notification uses ms-appx images";
        wide310x150Content.image.src = "ms-appx:///images/redWide310x150.png";
        wide310x150Content.image.alt = "Red image";

        // create the square150x150 template and attach it to the wide310x150 template.
        var square150x150Content = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare150x150Image();
        square150x150Content.image.src = "ms-appx:///images/graySquare150x150.png";
        square150x150Content.image.alt = "Gray image";

        // add the square150x150 template to the wide310x150 template.
        wide310x150Content.square150x150Content = square150x150Content;

        // add the wide310x150 to the Square310x310 template.
        tileContent.wide310x150Content = wide310x150Content;

        // send the notification to the app's application tile.
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(tileContent.createNotification());

        WinJS.log && WinJS.log(tileContent.getContent(), "sample", "status");
    }

    function sendTileLocalImageNotificationWithStringManipulation() {
        // create a string with the tile template xml
        var tileXmlString = "<tile>"
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
        var tileDOM = new Windows.Data.Xml.Dom.XmlDocument();
        // load the xml string into the DOM, catching any invalid xml characters 
        tileDOM.loadXml(tileXmlString);

        // create a tile notification
        var tile = new Windows.UI.Notifications.TileNotification(tileDOM);

        // send the notification to the app's application tile
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(tile);

        WinJS.log && WinJS.log(tileDOM.getXml(), "sample", "status");
    }

    function sendTileLocalImageNotificationWithXmlManipulation() {
        // get a XML DOM version of a square310x310 template by using getTemplateContent.
        var square310x310Xml = Windows.UI.Notifications.TileUpdateManager.getTemplateContent(Windows.UI.Notifications.TileTemplateType.tileSquare310x310Image);
        square310x310Xml.getElementsByTagName("image")[0].setAttribute("src", "ms-appx:///images/purpleSquare310x310.png");
        square310x310Xml.getElementsByTagName("image")[0].setAttribute("alt", "Purple image");

        // get a XML DOM version of a wide310x150 template by using getTemplateContent.
        var wide310x150Xml = Windows.UI.Notifications.TileUpdateManager.getTemplateContent(Windows.UI.Notifications.TileTemplateType.tileWide310x150ImageAndText01);

        // You will need to look at the template documentation to know how many text fields a particular template has
        // get the text attributes for this template and fill them in.
        var tileTextAttributes = wide310x150Xml.getElementsByTagName("text");
        tileTextAttributes[0].appendChild(wide310x150Xml.createTextNode("This tile notification uses ms-appx images"));

        // get the image attributes for this template and fill them in
        var tileImageAttributes = wide310x150Xml.getElementsByTagName("image");
        tileImageAttributes[0].setAttribute("src", "ms-appx:///images/redWide310x150.png");
        tileImageAttributes[0].setAttribute("alt", "Red image");

        // get a XML DOM version of a square150x150 template by using getTemplateContent.
        var square150x150Xml = Windows.UI.Notifications.TileUpdateManager.getTemplateContent(Windows.UI.Notifications.TileTemplateType.tileSquare150x150Image);
        var squareTileImageAttributes = square150x150Xml.getElementsByTagName("image");
        squareTileImageAttributes[0].setAttribute("src", "ms-appx:///images/graySquare150x150.png");
        squareTileImageAttributes[0].setAttribute("alt", "Gray image");

        // include the square150x150 template into the square310x310 notification.
        var node = square310x310Xml.importNode(square150x150Xml.getElementsByTagName("binding").item(0), true);
        square310x310Xml.getElementsByTagName("visual").item(0).appendChild(node);

        // include the wide310x150 template into the square310x310 notification.        
        node = square310x310Xml.importNode(wide310x150Xml.getElementsByTagName("binding").item(0), true);
        square310x310Xml.getElementsByTagName("visual").item(0).appendChild(node);

        // create the notification from the XML.
        var tileNotification = new Windows.UI.Notifications.TileNotification(square310x310Xml);

        // send the notification to the app's application tile.
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(tileNotification);

        WinJS.log && WinJS.log(square310x310Xml.getXml(), "sample", "status");
    }
})();