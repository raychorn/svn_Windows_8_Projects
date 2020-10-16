// Copyright (c) Microsoft Corporation.  All rights reserved.

#include "StreamUriResolver.h"
#include <ppltasks.h>
#include <strsafe.h>
#include <assert.h>

using namespace SDK::WebViewSampleCpp;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography;
using namespace concurrency;

namespace SDK
{
namespace WebViewSampleCpp
{

IAsyncOperation<IInputStream^>^ StreamUriResolver::UriToStreamAsync(Uri^ uri)
{
    // The WebView's buildLocalStreamUri method takes contentIdentifier and relativePath
    // parameters to generate a URI of the form:
    //     ms-local-stream://<package name>_<encoded contentIdentifier>/<relativePath>
    // The resolver can decode the contentIdentifier to determine what content should be
    // returned in the output stream.
    PCWSTR encodedContentIdentifier = wcschr(uri->Host->Data(), L'_');
    if (encodedContentIdentifier == nullptr)
    {
        throw ref new InvalidArgumentException();
    }
    else
    {
        encodedContentIdentifier++;
    }

    IBuffer^ buffer = CryptographicBuffer::DecodeFromHexString(ref new String(encodedContentIdentifier));
    String^ contentIdentifier = CryptographicBuffer::ConvertBinaryToString(BinaryStringEncoding::Utf8, buffer);
    String^ relativePath = ref new String(uri->Path->Data());

    // For this sample, we will return a stream for a file under the local app data
    // folder, under the subfolder named after the contentIdentifier and having the
    // given relativePath.  Real apps can have more complex behavior, such as handling
    // contentIdentifiers in a custom manner (not necessarily as paths), and generating
    // arbitrary streams that are not read directly from a file.
    Uri^ appDataUri = ref new Uri(L"ms-appdata:///local/" + contentIdentifier + relativePath);
    return GetFileStreamFromApplicationUriAsync(appDataUri);
}

IAsyncOperation<IInputStream^>^ StreamUriResolver::GetFileStreamFromApplicationUriAsync(Uri^ uri)
{
    return create_async([this, uri]()->IInputStream^
    {
        task<StorageFile^> getFileTask(StorageFile::GetFileFromApplicationUriAsync(uri));

        task<IInputStream^> getInputStreamTask = getFileTask.then([](StorageFile^ storageFile)
        {
            return storageFile->OpenAsync(FileAccessMode::Read);
        }).then([](IRandomAccessStream^ stream)
        {
            return stream->GetInputStreamAt(0);
        });

        return getInputStreamTask.get();
    });
}

} // namespace WebViewSampleCpp
} // namespace SDK
