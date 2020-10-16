#pragma once

#include "pch.h"

namespace SDKSample
{
    namespace HttpClientSample
    {
        public ref class PlugInFilter sealed : public Windows::Web::Http::Filters::IHttpFilter
        {
        public:
            PlugInFilter(Windows::Web::Http::Filters::IHttpFilter^ innerFilter);
            virtual ~PlugInFilter(void);
            virtual Windows::Foundation::IAsyncOperationWithProgress<
                Windows::Web::Http::HttpResponseMessage^,
                Windows::Web::Http::HttpProgress>^ SendRequestAsync(Windows::Web::Http::HttpRequestMessage^ request);

        private:
            Windows::Web::Http::Filters::IHttpFilter^ innerFilter;
        };
    }
}
