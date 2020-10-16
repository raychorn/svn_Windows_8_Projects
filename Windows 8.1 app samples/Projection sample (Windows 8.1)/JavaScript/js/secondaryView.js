//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    var ViewManagement = Windows.UI.ViewManagement;

    // This page is shown in secondary views created by this app.
    // See Scenario 1 for details on how to create a secondary view
    var thisView = new ProjectionViews.ViewLifetimeControl();

    thisView.addEventListener("initializedatareceived", function (e) {
        if (e.detail.title) {
            thisView.setTitle(e.detail.title);
        }
    }, false);

    thisView.initialize();

    var titleBox, outputBox;
    document.addEventListener("DOMContentLoaded", function () {
        titleBox = document.getElementById("titleBox");
        outputBox = document.getElementById("queryResultBox");
        document.getElementById("setTitleButton").addEventListener("click", setTitle, false);
        document.getElementById("clearTitleButton").addEventListener("click", clearTitle, false);
        document.getElementById("swapDisplayButton").addEventListener("click", swapDisplays, false);
        document.getElementById("stopProjectionButton").addEventListener("click", stopProjection, false);
        document.getElementById("secondMonitorAvailableButton").addEventListener("click", querySecondScreen, false);
        document.getElementById("resumeProjectionButton").addEventListener("click", resumeProjection, false);
    }, false);

    function setTitle() {
        // Set a title for the window. This title is visible
        // in system switchers       
        thisView.setTitle(titleBox.value);
        titleBox.value = "";
    }

    function clearTitle() {
        // Clear the title by setting it to blank
        titleBox.value = "";
        thisView.setTitle("");
    }

    function stopProjection() {
        ViewManagement.ProjectionManager.stopProjectingAsync(
            ViewManagement.ApplicationView.getForCurrentView().id,
            thisView.opener.viewId
        ).done();
    }

    function swapDisplays() {
        if (ViewManagement.ProjectionManager.projectionDisplayAvailable) {
            ViewManagement.ProjectionManager.swapDisplaysForViewsAsync(
                ViewManagement.ApplicationView.getForCurrentView().id,
                thisView.opener.viewId
                ).done();
        }
    }

    function querySecondScreen() {
        if (ViewManagement.ProjectionManager.projectionDisplayAvailable) {
            outputBox.innerText = "";
            outputBox.innerText = "A second monitor is available";
        } else {
            outputBox.innerText = "";
            outputBox.innerText = "A second monitor is NOT available";
        }
    }

    function resumeProjection() {
        ViewManagement.ProjectionManager.startProjectingAsync(
            ViewManagement.ApplicationView.getForCurrentView().id,
            thisView.opener.viewId
        ).done();
    }
})();