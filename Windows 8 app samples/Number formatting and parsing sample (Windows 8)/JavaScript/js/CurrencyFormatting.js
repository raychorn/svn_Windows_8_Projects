﻿//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/CurrencyFormatting.html", {
        ready: function (element, options) {
            document.getElementById("displayButton").addEventListener("click", doDisplay, false);
        }
    });

    function doDisplay() {
        // This scenario uses the Windows.Globalization.NumberFormatting.CurrencyFormatter class
        // to format a number as a currency.

        // Determine the current user's default currency.
        var userCurrency = Windows.System.UserProfile.GlobalizationPreferences.currencies;

        // Generate numbers used for formatting.
        var wholeNumber = 12345;
        var fractionalNumber = 12345.67;

        // Create formatter initialized using the current user's preference settings for number formatting.
        var userCurrencyFormat = new Windows.Globalization.NumberFormatting.CurrencyFormatter(userCurrency);
        var currencyDefault = userCurrencyFormat.format(fractionalNumber);

        // Create a formatter initialized to a specific currency, in this case it's the US Dollar, but with the default number formatting for the current user.
        var currencyFormatUSD = new Windows.Globalization.NumberFormatting.CurrencyFormatter("USD");  // Specified as an ISO 4217 code.
        var currencyUSD = currencyFormatUSD.format(fractionalNumber);

        // Create a formatter initialized to a specific currency, in this case it's the Euro with the default number formatting for France.
        var currencyFormatEuroFR = new Windows.Globalization.NumberFormatting.CurrencyFormatter("EUR", ["fr-FR"], "ZZ");
        var currencyEuroFR = currencyFormatEuroFR.format(fractionalNumber);

        // Create a formatter initialized to a specific currency, in this case it's the Euro with the default number formatting for Ireland.
        var currencyFormatEuroIE = new Windows.Globalization.NumberFormatting.CurrencyFormatter("EUR", ["gd-IE"], "IE");
        var currencyEuroIE = currencyFormatEuroIE.format(fractionalNumber);

        // Formatted so that fraction digits are always included.
        var currencyFormatUSD1 = new Windows.Globalization.NumberFormatting.CurrencyFormatter("USD");
        currencyFormatUSD1.fractionDigits = 2;
        var currencyUSD1 = currencyFormatUSD1.format(wholeNumber);

        // Formatted so that integer grouping separators are included.
        var currencyFormatUSD2 = new Windows.Globalization.NumberFormatting.CurrencyFormatter("USD");
        currencyFormatUSD2.isGrouped = 1;
        var currencyUSD2 = currencyFormatUSD2.format(fractionalNumber);

        // Display the results.
        var results = "Fixed number (" + fractionalNumber + ")\n" +
                      "With user's default currency: " + currencyDefault + "\n" +
                      "Formatted US Dollar: " + currencyUSD + "\n" +
                      "Formatted Euro (fr-FR defaults): " + currencyEuroFR + "\n" +
                      "Formatted Euro (gd-IE defaults): " + currencyEuroIE + "\n" +
                      "Formatted US Dollar (with fractional digits): " + currencyUSD1 + "\n" +
                      "Formatted US Dollar (with grouping separators): " + currencyUSD2;

        WinJS.log && WinJS.log(results, "sample", "status");
    }
})();
