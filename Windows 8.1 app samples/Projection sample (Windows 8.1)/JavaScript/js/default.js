//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    var sampleTitle = "Projection Views";

    var scenarios = [
        { url: "/html/scenario1.html", title: "Projection Session" }
    ];

    function activated(eventObject) {
        // The activated event is raised when the app is activated by the system. It tells the app 
        // whether it was activated because the user launched it or it was launched by some other means. 
        // Use the activated event handler to check the type of activation and respond appropriately to it,
        // and to load any state needed for the activation. App calling Projection API can support launch,
        // protocol, search and file contracts.

        var url = null;

        if (eventObject.detail.kind === Windows.ApplicationModel.Activation.ActivationKind.launch) {
            // Navigate to either the first scenario or to the last running scenario
            // before suspension or termination.
            url = WinJS.Application.sessionState.lastUrl || scenarios[0].url;
        } //else if (eventObject.kind === Windows.ApplicationModel.Activation.ActivationKind.protocol) {
        //   // noop       
        //} else if (eventObject.kind === Windows.ApplicationModel.Activation.ActivationKind.search) {
        //    // noop
        //} else if (eventObject.kind === Windows.ApplicationModel.Activation.ActivationKind.file) {
        //    // noop
        //}

        if (url) {
            // Use setPromise to indicate to the system that the splash screen must not be torn down
            // until after processAll and navigate complete asynchronously.
            WinJS.UI.processAll().then(function () {
                WinJS.Navigation.navigate(url, eventObject);
            });
        }
    }

    WinJS.Navigation.addEventListener("navigated", function (eventObject) {
        var url = eventObject.detail.location;
        var host = document.getElementById("contentHost");
        // Call unload method on current scenario, if there is one.
        host.winControl && host.winControl.unload && host.winControl.unload();
        WinJS.Utilities.empty(host);
        eventObject.detail.setPromise(WinJS.UI.Pages.render(url, host, eventObject.detail.state).then(function () {
            WinJS.Application.sessionState.lastUrl = url;
        }));
    });

    WinJS.Namespace.define("SdkSample", {
        sampleTitle: sampleTitle,
        scenarios: scenarios
    });

    WinJS.Namespace.define("ProjectionViews", {
        manager: new ProjectionViews.ViewManager()
    });

    WinJS.Application.addEventListener("activated", activated, false);
    WinJS.Application.start();
})();
