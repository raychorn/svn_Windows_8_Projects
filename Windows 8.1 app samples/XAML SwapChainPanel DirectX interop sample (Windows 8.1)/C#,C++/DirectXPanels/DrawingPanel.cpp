//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once
#include "pch.h"
#include "DrawingPanel.h"
#include <DirectXMath.h>
#include <math.h>
#include <ppltasks.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include "DirectXHelper.h"

using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Input::Inking;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Interop;
using namespace Concurrency;
using namespace DirectX;
using namespace D2D1;
using namespace DirectXPanels;
using namespace DX;

// Register dependency properties.
DependencyProperty^ DrawingPanel::m_brushColorProperty = DependencyProperty::Register("BrushColor", Color::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushColorValueChanged)));
DependencyProperty^ DrawingPanel::m_brushFitsToCurveProperty = DependencyProperty::Register("BrushFitsToCurve", bool::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushFitsToCurveValueChanged)));
DependencyProperty^ DrawingPanel::m_brushSizeProperty = DependencyProperty::Register("BrushSize", Windows::Foundation::Size::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushSizeValueChanged)));
DependencyProperty^ DrawingPanel::m_brushIsEraserProperty = DependencyProperty::Register("BrushIsEraser", bool::typeid, DrawingPanel::typeid, ref new PropertyMetadata(nullptr, ref new PropertyChangedCallback(&DrawingPanel::BrushIsEraserValueChanged)));

DrawingPanel::DrawingPanel() :
    m_drawingState(DrawingState::Uninitialized),
    m_currentStrokeIndex(0),
    m_currentStrokeSegmentIndex(0),
    m_activePointerId(0)
{
    critical_section::scoped_lock lock(m_criticalSection);

    CreateDeviceIndependentResources();
    CreateDeviceResources();
    CreateSizeDependentResources();
}

void DrawingPanel::StartProcessingInput()
{
    // Initialize the rendering surface and prepares it to receive input.	

    // Create a task to register for independent input and begin processing input messages.
    auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
    {
        // Create ink manager and set drawing attributes.
        m_inkManager = ref new InkManager();
        m_inkDrawingAttributes = ref new InkDrawingAttributes();

        m_inkDrawingAttributes->Color = m_brushColor;
        m_inkDrawingAttributes->FitToCurve = m_brushFitsToCurve;
        m_inkDrawingAttributes->Size = m_brushSize;
        m_inkManager->SetDefaultDrawingAttributes(m_inkDrawingAttributes);

        m_drawingState = DrawingState::None;

        // The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
        m_coreInput = CreateCoreIndependentInputSource(
            Windows::UI::Core::CoreInputDeviceTypes::Mouse |
            Windows::UI::Core::CoreInputDeviceTypes::Touch |
            Windows::UI::Core::CoreInputDeviceTypes::Pen
            );

        // Register for pointer events, which will be raised on the background thread.
        m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DrawingPanel::OnPointerPressed);
        m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DrawingPanel::OnPointerMoved);
        m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DrawingPanel::OnPointerReleased);

        // Begin processing input messages as they're delivered.
        m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    });

    // Run task on a dedicated high priority background thread.
    m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DrawingPanel::StopProcessingInput()
{
    // A call to ProcessEvents() with the ProcessUntilQuit flag will only return by default when the window closes.
    // Calling StopProcessEvents allows ProcessEvents to return even if the window isn't closing so the background thread can exit.
    m_coreInput->Dispatcher->StopProcessEvents();
}

#pragma region DependencyProperty change handlers
void DrawingPanel::BrushColorValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
    auto panel = safe_cast<DrawingPanel^>(sender) ;
    auto color = safe_cast<Color>(e->NewValue);

    critical_section::scoped_lock lock(panel->m_criticalSection);
    panel->m_brushColor = color;
    panel->m_d2dContext->CreateSolidColorBrush(DX::ConvertToColorF(color), &panel->m_strokeBrush);

    if (panel->m_drawingState != DrawingState::Uninitialized)
    {
        panel->m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ] ()
        {
            panel->m_inkDrawingAttributes->Color = color;
            panel->m_inkManager->SetDefaultDrawingAttributes(panel->m_inkDrawingAttributes);
        }));
    }
}

void DrawingPanel::BrushFitsToCurveValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
    auto panel = safe_cast<DrawingPanel^>(sender);
    auto value = safe_cast<bool>(e->NewValue);
    
    panel->m_brushFitsToCurve = value;

    if (panel->m_drawingState != DrawingState::Uninitialized)
    {
        panel->m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ] ()
        {
            panel->m_inkDrawingAttributes->FitToCurve = value;
            panel->m_inkManager->SetDefaultDrawingAttributes(panel->m_inkDrawingAttributes);
        }));
    }    
}

