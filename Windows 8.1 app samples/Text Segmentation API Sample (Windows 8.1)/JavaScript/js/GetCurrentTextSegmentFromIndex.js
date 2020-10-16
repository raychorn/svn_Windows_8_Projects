//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/GetCurrentTextSegmentFromIndex.html", {
        ready: function (element, options) {
            document.getElementById("WordSegmentButton").addEventListener("click", showWordSegmentFromIndex, false);
            document.getElementById("SelectionSegmentButton").addEventListener("click", showSelectionSegmentFromIndex, false);
        }
    });

    function showWordSegmentFromIndex() {

        // Initialize and obtain input values
        var notifyText = "";

        // Obtain the input string value, check for non-emptiness
        var inputStringText = inputStringBox.value;
        if ("" === inputStringText) {
            notifyText = "Cannot compute word segments: input string is empty.";
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }

        // Obtain the language tag value, check for non-emptiness and that it is valid
        // Ex. Valid Values:
        //     "en-US" (English (United States))
        //     "fr-FR" (French (France))
        //     "de-DE" (German (Germany))
        //     "ja-JP" (Japanese (Japan))
        //     "ar-SA" (Arabic (Saudi Arabia))
        //     "zh-CN" (China (PRC))
        var languageTagText = languageTagBox.value;
        if ("" === languageTagText) {
            notifyText += "\nlanguage tag input is empty ... using generic-language segmentation rules.";
            languageTagText = "und";
        }
        else {
            if (!Windows.Globalization.Language.isWellFormed(languageTagText)) {
                notifyText = "Language tag is not well formed: \"" + languageTagText + "\"";
                WinJS.log && WinJS.log(notifyText, "sample", "error");
                return;
            }
        }

        // Obtain the text index
        //   : If empty, assume the default ( = 0), returns the first segment
        //   : If not an actual number, throw an error
        //   : If Negative, or beyond the length of the input string, throw an error
        var textIndex = textIndexBox.value;
        if ("" === textIndex) {
            textIndex = 0;
            notifyText += "\nNo input index provided ... using first segment reference (index = 0) as default.";
        }
        else if (isNaN(parseFloat(textIndex)) || !isFinite(textIndex)) {
            notifyText = "Index value must be a valid number";
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }
        else if (textIndex < 0) {
            notifyText = "Cannot compute word segment for negative index value: " + textIndex;
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }
        else if (textIndex >= inputStringText.length) {
            notifyText = "Cannot compute word segment for out of bounds index value: " + textIndex;
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }

        // Notify that we are going to calculate selection segment
        notifyText += "\nFinding selection segment for the given index ...\n";
        notifyText += "\nInput: \"" + inputStringText + "\"";
        notifyText += "\nLanguage Tag: \"" + languageTagText + "\"";
        notifyText += "\nIndex: " + textIndex;

        // Construct the WordsSegmenter instance
        var segmenter = new Windows.Data.Text.WordsSegmenter(languageTagText);

        // Obtain the token segment
        var tokenSegment = segmenter.getTokenAt(inputStringText, textIndex);
        notifyText += "\nToken segment: \"" + tokenSegment.text + "\"";

        // send notifyText to the output pane
        WinJS.log && WinJS.log(notifyText, "sample", "status");
    }

    function showSelectionSegmentFromIndex() {

        // Initialize and obtain input values
        var notifyText = "";

        // Obtain the input string value, check for non-emptiness
        var inputStringText = inputStringBox.value;
        if ("" === inputStringText) {
            notifyText = "Cannot compute selection segments: input string is empty.";
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }

        // Obtain the language tag value, check for non-emptiness and that it is valid
        // Ex. Valid Values:
        //     "en-US" (English (United States))
        //     "fr-FR" (French (France))
        //     "de-DE" (German (Germany))
        //     "ja-JP" (Japanese (Japan))
        //     "ar-SA" (Arabic (Saudi Arabia))
        //     "zh-CN" (China (PRC))
        var languageTagText = languageTagBox.value;
        if ("" === languageTagText) {
            notifyText += "\nlanguage tag input is empty ... using generic-language segmentation rules.";
            languageTagText = "und";
        }
        else {
            if (!Windows.Globalization.Language.isWellFormed(languageTagText)) {
                notifyText = "Language tag is not well formed: \"" + languageTagText + "\"";
                WinJS.log && WinJS.log(notifyText, "sample", "error");
                return;
            }
        }

        // Obtain the text index
        //   : If empty, assume the default ( = 0), returns the first segment
        //   : If not an actual number, throw an error
        //   : If Negative, or beyond the length of the input string, throw an error
        var textIndex = textIndexBox.value;
        if ("" === textIndex) {
            textIndex = 0;
            notifyText += "\nNo input index provided ... using first segment reference (index = 0) as default.";
        }
        else if (isNaN(parseFloat(textIndex)) || !isFinite(textIndex)) {
            notifyText = "Index value must be a valid number";
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }
        else if (textIndex < 0) {
            notifyText = "Cannot compute selection segment for negative index value: " + textIndex;
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }
        else if (textIndex >= inputStringText.length) {
            notifyText = "Cannot compute selection segment for out of bounds index value: " + textIndex;
            WinJS.log && WinJS.log(notifyText, "sample", "error");
            return;
        }

        // Notify that we are going to calculate selection segment
        notifyText += "\nFinding selection segment for the given index ...\n";
        notifyText += "\nInput: \"" + inputStringText + "\"";
        notifyText += "\nLanguage Tag: \"" + languageTagText + "\"";
        notifyText += "\nIndex: " + textIndex;

        // Construct the SelectableWordsSegmenter instance
        var segmenter = new Windows.Data.Text.SelectableWordsSegmenter(languageTagText);

        // Obtain the token segment
        var tokenSegment = segmenter.getTokenAt(inputStringText, textIndex);
        notifyText += "\nToken segment: \"" + tokenSegment.text + "\"";

        // send notifyText to the output pane
        WinJS.log && WinJS.log(notifyText, "sample", "status");
    }
})();
