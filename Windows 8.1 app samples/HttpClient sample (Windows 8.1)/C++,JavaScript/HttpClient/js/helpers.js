﻿var Helpers = {
    tryGetUri: function (uriString) {
        // Create a Uri instance and catch exceptions related to invalid input. This method returns 'true'
        // if the Uri instance was successfully created and 'false' otherwise.
        try {
            return new Windows.Foundation.Uri(uriString.trim());
        }
        catch (ex) {
            WinJS.log && WinJS.log("Error: Invalid URI", "sample", "error");
        }

        return null;
    },

    displayTextResultAsync: function (response, output) {
        output.value += this.serializeHeaders(response);
        return response.content.readAsStringAsync().then(function (responseBodyAsText) {
            // Insert new lines.
            responseBodyAsText = responseBodyAsText.replace(/<br>/g, "\r\n");
            output.value += responseBodyAsText;
            return response;
        });
    },

    serializeHeaders: function (response) {
        var output = "";

        output += response.statusCode + " " + response.reasonPhrase + "\r\n";

        output += this.serializeHeaderCollection(response.headers);
        output += this.serializeHeaderCollection(response.content.headers);
        output += "\r\n";

        return output;
    },

    serializeHeaderCollection: function (headers) {
        var output = "";
        for (var iterator = headers.first() ; iterator.hasCurrent; iterator.moveNext()) {
            output += iterator.current.key + ": " + iterator.current.value + "\r\n";
        }
        return output;
    },

    scenarioStarted: function () {
        document.getElementById("startButton").disabled = true;
        document.getElementById("cancelButton").disabled = false;
        var outputField = document.getElementById("outputField");
        if (outputField) {
            outputField.value = "";
        }
    },

    scenarioCompleted: function () {
        document.getElementById("startButton").disabled = false;
        document.getElementById("cancelButton").disabled = true;
    },

    onError: function (error) {
        if (error.name === "Canceled") {
            WinJS.log && WinJS.log("Request canceled.", "sample", "error");
        } else {
            WinJS.log && WinJS.log(error, "sample", "error");
        }
        Helpers.scenarioCompleted();
    },

    replaceQueryString: function (newQueryString) {
        var addressField = document.getElementById("addressField");
        var resourceAddress = addressField.value;

        // Remove previous query string.
        var questionMarkIndex = resourceAddress.indexOf("?");
        if (questionMarkIndex !== -1) {
            resourceAddress = resourceAddress.substring(0, questionMarkIndex);
        }

        addressField.value = resourceAddress + newQueryString;
    },

    getEnumValueName: function (sourceEnum, value) {
        var valueName = "";
        for (var sourceName in sourceEnum) {
            if (sourceEnum[sourceName] === value) {
                valueName = sourceName;
            }
        }
        return valueName;
    },

    meteredConnectionFilter: null
};
