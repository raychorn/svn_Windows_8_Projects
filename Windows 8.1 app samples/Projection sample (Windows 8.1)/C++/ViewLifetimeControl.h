#pragma once

#include <mutex>

namespace SDKSample
{
    namespace Projection
    {
        public delegate void ViewReleasedHandler(Platform::Object^ sender, Platform::Object^ e);

        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class ViewLifetimeControl sealed
        {
        public:
            static ViewLifetimeControl^ CreateForCurrentView();
            property Windows::UI::Core::CoreDispatcher^ Dispatcher
            {
                Windows::UI::Core::CoreDispatcher^ get();
            };

            property int Id
            {
                int get();
            };
            
            int StartViewInUse();
            int StopViewInUse();

            event ViewReleasedHandler^ Released
            {
                Windows::Foundation::EventRegistrationToken add(ViewReleasedHandler^ handler);
                void remove(Windows::Foundation::EventRegistrationToken token);
            }

        private:
            ViewLifetimeControl(Windows::UI::Core::CoreWindow^ newWindow);
            void RegisterForEvents();
            void UnregisterForEvents();
            void FinalizeRelease();
            void ViewConsolidated(Windows::UI::ViewManagement::ApplicationView^ sender, Windows::UI::ViewManagement::ApplicationViewConsolidatedEventArgs^ e);
            void VisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ e);
            event ViewReleasedHandler^ InternalReleased;
            std::mutex globalMutex;

            property bool Consolidated
            {
                bool get();
                void set (bool value);
            }


            Windows::Foundation::EventRegistrationToken consolidatedToken, visibilityToken;
            Platform::Agile<Windows::UI::Core::CoreWindow^> window;
            Windows::UI::Core::CoreDispatcher^ dispatcher;
            int refCount;
            int viewId;
            bool consolidated, released;            
        };

        // This is a simple container for a set of data used to initialize the projection view
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class ProjectionViewPageInitializationData sealed
        {
        internal:
            Windows::UI::Core::CoreDispatcher^ MainDispatcher;
            ViewLifetimeControl^ ProjectionViewPageControl;
            int MainViewId;
        };
    }
}
