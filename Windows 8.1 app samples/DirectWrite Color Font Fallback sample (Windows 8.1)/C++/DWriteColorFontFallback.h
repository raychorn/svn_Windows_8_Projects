//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

namespace DWriteColorFontFallback
{
    // Constants used in the Sample.
    namespace SampleConstants
    {
        static const float UIMaxZoom                = 9.0f;
        static const float UIMinZoom                = 1.0f;

        static const float TextMargin               = 10.0f;
        static const float TitleFontSizeDIPS        = 28.0f;
        static const float SubTitleFontSizeDIPS     = 14.0f;
        static const float BodyFontSizeDIPS         = 20.0f;

        static const WCHAR* TitleFontFamilyName     = L"Segoe UI Light";
        static const WCHAR* SubTitleFontFamilyName  = L"Segoe UI Light";
        static const WCHAR* BodyFontFamilyName      = L"Segoe UI";
        static const WCHAR* LocaleName              = L"en-us";

        static const WCHAR* EmojiFontFamilyName     = L"Segoe UI Emoji";
        static const WCHAR* SymbolFontFamilyName    = L"Segoe UI Symbol";

        static const char16* TextStrings[] = {
            L"Segoe UI with standard System Font Fallback",
            L"This is a sentence of Latin text.\nHere are a set of symbols in the Emoji range '🌱🌲🌳🌴🌷🌹🌻🌽🌾🍅🍆🍇🍟🍠🍡🍢🍣🍤🍥🍦🍧🍨🍩🍪🍫🎂🎃🎄🎅🎆🎇🎉🎊🎋🎌🎍🎎'.\nHere are a set of symbols outside the Emoji range '➊➊➋➌➍➎➏➐➑➒'.\nNow for some Katakana letters 'カガキギクグケゲコゴサザ'.\n",
            L"Segoe UI with custom Font Fallback",
            L"",
            L"This is a sentence of Latin text.\nHere are a set of symbols in the Emoji range '🌱🌲🌳🌴🌷🌹🌻🌽🌾🍅🍆🍇'.\nHere are a set of symbols outside the Emoji range '➊➊➋➌➍➎➏➐➑➒'.\nNow for some Katakana letters 'カガキギクグケゲコゴサザ'.\n",
            L"\nLots of color Emoji: 🌂🌃🌄🌅🌆🌇🌈🌉🌊🌋🌍🌎🌏🌒🌓🌔🌕🌖🌗🌘🌝🌞🌱🌲🌳🌴🌷🌹🌻🌽🌾🍅🍆🍇🍈🍉🍌🍍🍎🍏🍐🍑🍒🍓🍔🍕🍖🍗🍘🍛🍜🍝🍟🍠🍡🍢🍣🍤🍥🍦🍧🍨🍩🍪🍫🍬🍭🍮🍯🍰🍱🍲🍳🍵🍹🍺🍻🍼🎂🎃🎄🎅🎆🎇🎉🎊🎋🎌🎍🎎🎐🎑🎓🎠🎡🎢🎣🎤🎦🎨🎯🎰🎱🎳🎴🎻🎼🎾🎿🏀🏁🏂🏄🏇🏊🏡🏤🏦🏧🏩🏫🏬🐌🐓🐝🐠🐡🐢🐣🐳🐵🐶🐸🐹👆👇👈👉👊👒👔👛👝👦👧👨👩👮👯👰👱👲👳👴👵👶👷👸👹👺👼👾💂💄💅💆💇💈💉💊💋💌💐💑💒💘💝💟💨💩💱💹💺💾📈📉📊📌📍📑📓📔📛📝📟📣📵🔞🔫😁😂😃😄😅😆😇😈😉😊😋😌😍😎😏😐😒😓😔😖😘😚😜😝😞😠😡😢😣😤😥😨😩😪😫😭😰😱😲😳😵😶😷🙅🙆🙇🙈🙉🙊🙋🙌🙍🙎🙏🚀🚃🚄🚅🚆🚈🚉🚊🚋🚌🚍🚎🚏🚐",
        };

        static const unsigned int MaxTextBlocks     = 5;
        static const float TopMargin                = 150.0f;
        static const float BottomMargin             = 50.0f;
        static const float LeftMargin               = 50.0f;
        static const float RightMargin              = 50.0f;

        static const unsigned int FontFallbackSystem                = 0;
        static const unsigned int FontFallbackEmoji                 = 1;
        static const unsigned int FontFallbackEmojiSystem           = 2;
        static const unsigned int FontFallbackEmojiSymbol           = 3;
        static const unsigned int FontFallbackEmojiSymbolSystem     = 4;
        static const unsigned int FontFallbackSymbol                = 5;
        static const unsigned int FontFallbackSymbolSystem          = 6;
        static const unsigned int MaxFontFallbackScenarios          = 7;

        static char16* FontFallbackDesc[] =
        {
            L"System",
            L"Segoe UI Emoji",
            L"Segoe UI Emoji -> System",
            L"Segoe UI Emoji -> Segoe UI Symbol",
            L"Segoe UI Emoji -> Segoe UI Symbol -> System",
            L"Segoe UI Symbol",
            L"Segoe UI Symbol -> System",
        };
    };
};
