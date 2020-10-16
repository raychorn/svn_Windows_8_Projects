//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    var page = WinJS.UI.Pages.define("/html/scenario4.html", {
        ready: function (element, options) {
            document.getElementById("getFiles").addEventListener("click", getFiles, false);
        }
    });

    // Get the Pictures library and enumerate all its files using the prefetch APIs
    function getFiles() {
        // Reset output.
        clearOutput();

        // Query the Pictures library.
        var query = Windows.Storage.KnownFolders.picturesLibrary.createFileQuery();
        query.getFilesAsync().done(function (files) {
            // Display file list with storage provider and availability.
            files.forEach(function (file) {
                // Create an entry in the list for the item.
                var list = document.getElementById("itemsList");
                var listItemElement = document.createElement("li");
                listItemElement.textContent = file.name;

                // Show the item's provider (This PC, SkyDrive, Network, etc).
                listItemElement.textContent += ": On " + file.provider.displayName;

                // Show if the item is available (SkyDrive items are usually available when
                // online or when they are marked for "always available offline").
                listItemElement.textContent += " (";
                if (file.isAvailable) {
                    listItemElement.textContent += "available";
                } else {
                    listItemElement.textContent += "not available";
                }
                listItemElement.textContent += ")";

                list.appendChild(listItemElement);
            });
        });
    }

    function clearOutput() {
        document.getElementById("itemsList").innerHTML = "";
    }
})();
