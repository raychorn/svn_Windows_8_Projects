//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    WinJS.UI.Pages.define("/pages/soundrecorder/soundrecorder.html", {

        processed: function (element, options) {
            this.soundRecorder = new Application.Capture.SoundRecorder();

            element.querySelector(".controls .record").onclick = this.toggleRecording;
            element.querySelector(".controls .play").onclick = this.togglePlayback;
            element.querySelector(".controls .save").onclick = this.save;

            // Bind the UI to the recorder and initialize the device
            return WinJS.Binding.processAll(element, this.soundRecorder);
        },

        ready: function (element, options) {
            this.soundRecorder.initialize(element.querySelector(".controls audio"));
        },

        // Start or stop recording
        toggleRecording: function () {
            if (Application.navigator.pageControl.soundRecorder.state === Application.Capture.SoundRecorderStates.recording) {
                Application.navigator.pageControl.soundRecorder.stop();
            } else {
                Application.navigator.pageControl.soundRecorder.start();
            }
        },

        // Toggle playback of the captured audio 
        togglePlayback: function () {
            if (Application.navigator.pageControl.soundRecorder.state === Application.Capture.SoundRecorderStates.playing) {
                Application.navigator.pageControl.soundRecorder.pause();
            } else {
                Application.navigator.pageControl.soundRecorder.play();
            }
        },

        // Save the captured audio
        save: function () {
            Application.navigator.pageControl.soundRecorder.saveAsync().done(function () {
                (new Application.Data.Folder({ folder: Windows.Storage.KnownFolders.musicLibrary })).invoke();
            });
        },

        // Dispose the sound recorder when unloading
        unload: function () {
            this.soundRecorder.dispose();
        },

        soundRecorder: null

    });
})();
