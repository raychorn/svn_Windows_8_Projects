// This class is a useful way to keep track of which view you have open,
// and to make sure those views do not get closed before you're done with them

#include "pch.h"
#include "ViewLifetimeControl.h"

using namespace SDKSample::Projection;

using namespace Platform;
using namespace std;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml::Data;

ViewLifetimeControl::ViewLifetimeControl(CoreWindow^ newWindow) : consolidated(true)
{
    dispatcher = newWindow->Dispatcher;
    window = newWindow;
    viewId = ApplicationView::GetApplicationViewIdForWindow(window.Get());

    // This class will automatically tell the view when its time to close
    // or stay alive in a few cases
    RegisterForEvents();
}

void ViewLifetimeControl::RegisterForEvents()
{
    // Views that are consolidated are no longer accessible to the user,
    // so it's a good idea to close them.
    consolidatedToken = ApplicationView::GetForCurrentView()->Consolidated += ref new TypedEventHandler<ApplicationView ^, ApplicationViewConsolidatedEventArgs ^>(this, &ViewLifetimeControl::ViewConsolidated);

    // On the other hand, if a view becomes visible, the user is engaging
    // with it, so it's a good idea not to close it
    visibilityToken = window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &ViewLifetimeControl::VisibilityChanged);
}

void ViewLifetimeControl::UnregisterForEvents()
{
    ApplicationView::GetForCurrentView()->Consolidated -= consolidatedToken;
    window->VisibilityChanged -= visibilityToken;
}

void ViewLifetimeControl::VisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ e)
{
    if (e->Visible)
    {
        // If a view becomes visible, the user is engaging
        // with it, so it's a good idea not to close it
        Consolidated = false;
    }
}

void ViewLifetimeControl::ViewConsolidated(ApplicationView^ sender, ApplicationViewConsolidatedEventArgs^ e)
{
    // Views that are consolidated are no longer accessible to the user,
    // so it's a good idea to close them.
    Consolidated = true;
}

// Called when a view has been "consolidated" (no longer accessible to the user) 
// and no other view is trying to interact with it. The view effectively
// shuts down in this message
void ViewLifetimeControl::FinalizeRelease()
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

// Creates ViewLifetimeControl for the particular view.
// Only do this once per view.
ViewLifetimeControl^ ViewLifetimeControl::CreateForCurrentView()
{
    return ref new ViewLifetimeControl(CoreWindow::GetForCurrentThread());
}


CoreDispatcher^ ViewLifetimeControl::Dispatcher::get()
{
    // This property never changes, so there's no need to lock
    return dispatcher;
}

int ViewLifetimeControl::Id::get()
{
    // This property never changes, so there's no need to lock
    return viewId;
}

bool ViewLifetimeControl::Consolidated::get()
{
    return consolidated;
}

void ViewLifetimeControl::Consolidated::set(bool value)
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
int ViewLifetimeControl::StartViewInUse()
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
int ViewLifetimeControl::StopViewInUse()
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
                ref new DispatchedHandler(this, &ViewLifetimeControl::FinalizeRelease));
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
EventRegistrationToken ViewLifetimeControl::Released::add(ViewReleasedHandler^ handler)
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

void ViewLifetimeControl::Released::remove(EventRegistrationToken token)
{
    lock_guard<mutex> lock(globalMutex);
    return InternalReleased -= token;
}