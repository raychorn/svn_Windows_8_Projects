//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/sendTextTile.html", {
        ready: function (element, options) {
            document.getElementById("sendTileTextNotification").addEventListener("click", sendTileTextNotification, false);
            document.getElementById("sendTileTextNotificationWithStringManipulation").addEventListener("click", sendTileTextNotificationWithStringManipulation, false);
            document.getElementById("sendTileTextNotificationWithXmlManipulation").addEventListener("click", sendTileTextNotificationWithXmlManipulation, false);
            document.getElementById("clearTileNotification").addEventListener("click", clearTileNotification, false);
        }
    });

    function clearTileNotification() {
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().clear();
        WinJS.log && WinJS.log("Tile cleared", "sample", "status");
    }

    function sendTileTextNotification() {
        // Note: This sample contains an additional project, NotificationsExtensions.
        // NotificationsExtensions exposes an object model for creating notifications, but you can also modify the xml
        // of the notification directly. See the additional function sendTileTextNotificationWithXmlManipulation to see how
        // to do it by modifying Xml directly, or sendTileTextNotificationWithStringManipulation to see how to do it
        // by modifying strings directly

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
        var tileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare310x310Text05();
        tileContent.textHeading.text = "Hello World! My very own tile notification";

        // create the notification for a wide310x150 template.
        var wide310x150Content = NotificationsExtensions.TileContent.TileContentFactory.createTileWide310x150Text03();
        wide310x150Content.textHeadingWrap.text = "Hello World! My very own tile notification";

        // create the square150x150 template and attach it to the wide310x150 template.
        var square150x150Content = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare150x150Text04();
        square150x150Content.textBodyWrap.text = "Hello World! My very own tile notification";
        wide310x150Content.square150x150Content = square150x150Content;

        // attach the wide310x150 template to the square310x310 template.
        tileContent.wide310x150Content = wide310x150Content;

        // send the notification
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(tileContent.createNotification());

        WinJS.log && WinJS.log(tileContent.getContent(), "sample", "status");
    }

    function sendTileTextNotificationWithStringManipulation() {
        // create a string with the tile template xml
        var tileXmlString = "<tile>"
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
        var tileDOM = new Windows.Data.Xml.Dom.XmlDocument();
        // load the xml string into the DOM, catching any invalid xml characters 
        tileDOM.loadXml(tileXmlString);

        // create a tile notification
        var tile = new Windows.UI.Notifications.TileNotification(tileDOM);

        // send the notification to the app's application tile
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(tile);

        WinJS.log && WinJS.log(tileDOM.getXml(), "sample", "status");
    }

    function sendTileTextNotificationWithXmlManipulation() {
        // get a XML DOM version of a square310x310 template by using getTemplateContent.
        var square310x310Xml = Windows.UI.Notifications.TileUpdateManager.getTemplateContent(Windows.UI.Notifications.TileTemplateType.tileSquare310x310Text05);
        square310x310Xml.getElementsByTagName("text")[0].setAttribute("id", "1");
        square310x310Xml.getElementsByTagName("text")[0].appendChild(square310x310Xml.createTextNode("Hello World! My very own tile notification"));

        // get a XML DOM version of a wide310x150 template by using getTemplateContent.
        var wide310x150Xml = Windows.UI.Notifications.TileUpdateManager.getTemplateContent(Windows.UI.Notifications.TileTemplateType.tileWide310x150Text03);

        // You will need to look at the template documentation to know how many text fields a particular template has
        // get the text attributes for this template and fill them in.
        var tileTextAttributes = wide310x150Xml.getElementsByTagName("text");
        tileTextAttributes[0].appendChild(wide310x150Xml.createTextNode("Hello World! My very own tile notification"));

        // get a XML DOM version of a square150x150 template by using getTemplateContent.
        var square150x150Xml = Windows.UI.Notifications.TileUpdateManager.getTemplateContent(Windows.UI.Notifications.TileTemplateType.tileSquare150x150Text04);
        var squareTileTextAttributes = square150x150Xml.getElementsByTagName("text");
        squareTileTextAttributes[0].appendChild(square150x150Xml.createTextNode("Hello World! My very own tile notification"));

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