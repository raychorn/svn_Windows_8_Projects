//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/notificationExpiration.html", {
        ready: function (element, options) {
            document.getElementById("useTileNotificationExpiration").addEventListener("click", useTileNotificationExpiration, false);
        }
    });

    function useTileNotificationExpiration() {
        var currentTime = new Date();
        var seconds = document.getElementById("seconds").value;
        var numericExpression = /^[0-9]+$/;
        if (!(seconds.length > 0 && seconds.length <= 3 && seconds.match(numericExpression))) {
            seconds = 10; 
        }

        var wide310x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileWide310x150Text04();
        wide310x150TileContent.textBodyWrap.text = "This notification will expire at " + new Date(currentTime.getTime() + seconds * 1000);
        wide310x150TileContent.branding = NotificationsExtensions.TileContent.TileBranding.none;

        var square150x150TileContent = NotificationsExtensions.TileContent.TileContentFactory.createTileSquare150x150Text04();
        square150x150TileContent.textBodyWrap.text = "This notification will expire at " + new Date(currentTime.getTime() + seconds * 1000);
        square150x150TileContent.branding = NotificationsExtensions.TileContent.TileBranding.none;
        wide310x150TileContent.square150x150Content = square150x150TileContent;

        var tileNotification = wide310x150TileContent.createNotification();

        var expiryTime = new Date(currentTime.getTime() + seconds * 1000);

        // set the expiration time on the notification
        tileNotification.expirationTime = expiryTime;
        Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication().update(tileNotification);

        WinJS.log && WinJS.log("Tile notification sent. It will expire at " + expiryTime, "sample", "status");
    }
})();