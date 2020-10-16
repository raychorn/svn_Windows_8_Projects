//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    var transcoding = Application.Transcoding;
    var storage = Windows.Storage;

    WinJS.UI.Pages.define("/pages/convert/convert.html", {

        processed: function (element, options) {
            element.querySelector(".controls .open").onclick = this.openFile;
            element.querySelector(".controls .save").onclick = this.saveFile;

            return WinJS.Binding.processAll(element, transcoding.transcoder);
        },

        ready: function(element, options){
            transcoding.transcoder.bind("state", this._transcoderStateChanged);
            this._transcoderStateChanged();
        },

        // Opens a media file using a picker and sets it in the transcoder
        openFile: function () {
            var picker = new storage.Pickers.FileOpenPicker();
            picker.fileTypeFilter.replaceAll(["*"]);
            picker.pickSingleFileAsync().done(function (file) {
                if (file) {
                    transcoding.transcoder.source = file;
                }
            });
        },

        // Saves a media file using the transcoder
        saveFile: function () {
            // Get the container value from the proper select element depending on the source type
            var container = Application.navigator.pageElement.querySelector(".controls ." +
                (transcoding.transcoder.state === transcoding.TranscoderStates.videoSource ? "video" : "audio") +
                "containervalue").value;

            // Get the profile value from the proper select element depending on the source type
            var profile = Application.navigator.pageElement.querySelector(".controls ." +
                (transcoding.transcoder.state === transcoding.TranscoderStates.videoSource ? "video" : "audio") +
                "profilevalue").value;
            
            // Use the file save picker to get the output and start transcoding
            var picker = new storage.Pickers.FileSavePicker();
            picker.fileTypeChoices.insert(container, ["." + container.toLowerCase()]);
            picker.pickSaveFileAsync().done(function (file) {
                if (file) {
                    transcoding.transcoder.startTranscodeAsync(file, container, profile);
                }
            });
        },

        // Unload the page
        unload: function () {
            transcoding.transcoder.unbind("state", this._transcoderStateChanged);
        },

        // Disable the output format controls and save button if the transcoder is not ready
        _transcoderStateChanged: function () {
            var disableOutputControls =
                transcoding.transcoder.state === transcoding.TranscoderStates.empty ||
                transcoding.transcoder.state === transcoding.TranscoderStates.unavailable;

            document.querySelector(".convertpage section[role=main] .controls .save").disabled = disableOutputControls;
            Array.prototype.forEach.call(document.querySelectorAll(".convertpage section[role=main] .controls .rows select"), function (select) {
                select.disabled = disableOutputControls;
            });
        }
    });
})();
