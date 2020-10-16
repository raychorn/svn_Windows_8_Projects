// This class is a useful way to keep track of which view you have open,
// and to make sure those views do not get closed before you're done with them

#include "pch.h"
#include "ViewData.h"

using namespace SDKSample::MultipleViews;

using namespace Platform;
using namespace std;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml::Data;

ViewData::ViewData(CoreWindow^ newWindow) : consolidated(true)
{
    dispatcher = newWindow->Dispatcher;
    window = newWindow;
    viewId = ApplicationView::GetApplicationViewIdForWindow(window.Get());

    // This class will automatically tell the view when its time to close
    // or stay alive in a few cases
    RegisterForEvents();
}

void ViewData::RegisterForEvents()
{
    // Views that are consolidated are no longer accessible to the user,
    // so it's a good idea to close them.
    consolidatedToken = ApplicationView::GetForCurrentView()->Consolidated += ref new TypedEventHandler<ApplicationView ^, ApplicationViewConsolidatedEventArgs ^>(this, &ViewData::ViewConsolidated);

    // On the other hand, if a view becomes visible, the user is engaging
    // with it, so it's a good idea not to close it
    visibilityToken = window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &ViewData::VisibilityChanged);
}

void ViewData::UnregisterForEvents()
{
    ApplicationView::GetForCurrentView()->Consolidated -= consolidatedToken;
    window->VisibilityChanged -= visibilityToken;
}

void ViewData::VisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ e)
{
    if (e->Visible)
    {
        // If a view becomes visible, the user is engaging
        // with it, so it's a good idea not to close it
        Consolidated = false;
    }
}

void ViewData::ViewConsolidated(ApplicationView^ sender, ApplicationViewConsolidatedEventArgs^ e)
{
    // Views that are consolidated are no longer accessible to the user,
    // so it's a good idea to close them.
    Consolidated = true;
}

// Called when a view has been "consolidated" (no longer accessible to the user) 
// and no other view is trying to interact with it. The view effectively
// shuts down in this message
void ViewData::FinalizeRelease()
{
    bool justReleased = false;
    globalMutex.lock();    
    if (!released && refCount == 0)
    {
        released = true;
        justReleased = true;
    }
    globalMutex.unlock();

    // This assumes that released will never be made false after it
    // it has been set to true
    if (justReleased)
    {
        UnregisterForEvents();
        InternalReleased(this, nullptr);
    }
}

// Creates ViewData for the particular view.
// Only do this once per view.
ViewData^ ViewData::CreateForCurrentView()
{
    return ref new ViewData(CoreWindow::GetForCurrentThread());
}

// For purposes of this sample, the collection of views
// is bound to a UI collection. This property is available for binding
String^ ViewData::Title::get()
{
    // This is expected to only be updated on the thread that this object is bound to
    // Hence, it's not necessary to lock
    return title;
}

void ViewData::Title::set(String^ value)
{
    // This is expected to only be updated on the thread that this object is bound to
    // Hence, it's not necessary to lock
    if (title != value)
    {
        title = value;
        PropertyChanged(this, ref new PropertyChangedEventArgs("Title"));
    }
}

CoreDispatcher^ ViewData::Dispatcher::get()
{
    // This property never changes, so there's no need to lock
    return dispatcher;
}

int ViewData::Id::get()
{
    // This property never changes, so there's no need to lock
    return viewId;
}

bool ViewData::Consolidated::get()
{
    return consolidated;
}

void ViewData::Consolidated::set(bool value)
{
    // This property should only be accessed by the thread on which the view lives, so there's no need to lock
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

// Signals that the view is being interacted with by another view,
// so it shouldn't be closed even if it becomes "consolidated"
int ViewData::StartViewInUse()
{
    bool releasedCopy = false;
    int refCountCopy = 0;
    
    globalMutex.lock();
    releasedCopy = released;
    if (!released)
    {
        refCountCopy = ++refCount;
    }
    globalMutex.unlock();

    if (releasedCopy)
    {
        throw ref new ObjectDisposedException("The view is being disposed");
    }

    return refCountCopy;
}

// Should come after any call to StartViewInUse
// Signals that the another view has finished interacting with the view tracked
// by this object
int ViewData::StopViewInUse()
{
    bool releasedCopy = false;
    int refCountCopy = -1;
    
    globalMutex.lock();
    releasedCopy = released;
    if (!released && refCount > 0)
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
            dispatcher->RunAsync(CoreDispatcherPriority::Low, 
                ref new DispatchedHandler(this, &ViewData::FinalizeRelease));
        }
    }
    globalMutex.unlock();

    if (releasedCopy)
    {
        throw ref new ObjectDisposedException("The view is being disposed");
    }

    if (refCountCopy < 0)
    {
        throw ref new Exception(HRESULT_FROM_WIN32(ERROR_INVALID_STATE), "Object was released too many times");
    }

    return refCountCopy;
}

// Signals to consumers that its time to close the view so that
// they can clean up (including calling Window.Close() when finished)
EventRegistrationToken ViewData::Released::add(ViewDataReleasedHandler^ handler)
{
    bool releasedCopy = false;
    EventRegistrationToken eventToken;
    globalMutex.lock();
    releasedCopy = released;
    if (!released)
    {
        eventToken = InternalReleased += handler;
    }
    globalMutex.unlock();

    if (releasedCopy)
    {
        throw ref new ObjectDisposedException("The view is being disposed");
    }

    return eventToken;
}

void ViewData::Released::remove(EventRegistrationToken token)
{
    lock_guard<mutex> lock(globalMutex);
    return InternalReleased -= token;
}