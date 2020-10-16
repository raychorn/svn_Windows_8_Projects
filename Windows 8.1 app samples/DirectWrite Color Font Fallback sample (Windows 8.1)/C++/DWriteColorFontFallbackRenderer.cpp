//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "DWriteColorFontFallbackRenderer.h"

#include "DirectXHelper.h" // For ThrowIfFailed and ReadDataAsync

using namespace DWriteColorFontFallback;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;
using namespace D2D1;

DWriteColorFontFallbackRenderer::DWriteColorFontFallbackRenderer(const std::shared_ptr<DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources),
    m_scaleFactor(1.0f),
    m_fontFallbackId(0)
{
    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    // Create a series of Font fallbacks
    // Emoji
    // Emoji -> System Default
    // Emoji -> Symbol
    // Emoji -> Symbol -> System Default
    // Symbol
    // Symbol -> System Default

    ComPtr<IDWriteFontFallbackBuilder> fallbackBuilder;
    ComPtr<IDWriteFontFallback> systemFallback;

    DX::ThrowIfFailed(dwriteFactory->GetSystemFontFallback(&systemFallback));
    m_fallbackList[SampleConstants::FontFallbackSystem] = systemFallback;

    DWRITE_UNICODE_RANGE range[] = {
        {0x00000, 0xffffffff},
    };
    WCHAR const* fallbackEmoji[1] = {
        SampleConstants::EmojiFontFamilyName,
    };

    DX::ThrowIfFailed(dwriteFactory->CreateFontFallbackBuilder(&fallbackBuilder));
    DX::ThrowIfFailed(fallbackBuilder->AddMapping(range, 1, fallbackEmoji, 1));
    DX::ThrowIfFailed(fallbackBuilder->CreateFontFallback(&m_fallbackList[SampleConstants::FontFallbackEmoji]));

    DX::ThrowIfFailed(dwriteFactory->CreateFontFallbackBuilder(&fallbackBuilder));
    DX::ThrowIfFailed(fallbackBuilder->AddMapping(range, 1, fallbackEmoji, 1));
    DX::ThrowIfFailed(fallbackBuilder->AddMappings(systemFallback.Get()));
    DX::ThrowIfFailed(fallbackBuilder->CreateFontFallback(&m_fallbackList[SampleConstants::FontFallbackEmojiSystem]));

    WCHAR const* fallbackEmojiSymbol[2] = {
        SampleConstants::EmojiFontFamilyName,
        SampleConstants::SymbolFontFamilyName,
    };

    DX::ThrowIfFailed(dwriteFactory->CreateFontFallbackBuilder(&fallbackBuilder));
    DX::ThrowIfFailed(fallbackBuilder->AddMapping(range, 1, fallbackEmojiSymbol, 2));
    DX::ThrowIfFailed(fallbackBuilder->CreateFontFallback(&m_fallbackList[SampleConstants::FontFallbackEmojiSymbol]));

    DX::ThrowIfFailed(dwriteFactory->CreateFontFallbackBuilder(&fallbackBuilder));
    DX::ThrowIfFailed(fallbackBuilder->AddMapping(range, 1, fallbackEmojiSymbol, 2));
    DX::ThrowIfFailed(fallbackBuilder->AddMappings(systemFallback.Get()));
    DX::ThrowIfFailed(fallbackBuilder->CreateFontFallback(&m_fallbackList[SampleConstants::FontFallbackEmojiSymbolSystem]));

    WCHAR const* fallbackSymbol[1] = {
        SampleConstants::SymbolFontFamilyName
    };

    DX::ThrowIfFailed(dwriteFactory->CreateFontFallbackBuilder(&fallbackBuilder));
    DX::ThrowIfFailed(fallbackBuilder->AddMapping(range, 1, fallbackSymbol, 1));
    DX::ThrowIfFailed(fallbackBuilder->CreateFontFallback(&m_fallbackList[SampleConstants::FontFallbackSymbol]));

    DX::ThrowIfFailed(dwriteFactory->CreateFontFallbackBuilder(&fallbackBuilder));
    DX::ThrowIfFailed(fallbackBuilder->AddMapping(range, 1, fallbackSymbol, 1));
    DX::ThrowIfFailed(fallbackBuilder->AddMappings(systemFallback.Get()));
    DX::ThrowIfFailed(fallbackBuilder->CreateFontFallback(&m_fallbackList[SampleConstants::FontFallbackSymbolSystem]));

    ComPtr<IDWriteTextFormat> textFormat;

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextFormat(
            SampleConstants::TitleFontFamilyName,
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            SampleConstants::TitleFontSizeDIPS,
            SampleConstants::LocaleName,
            &textFormat
            )
        );
    DX::ThrowIfFailed(textFormat.As(&m_textFormatTitle));
    DX::ThrowIfFailed(m_textFormatTitle->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    DX::ThrowIfFailed(m_textFormatTitle->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextFormat(
            SampleConstants::SubTitleFontFamilyName,
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            SampleConstants::SubTitleFontSizeDIPS,
            SampleConstants::LocaleName,
            &textFormat
            )
        );
    DX::ThrowIfFailed(textFormat.As(&m_textFormatSubTitle));
    DX::ThrowIfFailed(m_textFormatSubTitle->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    DX::ThrowIfFailed(m_textFormatSubTitle->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextFormat(
            SampleConstants::BodyFontFamilyName,
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            SampleConstants::BodyFontSizeDIPS,
            SampleConstants::LocaleName,
            &textFormat
            )
        );
    DX::ThrowIfFailed(textFormat.As(&m_textFormatBody));
    DX::ThrowIfFailed(m_textFormatBody->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    DX::ThrowIfFailed(m_textFormatBody->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextFormat(
            SampleConstants::BodyFontFamilyName,
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            SampleConstants::BodyFontSizeDIPS,
            SampleConstants::LocaleName,
            &textFormat
            )
        );
    DX::ThrowIfFailed(textFormat.As(&m_textFormatBodyWithCustomFontFallback));
    DX::ThrowIfFailed(m_textFormatBodyWithCustomFontFallback->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    DX::ThrowIfFailed(m_textFormatBodyWithCustomFontFallback->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

void DWriteColorFontFallbackRenderer::CreateDeviceDependentResources()
{
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();

    DX::ThrowIfFailed(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &m_textBrush
            )
        );
    DX::ThrowIfFailed(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::AntiqueWhite),
            &m_backgroundBrush
            )
        );
    DX::ThrowIfFailed(
        d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(0xdb7100, 1.0f),
            &m_actionBrush
            )
        );
}

// Initialization.
void DWriteColorFontFallbackRenderer::CreateWindowSizeDependentResources()
{
    auto windowBounds = m_deviceResources->GetWindowBounds();

    m_pageRect.top = SampleConstants::TopMargin;
    m_pageRect.left = SampleConstants::LeftMargin;
    m_pageRect.right = windowBounds.Width - SampleConstants::RightMargin;
    m_pageRect.bottom = windowBounds.Height - SampleConstants::BottomMargin;

    m_textRect.top = m_pageRect.top + SampleConstants::TextMargin;
    m_textRect.left = m_pageRect.left + SampleConstants::TextMargin;
    m_textRect.right = m_pageRect.right - SampleConstants::TextMargin;
    m_textRect.bottom = m_pageRect.bottom - SampleConstants::TextMargin;

    auto dwriteFactory = m_deviceResources->GetDWriteFactory();

    Platform::String^ string1 = ref new Platform::String(SampleConstants::TextStrings[0]);
    Platform::String^ string2 = ref new Platform::String(SampleConstants::TextStrings[1]);
    Platform::String^ string3 = ref new Platform::String(SampleConstants::TextStrings[2]);
    Platform::String^ string4 = ref new Platform::String(SampleConstants::FontFallbackDesc[m_fontFallbackId]);

    DWRITE_TEXT_METRICS metrics = {0};
    FLOAT layoutHeight = 0.0f;

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextLayout(
            string1->Data(),
            string1->Length(),
            m_textFormatTitle.Get(),
            m_textRect.right - m_textRect.left,
            m_textRect.bottom - m_textRect.top - layoutHeight,
            &m_layout[0]
            )
        );

    DX::ThrowIfFailed(m_layout[0]->GetMetrics(&metrics));
    m_metrics[0] = metrics;
    layoutHeight += metrics.height;

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextLayout(
            string2->Data(),
            string2->Length(),
            m_textFormatBody.Get(),
            m_textRect.right - m_textRect.left,
            m_textRect.bottom - m_textRect.top - layoutHeight,
            &m_layout[1]
            )
        );

    DX::ThrowIfFailed(m_layout[1]->GetMetrics(&metrics));
    m_metrics[1] = metrics;
    layoutHeight += metrics.height;

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextLayout(
            string3->Data(),
            string3->Length(),
            m_textFormatTitle.Get(),
            m_textRect.right - m_textRect.left,
            m_textRect.bottom - m_textRect.top - layoutHeight,
            &m_layout[2]
            )
        );

    DX::ThrowIfFailed(m_layout[2]->GetMetrics(&metrics));
    m_metrics[2] = metrics;
    layoutHeight += metrics.height;

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextLayout(
            string4->Data(),
            string4->Length(),
            m_textFormatSubTitle.Get(),
            m_textRect.right - m_textRect.left,
            m_textRect.bottom - m_textRect.top - layoutHeight,
            &m_layout[3]
            )
        );

    DX::ThrowIfFailed(m_layout[3]->GetMetrics(&metrics));
    m_metrics[3] = metrics;
    layoutHeight += metrics.height;

    DX::ThrowIfFailed(
        dwriteFactory->CreateTextLayout(
            string2->Data(),
            string2->Length(),
            m_textFormatBodyWithCustomFontFallback.Get(),
            m_textRect.right - m_textRect.left,
            m_textRect.bottom - m_textRect.top - layoutHeight,
            &m_layout[4]
            )
        );

    DX::ThrowIfFailed(m_layout[4]->GetMetrics(&metrics));
    m_metrics[4] = metrics;
    layoutHeight += metrics.height;
}

