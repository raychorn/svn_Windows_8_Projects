﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using System;
using System.ComponentModel;
using Windows.UI.Core;
using Windows.UI.ViewManagement;

namespace MultipleViews
{
    public delegate void ViewDataReleasedHandler(Object sender, EventArgs e);

    // This class is a useful way to keep track of which view you have open,
    // and to make sure those views do not get closed before you're done with them
    public sealed class ViewData : INotifyPropertyChanged
    {
        CoreDispatcher dispatcher;
        CoreWindow window;
        string title;
        int refCount = 0;
        int viewId;
        bool released = false;
        bool consolidated = true; // Views are created consolidated
        event ViewDataReleasedHandler InternalReleased;

        ViewData(CoreWindow newWindow)
        {
            dispatcher = newWindow.Dispatcher;
            window = newWindow;
            viewId = ApplicationView.GetApplicationViewIdForWindow(window);

            // This class will automatically tell the view when its time to close
            // or stay alive in a few cases
            RegisterForEvents();
        }

        private void RegisterForEvents()
        {
            // Views that are consolidated are no longer accessible to the user,
            // so it's a good idea to close them.
            ApplicationView.GetForCurrentView().Consolidated += ViewConsolidated;

            // On the other hand, if a view becomes visible, the user is engaging
            // with it, so it's a good idea not to close it
            window.VisibilityChanged += VisibilityChanged;
        }

        private void UnregisterForEvents()
        {
            ApplicationView.GetForCurrentView().Consolidated -= ViewConsolidated;
            window.VisibilityChanged -= VisibilityChanged;
        }

        private void VisibilityChanged(object sender, VisibilityChangedEventArgs e)
        {

            if (e.Visible)
            {
                // If a view becomes visible, the user is engaging
                // with it, so it's a good idea not to close it
                Consolidated = false;
            }
        }

        // Views that are consolidated are no longer accessible to the user,
        // so it's a good idea to close them.
        private void ViewConsolidated(ApplicationView sender, ApplicationViewConsolidatedEventArgs e)
        {
            Consolidated = true;
        }

        // Called when a view has been "consolidated" (no longer accessible to the user) 
        // and no other view is trying to interact with it. The view effectively
        // shuts down in this message
        private void FinalizeRelease()
        {
            bool justReleased = false;
            lock (this)
            {
                if (refCount == 0)
                {
                    justReleased = true;
                    released = true;
                }
            }

            // This assumes that released will never be made false after it
            // it has been set to true
            if (justReleased)
            {
                UnregisterForEvents();
                InternalReleased(this, null);
            }
        }

        // Creates ViewData for the particular view.
        // Only do this once per view.
        public static ViewData CreateForCurrentView()
        {
            return new ViewData(CoreWindow.GetForCurrentThread());
        }

        // For purposes of this sample, the collection of views
        // is bound to a UI collection. This property is available for binding
        public string Title
        {
            get
            {
                return title;
            }
            set
            {
                if (title != value)
                {
                    title = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Title"));
                    }
                }
            }
        }

        // Necessary to communicate with the window
        public CoreDispatcher Dispatcher
        {
            get
            {
                // This property never changes, so there's no need to lock
                return dispatcher;
            }
        }

        public int Id
        {
            get
            {
                // This property never changes, so there's no need to lock
                return viewId;
            }
        }

        public bool Consolidated
        {
            get
            {
                // This property should only be accessed by the thread on which the view lives, so there's no need to lock
                return consolidated;
            }
            set
            {
                if (consolidated != value)
                {
                    consolidated = value;
                    if (consolidated)
                    {
                        // The view isn't accessible to the user, so it's OK to close it.
                        StopViewInUse();
                    }
                    else
                    {
                        // The view has become visible, so do not close it until it's consolidated
                        StartViewInUse();
                    }
                }
            }
        }

        // Signals that the view is being interacted with by another view,
        // so it shouldn't be closed even if it becomes "consolidated"
        public int StartViewInUse()
        {
            bool releasedCopy = false;
            int refCountCopy = 0;

            // This method is called from several different threads
            // (each view lives on its own thread)
            lock (this)
            {
                releasedCopy = this.released;
                if (!released)
                {
                    refCountCopy = ++refCount;
                }
            }

            if (releasedCopy)
            {
                throw new InvalidOperationException("This view is being disposed");
            }

            return refCountCopy;
        }

        // Should come after any call to StartViewInUse
        // Signals that the another view has finished interacting with the view tracked
        // by this object
        public int StopViewInUse()
        {
            int refCountCopy = 0;
            bool releasedCopy = false;

            lock (this)
            {
                releasedCopy = this.released;
                if (!released)
                {
                    refCountCopy = --refCount;
                    if (refCountCopy == 0)
                    {
                        // If no other view is interacting with this view, and
                        // the view isn't accessible to the user, it's appropriate
                        // to close it
                        //
                        // Before actually closing the view, make sure there are no
                        // other important events waiting in the queue (this low-priority item
                        // will run after other events)
                        dispatcher.RunAsync(CoreDispatcherPriority.Low, FinalizeRelease);
                    }
                }
            }

            if (releasedCopy)
            {
                throw new InvalidOperationException("This view is being disposed");
            }
            return refCountCopy;
        }

        // Signals to consumers that its time to close the view so that
        // they can clean up (including calling Window.Close() when finished)
        public event PropertyChangedEventHandler PropertyChanged;
        public event ViewDataReleasedHandler Released
        {
            add
            {
                bool releasedCopy = false;
                lock (this)
                {
                    releasedCopy = released;
                    if (!released)
                    {
                        InternalReleased += value;
                    }
                }

                if (releasedCopy)
                {
                    throw new InvalidOperationException("This view is being disposed");
                }
            }

            remove
            {
                lock (this)
                {
                    InternalReleased -= value;
                }
            }
        }
    }
}