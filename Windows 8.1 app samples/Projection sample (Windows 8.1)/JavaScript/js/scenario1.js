//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    var ViewManagement = Windows.UI.ViewManagement;

    var viewSelect;

    var defaultTitle = "Presentation View";

    var page = WinJS.UI.Pages.define("/html/scenario1.html", {
        ready: function (element, options) {
            document.getElementById("createViewButton").addEventListener("click", createView, false);
            document.getElementById("startProjectionButton").addEventListener("click", startProjection, false);

            viewSelect = document.querySelector(".view-select").winControl;

            if (ProjectionViews.manager) {
                viewSelect.itemDataSource = ProjectionViews.manager.secondaryViews.dataSource;
            }
        }
    });

    function createView() {
        // Set up the secondary view, but don't show it yet.
        // The actual creation of the view is done in viewLifetimeControl.js.
        ProjectionViews.manager.createNewView("ms-appx:///html/secondaryView.html", { title: defaultTitle });
    }

    function startProjection() {
        viewSelect.selection.getItems().then(function (items) {
            if (items.length > 0) {
                var view = items[0].data;

                // Start projection using the previously created secondary view.
                return ViewManagement.ProjectionManager.startProjectingAsync(
                    view.viewId,
                    ViewManagement.ApplicationView.getForCurrentView().id
                );
            }
            return WinJS.Promise.wrap(false);
        }).done();
    }
})();
