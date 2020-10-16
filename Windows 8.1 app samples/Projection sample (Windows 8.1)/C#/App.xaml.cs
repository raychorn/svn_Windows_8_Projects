//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using System;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using SDKTemplate;

namespace SDKTemplate
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        /// <summary>
        /// Initializes the singleton Application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        /// 

        ConcurrentQueue<TaskCompletionSource<CoreDispatcher>> viewCreationTasks = new ConcurrentQueue<TaskCompletionSource<CoreDispatcher>>();
        public App()
        {
            this.InitializeComponent();
            this.Suspending += OnSuspending;
        }

        protected override void OnWindowCreated(WindowCreatedEventArgs args)
        {
            // Check if a secondary view is being created. 
            // This particular sample only lets secondary windows be created for projection,
            // but other apps might use secondary windows for a variety of purposes. 
            // Populating the contents of the window is delegated to the caller of App.CreateNewView
            var currentView = CoreApplication.GetCurrentView();
            if (!currentView.IsMain && !currentView.IsHosted)
            {
                TaskCompletionSource<CoreDispatcher> creationTask;
                if (viewCreationTasks.TryDequeue(out creationTask))
                {
                    creationTask.SetResult(args.Window.Dispatcher);
                }
                else
                {
                    throw new InvalidOperationException(
                        "No task available for cleaning up the new window. Was a window created outside of App.CreateNewView?"
                    );
                }
            }
        }

        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used when the application is launched to open a specific file, to display
        /// search results, and so forth.
        /// </summary>
        /// <param name="args">Details about the launch request and process.</param>
        protected override async void OnLaunched(LaunchActivatedEventArgs args)
        {
            Frame rootFrame = Window.Current.Content as Frame;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            
            if (rootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
                rootFrame = new Frame();
                // Associate the frame with a SuspensionManager key                                
                SuspensionManager.RegisterFrame(rootFrame, "AppFrame");

                if (args.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    // Restore the saved session state only when appropriate
                    try
                    {
                        await SuspensionManager.RestoreAsync();
                    }
                    catch (SuspensionManagerException)
                    {
                        //Something went wrong restoring state.
                        //Assume there is no state and continue
                    }
                }

                // Place the frame in the current Window
                Window.Current.Content = rootFrame;
            }
            if (rootFrame.Content == null || !String.IsNullOrEmpty(args.Arguments))
            {
                // When the navigation stack isn't restored or there are launch arguments
                // indicating an alternate launch (e.g.: via toast or secondary tile), 
                // navigate to the appropriate page, configuring the new page by passing required 
                // information as a navigation parameter
                if (!rootFrame.Navigate(typeof(MainPage), args.Arguments))
                {
                    throw new Exception("Failed to create initial page");
                }
            }
            // Ensure the current window is active
            Window.Current.Activate();
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private async void OnSuspending(object sender, SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();
            await SuspensionManager.SaveAsync();
            deferral.Complete();
        }

        // When a new view is created, the Window object associated with it
        // is delivered in OnWindowCreated. This function conveniently wraps that
        // interaction into a Task
        public Task<CoreDispatcher> CreateNewView()
        {
            var viewCreationTask = new TaskCompletionSource<CoreDispatcher>();
            viewCreationTasks.Enqueue(viewCreationTask);
            CoreApplication.CreateNewView("", "");
            return viewCreationTask.Task;
        }
    }
}
