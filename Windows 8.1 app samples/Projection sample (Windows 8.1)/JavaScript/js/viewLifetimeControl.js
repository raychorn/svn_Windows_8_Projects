//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    var ViewManagement = Windows.UI.ViewManagement;
    var messageTypes = {
        none: 0,
        queryProxyReadyForRelease: 1,
        proxyReadyForRelease: 2,
        proxyReleased: 3,
        titleUpdated: 4,
        initialize: 5
    };

    var emptyTitle = "<title cleared>";

    // These classes are a useful way to keep track of which view you have open,
    // and to make sure those views do not get closed before you're done with them.
    WinJS.Namespace.define("ProjectionViews", {

        // Store the app's default domain, which will be different in your app
        // Make sure to update this.
        thisDomain: document.location.protocol + "//" + document.location.host,

        // ViewManager defines a useful way to create and query secondary views.
        ViewManager: WinJS.Class.define(function () {
            var that = this;
            this._viewReleasedWrapper = function (e) { that._viewReleased(e); };
            window.addEventListener("message", function (e) {
                if (e.origin === ProjectionViews.thisDomain && e.data.type) {
                    that._handleMessage(e); 
                }
            }, false);

        },
        {
            _handleMessage: function (e) {
                var i = this.findViewIndexByViewId(e.data.viewId);
                this.secondaryViews.getItem(i).data._handleMessage(e);
            },
            findViewIndexByViewId: function (viewId) {
                for (var i = 0, len = this.secondaryViews.length; i < len; i++) {
                    var value = this.secondaryViews.getItem(i).data;
                    if (viewId === value.viewId) {
                        return i;
                    }
                }
                return null;
            },
            _viewReleased: function (e) {
                e.target.removeEventListener("released", this._viewReleasedWrapper, false);
                var i = this.findViewIndexByViewId(e.target.viewId);
                this.secondaryViews.splice(i, 1);
            },
            secondaryViews: new WinJS.Binding.List([]),
            createNewView: function (page, initData) {
                if (!page) {
                    throw new WinJS.ErrorFromName("WinJS.UI.ListDataSource.InvalidIndexReturned", strings.invalidIndexReturned);
                }

                var newView = MSApp.createNewView(page);
                newView.postMessage({
                    type: messageTypes.initialize,
                    initData: initData = initData || {} 
                }, ProjectionViews.thisDomain);
                var newProxy = new ProjectionViews.ViewLifetimeControlProxy(newView);

                newProxy.addEventListener("released", this._viewReleasedWrapper, false);
                this.secondaryViews.push(newProxy);
                return newProxy;
            }
        }),

        // ViewLifetimeControlProxy controls the life time of second views being created.
        ViewLifetimeControlProxy: WinJS.Class.mix(WinJS.Class.define(function (appView) {
            this.appView = appView;
            this.viewId = appView.viewId;
            this.title = "";
        },
        {
            _alertView: function (type, data) {
                if (!type) {
                    throw new WinJS.ErrorFromName("WinJS.UI.ListDataSource.InvalidIndexReturned", strings.invalidIndexReturned);
                }
                data = data || {};
                data.type = type;

                this.appView.postMessage(data, ProjectionViews.thisDomain);
            },
            _refCount: 0,
            appView: null,
            _handleMessage: function (e) {
                var data = e.data;
                switch (data.type) {
                    case messageTypes.queryProxyReadyForRelease:

                        // The secondary view may be ready to be closed (e.g., it has been consolidated).
                        // Make sure the main view is done with it, and, if so, get rid of this wrapper.
                        if (this._refCount === 0) {
                            this.dispatchEvent("released");
                            this._alertView(messageTypes.proxyReleased);
                        }
                        break;
                    case messageTypes.titleUpdated:

                        // For purposes of this sample, the collection of views
                        // is bound to a UI collection. This property is available for binding.
                        var oldValue = this.title; 
                        this.title = data.title;
                        this.notify("title", this.title, oldValue);
                        break;
                    default:
                        throw new WinJS.ErrorFromName("WinJS.UI.ListDataSource.InvalidIndexReturned", strings.invalidIndexReturned);
                }
            },

            // Signals that the secondary view is being interacted with by this view,
            // so it shouldn't be closed even if it becomes "consolidated".
            startViewInUse: function () {
                this._refCount++;
            },

            // Signals that the secondary view is no longer being interacted with by this view.
            stopViewInUse: function () {
                this._refCount--;

                if (this.refCount === 0) {
                    this._alertView(messageTypes.proxyReadyForRelease);
                }
            }
        }), WinJS.Utilities.eventMixin, WinJS.Binding.observableMixin),

        //ViewLifetimeControl is a mix class for tracking secondary views. For more example, please see the http://code.msdn.microsoft.com/windowsapps and search for Multiple Views sample.
        ViewLifetimeControl: WinJS.Class.mix(WinJS.Class.define(function () {
            var that = this;
            this.opener = MSApp.getViewOpener();
            this._handleMessageWrapper = function (e) { 
                if (e.origin === ProjectionViews.thisDomain && e.data.type) {
                    that._handleMessage(e); 
                }
            };
            this._onConsolidatedWrapper = function (e) { that._onConsolidated(e); };
            this._onVisibilityChangeWrapper = function (e) { that._onVisibilityChange(e); };
            this._finalizeReleaseWrapper = function (e) { that._finalizeRelease(e); };
            this.viewId = ViewManagement.ApplicationView.getForCurrentView().id;
        }, 
        {
            _refCount: 0,
            _proxyReleased: false,
            _consolidated: true, // Views are created consolidated
            _alertProxy: function (type, data) {
                if (!type) {
                    throw new WinJS.ErrorFromName("WinJS.UI.ListDataSource.InvalidIndexReturned", strings.invalidIndexReturned);
                }
                data = data || {};
                data.type = type;
                data.viewId = this.viewId;

                this.opener.postMessage(data, ProjectionViews.thisDomain);
            },
            _handleMessage: function (e) {
                var data = e.data;
                switch (data.type) {
                    // The main view has released its reference to this view.
                    case messageTypes.proxyReleased:
                        this._proxyReleased = true;
                        setImmediate(this._finalizeReleaseWrapper);
                        break;

                    // The main view has finished operations with this view.
                    // Double check if something has changed.
                    case messageTypes.proxyReadyForRelease:
                        this._alertProxy(messageTypes.queryProxyReadyForRelease);
                        break;

                    // Data sent to get this view working (depends on the context where the
                    // view is launched.)
                    case messageTypes.initialize:
                        this.dispatchEvent("initializedatareceived", e.data.initData);
                        break;
                    default:
                        throw new WinJS.ErrorFromName("WinJS.UI.ListDataSource.InvalidIndexReturned", strings.invalidIndexReturned);
                }
            },
            _onConsolidated: function () {

                // Views that are consolidated are no longer accessible to the user,
                // so it's a good idea to close them.
                this._setConsolidated(true);
            },
            _onVisibilityChange: function () {

                // If a view becomes visible, the user is engaging
                // with it, so it's a good idea not to close it
                if (!document.hidden) {
                    this._setConsolidated(false);
                }
            },
            _setConsolidated: function(value) {
                if (this._consolidated !== value) {
                    this._consolidated = value;
                    if (value) {
                        this.stopViewInUse();
                    } else {
                        this.startViewInUse();
                    }
                }
            },

            // Called when a view has been "consolidated" (no longer accessible to the user) 
            // and no other view is trying to interact with it. At the end of this, the view
            // is closed.
            _finalizeRelease: function () {
                if (this._refCount === 0 && this._proxyReleased) {
                    window.removeEventListener("message", this._handleMessageWrapper, false);
                    ViewManagement.ApplicationView.getForCurrentView().removeEventListener("consolidated", this._onConsolidatedWrapper, false);
                    document.removeEventListener("visibilitychange", this._onVisibilityChangeWrapper, false);
                    this.dispatchEvent("released");

                    window.close();
                }
            },
            setTitle: function (value) {
                ViewManagement.ApplicationView.getForCurrentView().title = value;

                // Setting the title on ApplicationView to blank will clear the title in
                // the system switcher. It would be good to still have a title in the app's UI.
                if (!value) {
                    value = emptyTitle; 
                }
                this._alertProxy(messageTypes.titleUpdated, { title: value });
            },

            // Add all event listeners. This allows you to attach events to this object
            // before it begins processing messages.
            initialize: function () {
                window.addEventListener("message", this._handleMessageWrapper, false);

                // This class will automatically tell the view when its time to close
                // or stay alive in a few cases.
                //
                // Views that are consolidated are no longer accessible to the user,
                // so it's a good idea to close them.
                ViewManagement.ApplicationView.getForCurrentView().addEventListener("consolidated", this._onConsolidatedWrapper, false);

                // On the other hand, if a view becomes visible, the user is engaging
                // with it, so it's a good idea not to close it
                document.addEventListener("visibilitychange", this._onVisibilityChangeWrapper, false);
            },

            // Signals that there are outstanding async operations, so don't close this view.
            startViewInUse: function () {
                this._refCount++;
            },

            // Should come after any call to StartViewInUse.
            // Signals that this view has finished async operations in it.
            stopViewInUse: function () {
                this._refCount--;

                if (this._refCount === 0) {
                    if (this._proxyReleased) {

                        // If no other view is interacting with this view, and
                        // the view isn't accessible to the user, it's appropriate
                        // to close it.
                        //
                        // Before actually closing the view, make sure there are no
                        // other important events waiting in the queue (this low-priority item
                        // will run after other events.)
                        setImmediate(this._finalizeReleaseWrapper);
                    } else {

                        // Check if the main view is not interacting with this view.
                        this._alertProxy(messageTypes.queryProxyReadyForRelease);
                    }
                }
            }
        }), WinJS.Utilities.eventMixin)
    });
})();