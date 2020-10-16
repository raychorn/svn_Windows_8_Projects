//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    var sampleTitle = "Multiple views";

    var scenarios = [
        { url: "/html/scenario1.html", title: "Creating and showing multiple views" },
        { url: "/html/scenario2.html", title: "Responding to activation" },
        { url: "/html/scenario3.html", title: "Using animations when switching" }
    ];

    function activated(eventObject) {
        var index = MultipleViews.manager.findViewIndexByViewId(eventObject.detail.currentlyShownApplicationViewId);
        var viewData = MultipleViews.manager.secondaryViews.getAt(index);
        if (viewData) {
            if (eventObject.detail.kind === Windows.ApplicationModel.Activation.ActivationKind.protocol) {
                viewData.appView.postMessage({
                    handleProtocolLaunch: true,
                    uri: eventObject.detail.uri.absoluteUri
                }, MultipleViews.thisDomain);
            }
        } else {
            // Use setPromise to indicate to the system that the splash screen must not be torn down
            // until after processAll and navigate complete asynchronously.
            eventObject.setPromise(WinJS.UI.processAll().then(function () {
                // Navigate to either the first scenario or to the last running scenario
                // before suspension or termination.

                // Check the data stored from last run. Remember, you need to do "Suspend and Shutdown"
                // then start the app again for this data to be saved.
                var shouldDisable = Windows.Storage.ApplicationData.current.localSettings.values[MultipleViews.disableMainViewKey];
                if (shouldDisable) {
                    Windows.UI.ViewManagement.ApplicationViewSwitcher.disableShowingMainViewOnActivation();
                }

                var url;
                if (eventObject.detail.kind === Windows.ApplicationModel.Activation.ActivationKind.protocol) {
                    url = scenarios[1].url;
                } else {
                    url = WinJS.Application.sessionState.lastUrl || scenarios[0].url;
                    
                }

                return WinJS.Navigation.navigate(url);
            }));
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

    WinJS.Namespace.define("MultipleViews", {
        manager: new MultipleViews.ViewManager(),
        disableMainViewKey: "DisableShowingMainViewOnActivation"
    });

    WinJS.Application.addEventListener("activated", activated, false);
    WinJS.Application.start();
})();