void DrawingPanel::BrushSizeValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
    auto panel = safe_cast<DrawingPanel^>(sender);
    auto value = safe_cast<Windows::Foundation::Size>(e->NewValue);

    panel->m_brushSize = value;
    
    if (panel->m_drawingState != DrawingState::Uninitialized)
    {
        panel->m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ] ()
        {
            panel->m_inkDrawingAttributes->Size = value;
            panel->m_inkManager->SetDefaultDrawingAttributes(panel->m_inkDrawingAttributes);
        }));
    }
}

void DrawingPanel::BrushIsEraserValueChanged(DependencyObject^ sender, DependencyPropertyChangedEventArgs^ e)
{
    auto panel = safe_cast<DrawingPanel^>(sender);
    auto value = safe_cast<bool>(e->NewValue);

    panel->m_brushIsEraser = value;
}
#pragma endregion

void DrawingPanel::CreateDeviceResources()
{
    DirectXPanelBase::CreateDeviceResources();

    // Create ink stroke style.
    ThrowIfFailed(
        m_d2dFactory->CreateStrokeStyle(
        StrokeStyleProperties(
        D2D1_CAP_STYLE_ROUND,
        D2D1_CAP_STYLE_ROUND,
        D2D1_CAP_STYLE_ROUND,
        D2D1_LINE_JOIN_ROUND,
        1.0f,
        D2D1_DASH_STYLE_SOLID,
        0.0f),
        nullptr,
        0,
        &m_inkStrokeStyle)
        );

    // Create ink stroke brush.
    ThrowIfFailed(
        m_d2dContext->CreateSolidColorBrush(ConvertToColorF(BrushColor), &m_strokeBrush)
        );

    // Enable multithreaded usage.
    ComPtr<ID3D10Multithread> d3dDeviceMultithreaded;
    m_d3dDevice.Get()->QueryInterface(__uuidof(ID3D10Multithread) , &d3dDeviceMultithreaded);
    d3dDeviceMultithreaded->SetMultithreadProtected(TRUE);

    if (!d3dDeviceMultithreaded->GetMultithreadProtected())
    {
        throw Platform::Exception::CreateException(E_FAIL);
    }
}

void DrawingPanel::CreateSizeDependentResources()
{
    m_currentBuffer.Reset();
    m_previousBuffer.Reset();

    DirectXPanelBase::CreateSizeDependentResources();

    ThrowIfFailed(
        m_swapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D) ,
        &m_currentBuffer
        )
        );

    ThrowIfFailed(
        m_swapChain->GetBuffer(
        1,
        __uuidof(ID3D11Texture2D) ,
        &m_previousBuffer
        )
        );
}

void DrawingPanel::Render()
{
    // Render and present the DirectX content.

    if (m_drawingState != DrawingState::Uninitialized)
    {
        m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ] ()
        {
            m_d2dContext->BeginDraw();

            m_d2dContext->Clear(m_backgroundColor);

            RenderCompletedStrokes();

            HRESULT hr = m_d2dContext->EndDraw();
            if (hr == D2DERR_RECREATE_TARGET)
            {
                CreateSizeDependentResources();
            }
            else
            {
                ThrowIfFailed(hr);
            }

            Present();
        }));
    }   
}

void DrawingPanel::Update()
{
    critical_section::scoped_lock lock(m_criticalSection);

    Render();
}

void DrawingPanel::Present()
{
    DXGI_PRESENT_PARAMETERS parameters = { 0 };
    parameters.DirtyRectsCount = 0;
    parameters.pDirtyRects = nullptr;
    parameters.pScrollRect = nullptr;
    parameters.pScrollOffset = nullptr;

    HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        // If the device was lost, recreate resources and attempt to render again.
        OnDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);
    }
}

void DrawingPanel::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
    // Handle the PointerPressed event, which will be raised on a background thread.

    if (m_drawingState == DrawingState::None)
    {
        critical_section::scoped_lock lock(m_criticalSection);

        // Store active pointer ID: only one contact can be inking at a time.
        m_activePointerId = e->CurrentPoint->PointerId;

        if (BrushIsEraser || e->CurrentPoint->Properties->IsEraser)
        {
            m_drawingState = DrawingState::Erasing;
            m_inkManager->Mode = InkManipulationMode::Erasing;
        }
        else
        {
            m_drawingState = DrawingState::Inking;
            m_inkManager->Mode = InkManipulationMode::Inking;
        }

        auto pointerPoint = e->CurrentPoint;
        m_previousPoint = pointerPoint->Position;
        // Pass pointer information to ink manager
        m_inkManager->ProcessPointerDown(pointerPoint);
    }
}

