//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/scenario2.html", {
        ready: function (element, options) {

            listView.addEventListener("itemdragstart", function (eventObject) {
                eventObject.detail.dataTransfer.setData("Text", JSON.stringify(eventObject.detail.dragInfo.getIndices()));
            });

            var dropTarget = element.querySelector("#myDropTarget");
            dropTarget.addEventListener("dragover", function (eventObject) {
                //allow drop
                eventObject.preventDefault();
            });

            dropTarget.addEventListener("dragenter", function (eventObject) {
                WinJS.Utilities.addClass(dropTarget, "drop-ready");
            });



            dropTarget.addEventListener("dragleave", function (eventObject) {
                WinJS.Utilities.removeClass(dropTarget, "drop-ready");
            });


            dropTarget.addEventListener("drop", function (eventObject) {
                // Get indicies -> keys of items that were trashed, and remove from datasource:
                WinJS.Utilities.removeClass(dropTarget, "drop-ready");
                var indexSelected = JSON.parse(eventObject.dataTransfer.getData("Text"));
                var listview = document.querySelector("#listView").winControl;
                var ds = listview.itemDataSource;
                
                ds.itemFromIndex(indexSelected[0]).then(function (item) {
                    WinJS.log && WinJS.log("You dropped the item at index " + item.index + ", "
                    + item.data.title, "sample", "status");
                });
            });

        }
    });

})();
