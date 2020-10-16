//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/globalization.html", {
        ready: function (element, options) {
            document.getElementById("viewCurrentResources").addEventListener("click", viewCurrentResources, false);
            document.getElementById("sendScaledImageTileNotification").addEventListener("click", sendScaledImageTileNotification, false);
            document.getElementById("sendTextResourceTileNotification").addEventListener("click", sendTextResourceTileNotification, false);
            document.getElementById("sendTileNotificationWithQueryStrings").addEventListener("click", sendTileNotificationWithQueryStrings, false);            
        }
    });

    function viewCurrentResources() {
        var context = Windows.ApplicationModel.Resources.Core.ResourceContext.getForCurrentView();
        var qualifierValues = context.qualifierValues;
        var scale = qualifierValues["scale"];
        var contrast = qualifierValues["contrast"];
        var ASLS = context.languages[0]; // the application specific language is always first in the list
        WinJS.log && WinJS.log("You system is currently set to the following values: Application Language: " + ASLS + ", Scale: " + scale + ", Contrast: " + contrast + ". If using web images and addImageQuery, the following query string would be appened to the URL: ?ms-lang=" + ASLS + "&ms-scale=" + scale + "&ms-contrast=" + contrast, "sample", "status");
    }

    function sendTileNotificationWithQueryStrings() {
        var wide310x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileWide310x150ImageAndText01();
        wide310x150TileContent.textCaptionWrap.text = "This tile notification uses web images";
        wide310x150TileContent.image.src = document.getElementById("imageSrcInput").value;
        wide310x150TileContent.addImageQuery = true;        

        var square150x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare150x150Image();
        square150x150TileContent.image.src = document.getElementById("imageSrcInput").value;
        wide310x150TileContent.square150x150Content = square150x150TileContent;

        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(wide310x150TileContent.createNotification());

        WinJS.log && WinJS.log(wide310x150TileContent.getContent(), "sample", "status");
    }

    function sendScaledImageTileNotification() {
        var context = Windows.ApplicationModel.Resources.Core.ResourceContext.getForCurrentView();
        var qualifierValues = context.qualifierValues;
        var scale = qualifierValues["scale"];
        
        var wide310x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileWide310x150SmallImageAndText03();
        wide310x150TileContent.textBodyWrap.text = "blueWide310x150.png in the xml is actually blueWide310x150.scale-" + scale + ".png";
        wide310x150TileContent.image.src = "ms-appx:///images/blueWide310x150.png";

        var square150x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare150x150Image(); 
        square150x150TileContent.image.src = "ms-appx:///images/graySquare150x150.png";
        wide310x150TileContent.square150x150Content = square150x150TileContent;

        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(wide310x150TileContent.createNotification());

        WinJS.log && WinJS.log(wide310x150TileContent.getContent(), "sample", "status");
    }

    function sendTextResourceTileNotification() {
        var wide310x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileWide310x150Text03();

        // check out /en-US resources.resjson to understand where this string will come from
        wide310x150TileContent.textHeadingWrap.text = "ms-resource:greeting";

        var square150x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare150x150Text04();
        square150x150TileContent.textBodyWrap.text = "ms-resource:greeting";
        wide310x150TileContent.square150x150Content = square150x150TileContent;

        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(wide310x150TileContent.createNotification());

        WinJS.log && WinJS.log(wide310x150TileContent.getContent(), "sample", "status");
    }
})();