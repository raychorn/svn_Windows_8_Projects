﻿//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/updateBadge.html", {
        ready: function (element, options) {
            runAnimation.addEventListener("click", updateBadge, false);
            badge = element.querySelector("#badge");
        }
    });

    var badge;
    var animating = WinJS.Promise.wrap();

    function updateBadge() {
        // If element is still animating in from previous update, wait until current animation is complete before starting the next update animation.
        animating = animating.then( function () {
            badge.innerHTML = Math.floor(Math.random() * 10);
            return WinJS.UI.Animation.updateBadge(badge);
        });
    }
})();
