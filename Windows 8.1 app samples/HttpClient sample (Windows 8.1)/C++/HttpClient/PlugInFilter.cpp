#include "pch.h"
#include "PlugInFilter.h"
#include <ppltasks.h>

using namespace Concurrency;
using namespace Platform;
using namespace SDKSample::HttpClientSample;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;

PlugInFilter::PlugInFilter(IHttpFilter^ innerFilter)
{
    if (innerFilter == nullptr)
    {
        throw ref new Exception(E_INVALIDARG, "innerFilter cannot be null.");
    }
    this->innerFilter = innerFilter;
}

PlugInFilter::~PlugInFilter(void)
{
}

IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ PlugInFilter::SendRequestAsync(
    HttpRequestMessage^ request)
{
    request->Headers->Insert("Custom-Header", "CustomRequestValue");

    return create_async([=](progress_reporter<HttpProgress> reporter, cancellation_token token){
        HttpResponseMessage^ response = nullptr;

        task<HttpResponseMessage^> sendRequestTask(innerFilter->SendRequestAsync(request), token);
        sendRequestTask.wait();
        response = sendRequestTask.get();

        if (token.is_canceled())
        {
            cancel_current_task();
            return response;
        }

        response->Headers->Insert("Custom-Header", "CustomResponseValue");
        return response;
    });
}
