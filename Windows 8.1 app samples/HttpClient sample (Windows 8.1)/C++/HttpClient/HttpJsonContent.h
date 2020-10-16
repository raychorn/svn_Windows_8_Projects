#pragma once

#include "pch.h"

namespace SDKSample
{
    namespace HttpClientSample
    {
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class HttpJsonContent sealed : public Windows::Web::Http::IHttpContent
        {
        public:
            HttpJsonContent(Windows::Data::Json::IJsonValue^ jsonValue);
            virtual ~HttpJsonContent(void);
            virtual Windows::Foundation::IAsyncOperationWithProgress<
                unsigned long long,
                unsigned long long>^ BufferAllAsync();
            virtual Windows::Foundation::IAsyncOperationWithProgress<
                Windows::Storage::Streams::IBuffer^,
                unsigned long long>^ ReadAsBufferAsync();
            virtual Windows::Foundation::IAsyncOperationWithProgress<
                Windows::Storage::Streams::IInputStream^,
                unsigned long long>^ ReadAsInputStreamAsync();
            virtual Windows::Foundation::IAsyncOperationWithProgress<
                Platform::String^,
                unsigned long long>^ ReadAsStringAsync();
            virtual bool TryComputeLength(unsigned long long* length);
            virtual Windows::Foundation::IAsyncOperationWithProgress<
                unsigned long long,
                unsigned long long>^ WriteToStreamAsync(Windows::Storage::Streams::IOutputStream^ outputStream);

            property Windows::Web::Http::Headers::HttpContentHeaderCollection^ Headers
            {
                virtual Windows::Web::Http::Headers::HttpContentHeaderCollection^ get();
            };

        private:
            Windows::Data::Json::IJsonValue^ jsonValue;
            Windows::Web::Http::Headers::HttpContentHeaderCollection^ headers;

            unsigned long long GetLength();
        };
    }
}
