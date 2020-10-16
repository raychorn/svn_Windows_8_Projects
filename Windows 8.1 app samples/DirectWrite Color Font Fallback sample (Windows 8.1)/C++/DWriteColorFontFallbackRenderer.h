//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "DeviceResources.h"
#include "DWriteColorFontFallback.h"

namespace DWriteColorFontFallback
{
    // This sample renderer instantiates a basic rendering pipeline.
    class DWriteColorFontFallbackRenderer
    {
    public:
        DWriteColorFontFallbackRenderer(const std::shared_ptr<DeviceResources>& deviceResources);
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void ReleaseDeviceDependentResources();
        void Render(unsigned int fallback, bool colorGlyphs, float zoom, D2D1_POINT_2F point);

    private:
        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>        m_textBrush;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>        m_backgroundBrush;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>        m_actionBrush;

        Microsoft::WRL::ComPtr<IDWriteTextFormat1>          m_textFormatTitle;
        Microsoft::WRL::ComPtr<IDWriteTextFormat1>          m_textFormatSubTitle;
        Microsoft::WRL::ComPtr<IDWriteTextFormat1>          m_textFormatBody;
        Microsoft::WRL::ComPtr<IDWriteTextFormat1>          m_textFormatBodyWithCustomFontFallback;

        Microsoft::WRL::ComPtr<IDWriteFontFallback>         m_fallbackList[SampleConstants::MaxFontFallbackScenarios];
        Microsoft::WRL::ComPtr<IDWriteTextFormat1>          m_textFormat[SampleConstants::MaxTextBlocks];

        Microsoft::WRL::ComPtr<IDWriteTextLayout>           m_layout[SampleConstants::MaxTextBlocks];
        DWRITE_TEXT_METRICS                                 m_metrics[SampleConstants::MaxTextBlocks];
        D2D1_RECT_F                                         m_textRect;
        D2D1_RECT_F                                         m_pageRect;

        float                                               m_scaleFactor;
        unsigned int                                        m_fontFallbackId;
    };
}
