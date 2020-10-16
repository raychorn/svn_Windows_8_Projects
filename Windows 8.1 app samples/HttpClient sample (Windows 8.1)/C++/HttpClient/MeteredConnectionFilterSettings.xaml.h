//
// MeteredConnectionFilterSettings.xaml.h
// Declaration of the MeteredConnectionFilterSettings class
//

#pragma once

#include "MeteredConnectionFilterSettings.g.h"

namespace SDKSample
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class MeteredConnectionFilterSettings sealed
    {
    public:
        MeteredConnectionFilterSettings(HttpFilters::HttpMeteredConnectionFilter^ meteredConnectionFilter);
        void OptInSwitch_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    private:
        HttpFilters::HttpMeteredConnectionFilter^ meteredConnectionFilter;
    };
}