void DrawingPanel::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
    // Handle the PointerMoved event, which will be raised on a background thread.    

    if (m_drawingState == DrawingState::Inking && e->CurrentPoint->PointerId == m_activePointerId)
    {
        critical_section::scoped_lock lock(m_criticalSection);

        RenderActiveStroke(e->CurrentPoint);

        // Pass pointer information to ink manager.        
        m_inkManager->ProcessPointerUpdate(e->CurrentPoint);
    }
    else if (m_drawingState == DrawingState::Erasing)
    {
        critical_section::scoped_lock lock(m_criticalSection);

        // Pass pointer information to ink manager.        
        auto invalidatedRect = safe_cast<Windows::Foundation::Rect>(m_inkManager->ProcessPointerUpdate(e->CurrentPoint));

        // Re-render the current scene if any strokes were erased.
        if (invalidatedRect.Width > 0 && invalidatedRect.Height > 0)
        {
            Render();
        }
    }
}

void DrawingPanel::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
    // Handle the PointerReleased event, which will be raised on a background thread.

    if (m_drawingState == DrawingState::Inking || m_drawingState == DrawingState::Erasing)
    {
        critical_section::scoped_lock lock(m_criticalSection);

        m_drawingState = DrawingState::None;
        auto pointerPoint = e->CurrentPoint;
        // Pass pointer information to ink manager.
        m_inkManager->ProcessPointerUp(pointerPoint);

        // Reset active pointer ID.
        m_activePointerId = 0;

        Render();

        m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ] ()
        {
            if (m_inkManager->GetStrokes()->Size > 0)
            {
                concurrency::task<IVectorView<InkRecognitionResult^>^> recognizeTask(m_inkManager->RecognizeAsync(InkRecognitionTarget::Recent));
                recognizeTask.then([this] (IVectorView<InkRecognitionResult^>^ recognitionResults)
                {
                    auto results = ref new Array<String^>(recognitionResults->Size);

                    this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ]()
                    {
                        // Get top recognition candidates.
                        for (unsigned int i = 0; i < recognitionResults->Size; i++)
                        {
                            results[i] = recognitionResults->GetAt(i)->GetTextCandidates()->GetAt(0);
                        }

                        RecognitionResultsUpdated(this, ref new RecognitionResultUpdatedEventArgs(results));
                    }, CallbackContext::Any));
                });
            }
            else
            {
                this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ]()
                {
                    RecognitionResultsUpdated(this, ref new RecognitionResultUpdatedEventArgs(ref new Array<String^>(0)));
                }, CallbackContext::Any));
            }
        }));
    }
}

void DrawingPanel::RenderActiveStroke(PointerPoint^ newPoint)
{
    // Must be called on the background thread.

    // While actively inking, copy the last presented buffer to the current buffer since it contains the live ink strokes
    // drawn in response to previous pointer move events, then draw the current stroke segment.
    m_d3dContext->CopyResource(m_currentBuffer.Get(), m_previousBuffer.Get());

    m_d2dContext->BeginDraw();

    Point newPosition = newPoint->Position;

    m_d2dContext->DrawLine(
        Point2F(m_previousPoint.X, m_previousPoint.Y),
        Point2F(newPosition.X, newPosition.Y),
        m_strokeBrush.Get(),
        BrushSize.Height,
        m_inkStrokeStyle.Get()
        );

    m_previousPoint = newPosition;

    HRESULT hr = m_d2dContext->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET)
    {
        CreateSizeDependentResources();
    }
    else
    {
        ThrowIfFailed(hr);
    }
    
    Present();
}

void DrawingPanel::RenderCompletedStrokes(unsigned int strokeCount)
{
    // Must be called on the background thread.

    // Get previous ink strokes from the ink manager up to the given count and render them.
    auto strokes = m_inkManager->GetStrokes();

    for (unsigned int i = 0; i < strokeCount; i++)
    {
        auto stroke = strokes->GetAt(i);
        ComPtr<ID2D1PathGeometry> strokeGeometry;
        ConvertStrokeToGeometry(stroke, &strokeGeometry);
        
        // Create brush from current stroke
        ComPtr<ID2D1SolidColorBrush> strokeBrush;
        ThrowIfFailed(
            m_d2dContext->CreateSolidColorBrush(ConvertToColorF(stroke->DrawingAttributes->Color), &strokeBrush)
            );

        m_d2dContext->DrawGeometry(strokeGeometry.Get(), strokeBrush.Get(), stroke->DrawingAttributes->Size.Width, m_inkStrokeStyle.Get());
    }
}

