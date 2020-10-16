//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/scenario3.html", {
        ready: function (element, options) {

           
            myDragContent.addEventListener("dragstart", function (eventObject) {
                var dragData = { sourceElement: myDragContent.id, data: myItemTitle.innerText, imgSrc: myImg.src };
                eventObject.dataTransfer.setData("Text", JSON.stringify(dragData));
            });


            listView.addEventListener("itemdragenter", function (eventObject) {
                if (eventObject.detail.dataTransfer.types.contains("Text")) {
                    eventObject.preventDefault();
                }


            });

            listView.addEventListener("itemdragdrop", function (eventObject) {
                var dragData = JSON.parse(eventObject.detail.dataTransfer.getData("Text"));

                if (dragData && dragData.sourceElement === myDragContent.id) {

                    var dropIndex = eventObject.detail.insertAfterIndex;

                    var newItemData = { title: dragData.data, text: ("id: " + dragData.sourceElement), picture: dragData.imgSrc };
                    // insertAfterIndex tells us the insert we need to add the new item to. If we're inserting at the start, insertAfterIndex
                    // is -1. Adding 1 to insertAfterIndex gives us the location in the array that our new item should appear at

                    dropIndex = Math.min(myData.length, dropIndex + 1); //there is a bug that addresses the large values of insertAfterIndex
                    myData.splice(dropIndex, 0, newItemData);

                }

                else {
                    //throw error that illegal content was dropped
                }

            });


        }
    });

})();
