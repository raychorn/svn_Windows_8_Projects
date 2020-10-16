﻿//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    
    // System Media Trasport Controls
    var smtc;

    // HTML5 audio element. (video element would also work)
    var player;

    // contains list of media files selected by the user.
    var playlist = [];

    // keeps track of current item playing.
    var currentItemIndex;

    var page = WinJS.UI.Pages.define("/html/scenario1.html", {
        ready: function (element, options) {
            smtc = Windows.Media.SystemMediaTransportControls.getForCurrentView();
            player = document.getElementById("player");
            document.getElementById("pickerButton").addEventListener("click", getFilesWithPicker, false);

            // Add event listeners for the audio player.
            player.addEventListener("ended", itemEnded, false);
            player.addEventListener("playing", itemPlaying, false);
            player.addEventListener("pause", itemPaused, false);

            // Register for button pressed events from System Media Transport Controls. 
            // Required for listening to System Media Trasport Controls UI and media keys on keyboards and external devices.
            smtc.addEventListener("buttonpressed", smtcButtonPressed, false);
        }
    });

    // supported audio and video formats for Windows Store apps:
    // http://msdn.microsoft.com/en-us/library/windows/apps/hh986969.aspx
    var supportedAudioFormats = [
        ".3g2", ".3gp2", ".3gp", ".3gpp", ".m4a", ".asf", ".wma", ".aac", ".adt", ".adts", ".mp3", ".wav", ".ac3", ".ec3",
    ];

    var supportedVideoFormats = [
        ".3g2", ".3gp2", ".3gp", ".3gpp", ".m4v", ".mp4v", ".mp4", ".mov", ".m2ts", ".asf", ".wmv", ".avi",
    ];

    // Opens file picker for user to select one or more files to our playlist.
    // After selection, media player will automatically start playing from first file.
    function getFilesWithPicker() {
        var filePicker = new Windows.Storage.Pickers.FileOpenPicker();
        filePicker.fileTypeFilter.replaceAll(supportedAudioFormats);
        filePicker.suggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.musicLibrary;

        filePicker.pickMultipleFilesAsync().then(function (files) {
            if (files.size === 0) {
                // user canceled the picker
                WinJS.log && WinJS.log("No files selected", "sample", "status");
            } else {
                WinJS.log && WinJS.log(files.size + " file(s) selected", "sample", "status");

                playlist = files;
                currentItemIndex = 0;
                updateItemInfo();
                startPlayer();
            }
        });
    }

    // Linked to player HTML element when Player play button is pressed.
    function itemPlaying() {
        // Let media control know player is playing to keep playback state the same.
        smtc.playbackStatus = Windows.Media.MediaPlaybackStatus.playing;
    }

    // Linked to player HTML element when Player pause button is pressed.
    function itemPaused() {
        // Let media control know player is paused to keep playback state the same.
        smtc.playbackStatus = Windows.Media.MediaPlaybackStatus.paused;
    }

    // Linked to the player HTML element when player ends playback of current item.
    // Automatically moves to next item or cycles to first item in playlist if last item.
    function itemEnded() {
        WinJS.log && WinJS.log("Current item ended, moving to next item", "sample", "status");
        if (currentItemIndex === playlist.length - 1) {
            currentItemIndex = -1;
        }
        nextItem();
    }


    // Method is called when a media key is pressed on a keyboard or
    // System Media Trasport controls UI button is pressed.
    function smtcButtonPressed(ev) {
        var smtcButtons = Windows.Media.SystemMediaTransportControlsButton;
        switch (ev.button) {
            case smtcButtons.play:
                WinJS.log && WinJS.log("Play pressed", "sample", "status");
                startPlayer();
                break;

            case smtcButtons.pause:
                WinJS.log && WinJS.log("Pause pressed", "sample", "status");
                pausePlayer();
                break;

            case smtcButtons.stop:
                WinJS.log && WinJS.log("Stop pressed", "sample", "status");
                stopPlayer();
                break;

            case smtcButtons.next:
                WinJS.log && WinJS.log("Next pressed", "sample", "status");
                nextItem();
                break;

            case smtcButtons.previous:
                WinJS.log && WinJS.log("Previous pressed", "sample", "status");
                previousItem();
                break;

                // Insert additional case statements for other buttons you want to handle in your app.
                // Remember that you also need to first enable those buttons via the corresponding
                // IsXXXXEnabled property on the SystemMediaTransportControls object.
        }
    }

    function startPlayer() {
        if (player.paused) {
            player.play();
        }
    }

    function pausePlayer() {
        if (!player.paused) {
            player.pause();
        }
    }

    function stopPlayer() {
        if (!player.paused) {
            player.pause();
        }
        player.currentTime = 0;
        smtc.playbackStatus = Windows.Media.MediaPlaybackStatus.stopped;
    }

    function nextItem() {
        currentItemIndex++;
        updateItemInfo();
    }

    function previousItem() {
        currentItemIndex--;
        updateItemInfo();
    }

    function getMediaPlaybackType(storageFile) {
        var mediaPlaybackType = Windows.Media.MediaPlaybackType.unknown;
        var fileMimeType = storageFile.contentType.toLowerCase();

        if (fileMimeType.indexOf("audio/") === 0) {
            mediaPlaybackType = Windows.Media.MediaPlaybackType.music;
        }
        else if (fileMimeType.indexOf("video/") === 0) {
            mediaPlaybackType = Windows.Media.MediaPlaybackType.video;
        }
        else if (fileMimeType.indexOf("image/") === 0) {
            mediaPlaybackType = Windows.Media.MediaPlaybackType.image;
        }

        return mediaPlaybackType;
    }

    // Function is invoked when the current item is changed by the user in any manner.
    function updateItemInfo() {
        // update media source to new item
        var uri = URL.createObjectURL(playlist[currentItemIndex]);
        player.setAttribute("src", uri, false);

        // Enables corresponding buttons in System Media Transport Controls UI and
        // corresponding media keys on keyboard
        smtc.isPlayEnabled = true;
        smtc.isPauseEnabled = true;
        smtc.isStopEnabled = true;

        // Logic to enable and disable the previous and next buttons 
        // depending on current item in playlist.
        if (currentItemIndex === 0) {
            smtc.isPreviousEnabled = false;
        }
        else {
            smtc.isPreviousEnabled = true;
        }

        if (currentItemIndex === playlist.length - 1) {
            smtc.isNextEnabled = false;
        }
        else {
            smtc.isNextEnabled = true;
        }

        // media player will stop playing when the source is updated.
        // If System Media Trasport Controls was reporting playing, restart the player.
        if (smtc.playbackStatus === Windows.Media.MediaPlaybackStatus.playing) {
            startPlayer();
        }

        // updates what is displayed in the System Media Trasport Controls UI with
        // metadata information from the file
        smtc.displayUpdater.copyFromFileAsync(getMediaPlaybackType(playlist[currentItemIndex]), playlist[currentItemIndex]).done(
            function (ev) {
                smtc.displayUpdater.update();
            });
    }

})();