// Converts bezier control points in each segment to a path geometry bezier curve.
void DrawingPanel::ConvertStrokeToGeometry(InkStroke^ stroke, unsigned int segmentCount, ID2D1PathGeometry** geometry)
{
    // Create geometry path.
    ThrowIfFailed(
        m_d2dFactory->CreatePathGeometry(geometry)
        );

    // Create and initialize geometry sink.
    ComPtr<ID2D1GeometrySink> sink;
    ThrowIfFailed(
        (*geometry)->Open(&sink)
        );

    sink->SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);
    sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

    auto strokeSegments = stroke->GetRenderingSegments();

    // Set starting point to position of first stroke segment.
    sink->BeginFigure(ConvertToPoint2F(strokeSegments->GetAt(0)->Position), D2D1_FIGURE_BEGIN_FILLED);

    // Add bezier segments for the remaining stroke segments to geometry path sink.
    for (unsigned int i = 1; i < segmentCount; i++)
    {
        auto strokeSegment = strokeSegments->GetAt(i);
        sink->AddBezier(
            D2D1::BezierSegment(
            ConvertToPoint2F(strokeSegment->BezierControlPoint1),
            ConvertToPoint2F(strokeSegment->BezierControlPoint2),
            ConvertToPoint2F(strokeSegment->Position)
            )
            );
    }

    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    ThrowIfFailed(
        sink->Close()
        );
}

// Saves active ink strokes to the specified stream as an image.
IAsyncAction^ DrawingPanel::SaveStrokesToStreamAsync(IRandomAccessStream^ stream)
{
    return m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ] ()
    {
        create_task(m_inkManager->SaveAsync(stream)).wait();        
    }));
}

// Loads saved strokes from the specified stream.
IAsyncAction^ DrawingPanel::LoadStrokesFromStreamAsync(IRandomAccessStream^ stream)
{
    return m_coreInput->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([= ] ()
    {
        create_task(m_inkManager->LoadAsync(stream)).wait();
    }));
}

// Loads saved strokes from the specified stream and redraws them over time in the order they were drawn.
void DrawingPanel::BeginStrokesReplayFromStream(IRandomAccessStream^ stream, int intervalInMilliseconds)
{
    m_drawingState = DrawingState::Replaying;
    m_currentStrokeIndex = 0;
    m_currentStrokeSegmentIndex = 0;

    if (m_replayTimer != nullptr)
    {
        m_replayTimer->Cancel();
    }

    // Delegate for the periodic timer that replays strokes.
    auto timerDelegate = [= ](ThreadPoolTimer^ timer)
    {
        critical_section::scoped_lock lock(m_criticalSection);

        auto strokes = m_inkManager->GetStrokes();
        auto stroke = strokes->GetAt(m_currentStrokeIndex);
        m_currentStrokeSegmentIndex++;

        if (m_currentStrokeSegmentIndex >= stroke->GetRenderingSegments()->Size)
        {
            // Move to next stroke.
            m_currentStrokeIndex++;
            m_currentStrokeSegmentIndex = 0;
        }

        // Convert segments (up to the current segment index) of the current stroke to a single D2D geometry
        ComPtr<ID2D1PathGeometry> strokeGeometry;
        ConvertStrokeToGeometry(stroke, m_currentStrokeSegmentIndex, &strokeGeometry);

        // Create brush from stroke
        ComPtr<ID2D1SolidColorBrush> strokeBrush;
        ThrowIfFailed(
            m_d2dContext->CreateSolidColorBrush(ConvertToColorF(stroke->DrawingAttributes->Color), &strokeBrush)
            );

        m_d2dContext->BeginDraw();

        m_d2dContext->Clear(m_backgroundColor);

        // Render any strokes that have already been completely replayed.
        if (m_currentStrokeIndex > 0)
        {
            RenderCompletedStrokes(m_currentStrokeIndex);
        }

        // Render segments of the current stroke.
        m_d2dContext->DrawGeometry(strokeGeometry.Get(), strokeBrush.Get(), stroke->DrawingAttributes->Size.Width, m_inkStrokeStyle.Get());

        HRESULT hr = m_d2dContext->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            CreateSizeDependentResources();
        }
        else
        {
            ThrowIfFailed(hr);
        }
        Present();

        if (m_currentStrokeIndex >= strokes->Size)
        {
            // Finished all strokes, so cancel the periodic timer.
            m_replayTimer->Cancel();
            m_drawingState = DrawingState::None;
        }
    };

    // Load strokes from stream, then start a periodic timer with given interval.
    task<void> loadTask(m_inkManager->LoadAsync(stream));
    loadTask.then([= ] ()
    {
        TimeSpan period;
        period.Duration = intervalInMilliseconds * 10000; // Duration is in hundreds of ns.
        m_replayTimer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler(timerDelegate), period);
    });
}

// Stops replaying current strokes.
void DrawingPanel::StopStrokesReplay()
{
    if (m_drawingState == DrawingState::Replaying)
    {
        critical_section::scoped_lock lock(m_criticalSection);

        m_currentStrokeIndex = 0;
        m_currentStrokeSegmentIndex = 0;

        if (m_replayTimer != nullptr)
        {
            m_replayTimer->Cancel();
        }

        m_drawingState = DrawingState::None;

        Render();
    }
}
