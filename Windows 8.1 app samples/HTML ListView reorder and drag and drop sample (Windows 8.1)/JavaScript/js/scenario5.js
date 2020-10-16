//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    WinJS.Namespace.define("Scenario5", { 

        currDropIndex: -1,

        mydragoverhandler: function (eventObject) {
            eventObject.preventDefault();

            var targetIndex = listView.winControl.indexOfElement(eventObject.srcElement);
            var dropTarget = listView.winControl.elementFromIndex(targetIndex);

            if (!WinJS.Utilities.hasClass(dropTarget, "drop-ready")) {
                WinJS.Utilities.addClass(dropTarget, "drop-ready");
            }
        },

        mydrophandler: function (eventObject) {
            var dragData = JSON.parse(eventObject.dataTransfer.getData("Text"));

            if (dragData && dragData.sourceElement === myDragContent.id) {

                var newItemData = { title: dragData.data, text: ("id: " + dragData.sourceElement), picture: dragData.imgSrc };

                var targetItemIndex = listView.winControl.indexOfElement(eventObject.srcElement);
                WinJS.log && WinJS.log("You dropped \"" + newItemData.title + "\" on the item at index " + targetItemIndex, "sample", "status");

                var dropTarget = listView.winControl.elementFromIndex(targetItemIndex);
                if (WinJS.Utilities.hasClass(dropTarget, "drop-ready")) {
                    WinJS.Utilities.removeClass(dropTarget, "drop-ready");
                }
            }
            else {
                //throw error that illegal content was dropped
            }
        },

        mydragleavehandler: function (eventObject) {
            var targetIndex = listView.winControl.indexOfElement(eventObject.srcElement);
            var dropTarget = listView.winControl.elementFromIndex(targetIndex);

            WinJS.Utilities.removeClass(dropTarget, "drop-ready");
        }
    });

    WinJS.Utilities.markSupportedForProcessing(Scenario5.mydragoverhandler);
    WinJS.Utilities.markSupportedForProcessing(Scenario5.mydrophandler);
    WinJS.Utilities.markSupportedForProcessing(Scenario5.mydragleavehandler);

    var page = WinJS.UI.Pages.define("/html/scenario5.html", {

        ready: function (element, options) {
            Scenario5.myProperty = 5;
            myDragContent.addEventListener("dragstart", function (eventObject) {
                var dragData = { sourceElement: myDragContent.id, data: myItemTitle.innerText, imgSrc: myImg.src };
                eventObject.dataTransfer.setData("Text", JSON.stringify(dragData));
            });
        }
    });

})();
