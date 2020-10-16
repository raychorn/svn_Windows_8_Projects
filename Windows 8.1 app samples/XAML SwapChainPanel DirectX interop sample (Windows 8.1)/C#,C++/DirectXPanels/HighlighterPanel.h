//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once
#include "pch.h"
#include "DirectXPanelBase.h"

namespace DirectXPanels
{
    // Hosts a DirectX rendering surface that supports drawing yellow ink.  If a MinBlend composite mode is applied to the panel, this will simulate a
    // highlighter effect. Ink is drawn in response to user input and does not synchronize with the vertical blanking interval, providing the lowest 
    // possible latency while drawing at the cost of additional CPU and GPU work. The content is not preserved, so it may disappear on suspend or
    // if the DirectX device is recreated.

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class HighlighterPanel sealed : public DirectXPanels::DirectXPanelBase
    {
    public:
        HighlighterPanel();

        void StartProcessingInput();
        void StopProcessingInput();        

    private protected:
        enum DrawingState {
            None = 0,
            Inking
        };

        virtual void Render() override;
        virtual void CreateDeviceResources() override;
        virtual void CreateSizeDependentResources() override;        

        void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

        virtual void Present() override;

        DrawingState                                                        m_drawingState;

        Windows::UI::Core::CoreIndependentInputSource^                      m_coreInput;
        Windows::Foundation::IAsyncAction^                                  m_inputLoopWorker;

        Microsoft::WRL::ComPtr<ID3D11Texture2D>                             m_currentBuffer;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>                             m_previousBuffer;

        Microsoft::WRL::ComPtr<ID2D1StrokeStyle>                            m_inkStrokeStyle;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>                        m_strokeBrush;

        Windows::Foundation::Point                                          m_previousPoint;
        unsigned int                                                        m_activePointerId;
    };
}