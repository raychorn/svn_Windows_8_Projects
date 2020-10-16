//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/PinTileAlternateVisualElementsAsync.html", {
        ready: function (element, options) {
            document.getElementById("pinButton").addEventListener("click", pinSecondaryTile, false);
        }
    });

    function pinSecondaryTile(sender) {
        var Scenario1TileId = "SecondaryTile.Logo";

        // We're pinning a secondary tile, so first we need to build up the tile.Pin
        // Start by creating the Uri assests.
        var square70x70Logo = new Windows.Foundation.Uri("ms-appx:///Images/square70x70Tile-sdk.png");
        var square150x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/square150x150Tile-sdk.png");
        var wide310x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/wide310x150Tile-sdk.png");
        var square310x310Logo = new Windows.Foundation.Uri("ms-appx:///Images/square310x310Tile-sdk.png");
        var square30x30Logo = new Windows.Foundation.Uri("ms-appx:///Images/square30x30Tile-sdk.png");

        // If a user launches the sample through the pinned secondary tile, the app will want to respond appropriately.
        // In this case, we'll use activation arguments to tell that the app was launched from that secondary tile.
        // Responding appropriately, for the purpose of this sample, will mean displaying the activation arguments passed.
        // We'll set the time the tile was pinned as the activation argument.

        // Get the current time...
        var currentTime = new Date();

        // Now, to make this explicit and highly readable, assign current time plus some name to a variable called tileActivationArguments
        var newTileActivationArguments = Scenario1TileId  + " WasPinnedAt=" + currentTime;

        // And we're ready to construct our tile!
        // Pass in all the details we specified above into the constructor...
        var tile = new Windows.UI.StartScreen.SecondaryTile(Scenario1TileId,
            "Title text shown on the tile",
            newTileActivationArguments,
            square150x150Logo,
            Windows.UI.StartScreen.TileSize.square150x150);

        // To have the larger tile sizes available the assets must be provided.
        tile.visualElements.wide310x150Logo = wide310x150Logo;
        tile.visualElements.square310x310Logo = square310x310Logo;

        // The 70x70 asset does not have to be supplied as it will be created by downsizing the 150x150.
        // However it can be provided as shown.
        tile.visualElements.square70x70Logo = square70x70Logo;

        // Like the background color, the small tile logo is inherited from the parent application tile by default. 
        // Let's override it, just to see how that's done.
        tile.visualElements.square30x30Logo = square30x30Logo;

        // The display of the app name can be controlled for each tile size.
        // The default is false.
        tile.visualElements.showNameOnSquare150x150Logo = false;
        tile.visualElements.showNameOnWide310x150Logo = true;
        tile.visualElements.showNameOnSquare310x310Logo = true;

        // Add the handler for the VisualElemets request.
        tile.onvisualelementsrequested = visualElementsRequestedAsyncHandler;

        // Specify a foreground text value.
        // The tile background color is inherited from the parent unless a separate value is specified.
        tile.visualElements.foregroundText = Windows.UI.StartScreen.ForegroundText.dark;

        // Set this to false if roaming doesn't make sense for the secondary tile.
        // The default is true;
        tile.roamingEnabled = false;

        // Now, we're almost ready to try and pin the tile.
        // when we make the pin request, Windows will show a flyout asking for user confirmation.
        // To make this predictable and meet user expectations, we want to consistently place the flyout near the button where the flyout was invoked.
        // Grab the bounding rectangle of the element that was passed in...
        var selectionRect = document.getElementById("pinButton").getBoundingClientRect();

        // Using the bounding rectangle of the element that was used to invoke the flyout, we will be able to derive coordinates to show the flyout over.
        // We now are ready to try and pin the tile.
        tile.requestCreateForSelectionAsync({ x: selectionRect.left, y: selectionRect.top, width: selectionRect.width, height: selectionRect.height }, Windows.UI.Popups.Placement.below).done(function (isCreated) {
            if (isCreated) {
                // The tile was successfully created, so we'll display a status message
                WinJS.log && WinJS.log("Secondary tile was successfully pinned.", "sample", "status");
            } else {
                // something happened - either the user cancelled, or an error was encountered.
                // Either way, the flyout is now gone, and we'll react the same way.
                WinJS.log && WinJS.log("Secondary tile was not pinned.", "sample", "error");
            }
        });
    }

    function visualElementsRequestedAsyncHandler(args)
    {
        // Request the deferral object if additional time is needed to acquire or process the assets.
        // The Pin to Start Flyout will display a progress control during this time.
        var deferral = args.request.getDeferral();

        // This delay is to simulate doing an async operation or any processing that will 
        // take a noticeable amount of time to complete.
        WinJS.Promise.timeout(3000).then(function () {
            assignVisualElements(args);

            // Once the assets have been assigned, this signals the Pin to Start Flyout to show the tiles.
            deferral.complete();
        });
    }

    function assignVisualElements(args)
    {
        var alternate1Square70x70Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate1Square70x70Tile-sdk.png");
        var alternate1Square150x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate1Square150x150Tile-sdk.png");
        var alternate1Wide310x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate1Wide310x150Tile-sdk.png");
        var alternate1Square310x310Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate1Square310x310Tile-sdk.png");

        var alternate2Square70x70Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate2Square70x70Tile-sdk.png");
        var alternate2Square150x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate2Square150x150Tile-sdk.png");
        var alternate2Wide310x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate2Wide310x150Tile-sdk.png");
        var alternate2Square310x310Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate2Square310x310Tile-sdk.png");

        var alternate3Square70x70Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate3Square70x70Tile-sdk.png");
        var alternate3Square150x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate3Square150x150Tile-sdk.png");
        var alternate3Wide310x150Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate3Wide310x150Tile-sdk.png");
        var alternate3Square310x310Logo = new Windows.Foundation.Uri("ms-appx:///Images/alternate3Square310x310Tile-sdk.png");

        // This check verifies that we are still within the deadline to complete the request.
        if (args.request.deadline > new Date()) {
            args.request.alternateVisualElements[0].square70x70Logo = alternate1Square70x70Logo;
            args.request.alternateVisualElements[0].square150x150Logo = alternate1Square150x150Logo;
            args.request.alternateVisualElements[0].wide310x150Logo = alternate1Wide310x150Logo;
            args.request.alternateVisualElements[0].square310x310Logo = alternate1Square310x310Logo;

            args.request.alternateVisualElements[0].backgroundColor = Windows.UI.Colors.green;
            args.request.alternateVisualElements[0].foregroundText = Windows.UI.StartScreen.ForegroundText.dark;
            args.request.alternateVisualElements[0].showNameOnSquare310x310Logo = true;
        }

        // If there is still time left, continue assigning assets.
        if (args.request.deadline > new Date()) {
            args.request.alternateVisualElements[1].square70x70Logo = alternate2Square70x70Logo;
            args.request.alternateVisualElements[1].square150x150Logo = alternate2Square150x150Logo;
            args.request.alternateVisualElements[1].wide310x150Logo = alternate2Wide310x150Logo;
            args.request.alternateVisualElements[1].square310x310Logo = alternate2Square310x310Logo;

            args.request.alternateVisualElements[1].backgroundColor = Windows.UI.Colors.darkViolet;
            args.request.alternateVisualElements[1].foregroundText = Windows.UI.StartScreen.ForegroundText.dark;
            args.request.alternateVisualElements[1].showNameOnWide310x150Logo = true;
        }

        if (args.request.deadline > new Date()) {
            args.request.alternateVisualElements[2].square70x70Logo = alternate3Square70x70Logo;
            args.request.alternateVisualElements[2].square150x150Logo = alternate3Square150x150Logo;
            args.request.alternateVisualElements[2].wide310x150Logo = alternate3Wide310x150Logo;
            args.request.alternateVisualElements[2].square310x310Logo = alternate3Square310x310Logo;

            args.request.alternateVisualElements[2].backgroundColor = Windows.UI.Colors.red;
            args.request.alternateVisualElements[2].foregroundText = Windows.UI.StartScreen.ForegroundText.light;
            args.request.alternateVisualElements[2].showNameOnSquare150x150Logo = true;
        }
    }
})();
