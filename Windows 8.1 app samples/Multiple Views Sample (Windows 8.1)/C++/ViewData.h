#pragma once

namespace SDKSample
{
    namespace MultipleViews
    {
        public delegate void ViewDataReleasedHandler(Platform::Object^ sender, Platform::Object^ e);

        [Windows::UI::Xaml::Data::Bindable]
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class ViewData sealed : Windows::UI::Xaml::Data::INotifyPropertyChanged
        {
        public:
            static ViewData^ CreateForCurrentView();
            property Windows::UI::Core::CoreDispatcher^ Dispatcher
            {
                Windows::UI::Core::CoreDispatcher^ get();
            };

            property int Id
            {
                int get();
            };
            
            property Platform::String^ Title
            {
                Platform::String^ get();
                void set(Platform::String^ value);
            };

            int StartViewInUse();
            int StopViewInUse();

            event ViewDataReleasedHandler^ Released
            {
                Windows::Foundation::EventRegistrationToken add(ViewDataReleasedHandler^ handler);
                void remove(Windows::Foundation::EventRegistrationToken token);
            }
            virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;

        private:
            ViewData(Windows::UI::Core::CoreWindow^ newWindow);
            void RegisterForEvents();
            void UnregisterForEvents();
            void FinalizeRelease();
            void ViewConsolidated(Windows::UI::ViewManagement::ApplicationView^ sender, Windows::UI::ViewManagement::ApplicationViewConsolidatedEventArgs^ e);
            void VisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ e);
            event ViewDataReleasedHandler^ InternalReleased;
            std::mutex globalMutex;

            property bool Consolidated
            {
                bool get();
                void set (bool value);
            }


            Windows::Foundation::EventRegistrationToken consolidatedToken, visibilityToken;
            Platform::Agile<Windows::UI::Core::CoreWindow^> window;
            Windows::UI::Core::CoreDispatcher^ dispatcher;
            Platform::String^ title;
            int refCount;
            int viewId;
            bool consolidated, released;            
        };
    }
}
