//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once
#include "pch.h"
#include "DirectXPanelBase.h"
#include "StepTimer.h"
#include "ShaderStructures.h"

namespace DirectXPanels
{
    // Hosts a DirectX rendering surface that draws a simple tic-tac-toe board using Direct2D.  This panel optionally
    // updates the size of its swap chain to match the current composition scale if the IsSwapChainSizeScaled property is set
    // to true.

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class D2DPanel sealed : public DirectXPanels::DirectXPanelBase
    {
    public:
        D2DPanel();

    private protected:
        virtual void Render() override;
        virtual void CreateDeviceResources() override;

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>                        m_strokeBrush;
    };
}