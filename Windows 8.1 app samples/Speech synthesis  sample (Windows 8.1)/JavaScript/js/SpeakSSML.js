﻿//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/SpeakSSML.html", {
        ready: function (element, options) {
            var speakBtn = document.getElementById("Speak"); //initialization for speak button 
            var saveToFileBtn = document.getElementById("SaveAudio"); //initialization for save to file button
            var VoicesSelect = document.getElementById("lstSelectVoices"); //initialization for dropdown containing all voices installed
            speakBtn.addEventListener("click", SpeakFn, false); //speak button hit
            saveToFileBtn.addEventListener("click", SaveToFileFn, false); //save to file button hit
            VoicesSelect.addEventListener("click", SetVoiceFunction, false); //voice selected via voices drop down
            listbox_GetVoices();  //initialize dropdown to all voices installed in the system          
        }
    });
    var synth = new Windows.Media.SpeechSynthesis.SpeechSynthesizer(); //creating a speech synthesizer object

    function SpeakFn() {
        document.getElementById("Speak").disabled = true;
        var statusDiv = document.getElementById("SSMLStatus"); //Audio play status indicator
        var txtData = document.getElementById("Data");
        statusDiv.style.color = "green";

        var audio = new Audio(); //creating an audio object
        var start = Date.now();

        //synthesize a stream from ssml input. This API converts SSML to a media stream
        synth.synthesizeSsmlToStreamAsync(txtData.value).then(function (markersStream) {


            var stop = Date.now();
            var elapsed = stop - start;
            statusDiv.innerText = "Finished Speaking";

            var blob = MSApp.createBlobFromRandomAccessStream(markersStream.ContentType, markersStream);
            audio.src = URL.createObjectURL(blob, { oneTimeOnly: true });
            markersStream.seek(0); //start at beginning when speak is hit
            audio.AutoPlay = Boolean(true);

            //play audio
            audio.onplay = function () {
                statusDiv.innerText = "Playing";
            };

            //audio on completed
            audio.onended = function () {
                statusDiv.innerText = "Completed";
                document.getElementById("Speak").disabled = false;
            };

            audio.play();

        },
        function OnError(error) {
            statusDiv.innerText = "Failed";
            statusDiv.style.color = "red";
        });
    }

    //implement save to file functionality
    function SaveToFileFn() {

        var savePicker = new Windows.Storage.Pickers.FileSavePicker();
        var txtData = document.getElementById("Data");

        savePicker.defaultFileExtension = ".wav";
        savePicker.fileTypeChoices.insert("Audio file", [".wav"]);

        savePicker.pickSaveFileAsync().then(function (file) {
            if (file !== null) {
                synth.synthesizeSsmlToStreamAsync(txtData.value).then(function (markerStream) {

                    // open the output stream                    
                    var buffer = new Windows.Storage.Streams.Buffer(markerStream.size);
                    file.openAsync(Windows.Storage.FileAccessMode.readWrite).then(function (writeStream) {
                        var outputStream = writeStream.getOutputStreamAt(writeStream.size);
                        var dataWriter = new Windows.Storage.Streams.DataWriter(outputStream);

                        markerStream.readAsync(buffer, markerStream.size, Windows.Storage.Streams.InputStreamOptions.none).then(function () {
                            dataWriter.writeBuffer(buffer);
                            // close the data file streams
                            dataWriter.storeAsync().then(function () {
                                outputStream.flushAsync().then(function () {
                                });
                            });
                        });
                    });
                });
            }
        });

    }

    function listbox_GetVoices() {
        var VoicesSelect = document.getElementById("lstSelectVoices");

        // get the list of all of the voices installed on this machine
        var allVoices = Windows.Media.SpeechSynthesis.SpeechSynthesizer.allVoices;

        var defaultVoice = Windows.Media.SpeechSynthesis.SpeechSynthesizer.defaultVoice;

        // go through this list
        for (var voiceIndex = 0; voiceIndex < allVoices.size; voiceIndex++) {

            var currVoice = allVoices[voiceIndex];
            var option = document.createElement("option");
            // add this voice to the voice select drop down
            option.text = currVoice.displayName;
            VoicesSelect.add(option, null);

            if (currVoice.id === defaultVoice.id) {
                VoicesSelect.selectedIndex = voiceIndex;
            }
        }

        // update the SSML to use the current language
        updateSSMLText();
    }

    function SetVoiceFunction() {
        var voicesSelect = document.getElementById("lstSelectVoices");
        if (voicesSelect.selectedIndex !== -1) {

            // get all voices
            var allVoices = Windows.Media.SpeechSynthesis.SpeechSynthesizer.allVoices;

            // use the same index to find the voice that the  user selected
            var selectedVoice = allVoices[voicesSelect.selectedIndex];

            // and use that voice, to be set
            synth.voice = selectedVoice;

            // update the SSML to use the current language
            updateSSMLText();
        }
    }

    function updateSSMLText() {

        try {
            var txtData = document.getElementById("Data");
            var text = txtData.value;

            var language = synth.voice.language;
            var xmlDoc = new Windows.Data.Xml.Dom.XmlDocument();

            xmlDoc.async = "false";
            xmlDoc.loadXml(text);

            var langAttribute = xmlDoc.documentElement.getAttributeNode("xml:lang");
            langAttribute.value = language;

            txtData.value = xmlDoc.getXml();
        }
        catch (e) {
            // don't need to do anything. this could be unparseable xml due to a 
            // edit in progress, just can't update the SSML right now.
        }
    }

})();


