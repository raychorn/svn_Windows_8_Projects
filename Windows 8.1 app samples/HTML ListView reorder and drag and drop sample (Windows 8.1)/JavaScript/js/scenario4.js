//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/scenario4.html", {
        ready: function (element, options) {

            myDragContent.addEventListener("dragstart", function (eventObject) {
                var dragData = { sourceElement: myDragContent.id, data: myItemTitle.innerText, imgSrc: myImg.src };
                eventObject.dataTransfer.setData("Text", JSON.stringify(dragData));
            });

            var dropTarget = element.querySelector("#listView");

            listView.addEventListener("dragover", function (eventObject) {
                eventObject.preventDefault();
            });

            listView.addEventListener("itemdragenter", function (eventObject) {
                if (eventObject.detail.dataTransfer.types.contains("Text")) {
                    WinJS.Utilities.addClass(dropTarget, "drop-ready");
                }
            });

            listView.addEventListener("itemdragleave", function (eventObject) {
                    WinJS.Utilities.removeClass(dropTarget, "drop-ready");
            });

            listView.addEventListener("drop", function (eventObject) {
                WinJS.Utilities.removeClass(dropTarget, "drop-ready");
                var dragData = JSON.parse(eventObject.dataTransfer.getData("Text"));

                if (dragData && dragData.sourceElement === myDragContent.id) {

                    var dropIndex = 0;

                    var newItemData = { title: dragData.data, text: ("id: " + dragData.sourceElement), picture: dragData.imgSrc };

                    myData.splice(dropIndex, 0, newItemData);

                }

                else {
                    //throw error that illegal content was dropped
                }
                
            });


        }
    });

})();
