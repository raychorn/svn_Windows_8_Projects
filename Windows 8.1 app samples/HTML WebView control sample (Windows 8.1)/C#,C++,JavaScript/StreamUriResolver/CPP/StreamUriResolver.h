// Copyright (c) Microsoft Corporation.  All rights reserved.

#pragma once

namespace SDK
{
namespace WebViewSampleCpp
{
    public ref class StreamUriResolver sealed : public Windows::Web::IUriToStreamResolver
    {
    public:
        virtual Windows::Foundation::IAsyncOperation<Windows::Storage::Streams::IInputStream^>^ UriToStreamAsync(Windows::Foundation::Uri^ uri);

    private:
        Windows::Foundation::IAsyncOperation<Windows::Storage::Streams::IInputStream^>^ GetFileStreamFromApplicationUriAsync(Windows::Foundation::Uri^ uri);
    };

} // namespace WebViewSampleCpp
} // namespace SDK