// Release all references to resources that depend on the graphics device.
// This method is invoked when the device is lost and resources are no longer usable.
void DWriteColorFontFallbackRenderer::ReleaseDeviceDependentResources()
{
    m_textBrush.Reset();
    m_backgroundBrush.Reset();
    m_actionBrush.Reset();
}

// Renders one frame.
void DWriteColorFontFallbackRenderer::Render(unsigned int fallback, bool colorGlyphs, float zoom, D2D1_POINT_2F point)
{
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();
    auto windowBounds = m_deviceResources->GetWindowBounds();

    if (m_fontFallbackId != fallback)
    {
        if (fallback < SampleConstants::MaxFontFallbackScenarios)
        {
            m_fontFallbackId = fallback;
            DX::ThrowIfFailed(m_textFormatBodyWithCustomFontFallback->SetFontFallback(m_fallbackList[m_fontFallbackId].Get()));
        }
        else
        {
            m_fontFallbackId = SampleConstants::FontFallbackSystem;
            DX::ThrowIfFailed(m_textFormatBodyWithCustomFontFallback->SetFontFallback(nullptr));
        }
        CreateWindowSizeDependentResources();
    }

    d2dContext->BeginDraw();
    d2dContext->Clear(D2D1::ColorF(0.1f, 0.1f, 0.4f));
    d2dContext->SetTransform(
        Matrix3x2F::Translation(point.x, point.y) *
        Matrix3x2F::Scale(zoom, zoom) *
        m_deviceResources->GetOrientationTransform2D()
        );
    d2dContext->FillRectangle(&m_pageRect, m_backgroundBrush.Get());

    D2D1_POINT_2F layoutOrigin;
    layoutOrigin.x = m_textRect.left;
    layoutOrigin.y = m_textRect.top;

    D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;
    if (colorGlyphs)
    {
        options |= D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT;
    }

    for (int i = 0; i < SampleConstants::MaxTextBlocks; i++)
    {
        d2dContext->DrawTextLayout(
            layoutOrigin,
            m_layout[i].Get(),
            i == 1 || i == 4 ? m_textBrush.Get() : m_actionBrush.Get(),
            options
            );
        layoutOrigin.y += m_metrics[i].height;
    }

    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    HRESULT hr = d2dContext->EndDraw();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        DX::ThrowIfFailed(hr);
    }
}
