//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/S1-CreateTPMVSC.html",
    {
        ready: function (element, options) {
            document.getElementById("Create").addEventListener(
                 "click",
                  Create_Click,
                  false);
        }
    });

    var pinPolicyDisallowed = "Disallowed";
    var pinPolicyAllowed = "Allowed";
    var pinPolicyRequireOne = "Require At Least One";

    function Create_Click() {

        var adminKey = Windows.Security.Cryptography.CryptographicBuffer
                       .generateRandom(SdkSample.ADMIN_KEY_LENGTH_IN_BYTES);

        var button = document.getElementById("Create");
        button.disabled = true;

        var pinPolicy = null;
        try {
            pinPolicy = parsePinPolicy();
        }
        catch (error) {
            WinJS.log("Failed to parse PIN policy due to exception: " +
                      error.toString(),
                      "sample",
                      "error");
            button.disabled = false;
            return;
        }

        WinJS.log("Creating TPM virtual smart card...", "sample", "status");

        Windows.Devices.SmartCards.SmartCardProvisioning
        .requestVirtualSmartCardCreationAsync(
            document.getElementById("FriendlyNameText").value,
            adminKey,
            pinPolicy).then(
        function (smartCardProvisioning) {

            // If the user select Cancel on the card creation prompt, null
            // will be returned. In this case, we need to cancel the following
            // asynchronous calls.
            if (null === smartCardProvisioning) {

                // To cancel an async chain in Javascript, we can return the
                // static, pre-canceled promise WinJS.Promise.cancel.
                return WinJS.Promise.cancel;
            }

            // The following two lines are not directly related to TPM virtual
            // smart card creation, but are used to demonstrate how to handle
            // CardAdded events by registering an event handler with a
            // SmartCardReader object.  Since we are using a TPM virtual smart
            // card in this case, the card cannot actually be added to or
            // removed from the reader, but a CardAdded event will fire as
            // soon as the event handler is added, since the card is already
            // inserted.
            //
            // To prevent it from being garbage collected, we must retain
            // a reference to the SmartCardReader object to
            // which we are adding the event handler.  Hence we assign the
            // reader object associated with the SmartCardProvisioning we
            // received from RequestVirtualSmartCardCreationAsync to
            // SdkSample.reader.  This will also be used to access the
            // reader and associated card object in the following
            // scenarios.
            SdkSample.reader =
                smartCardProvisioning.smartCard.reader;
            SdkSample.reader.oncardadded = handleCardAdded;

            // Store the admin key as well so that we can use it to it for
            // challenge response authentication in the following scenarios
            SdkSample.adminKey = adminKey;

            // We now have a TPM virtual smart card which is provisioned and
            // ready to use; therefore, the following steps are not strictly
            // necessary.  However, should you want to re-provision the card
            // in the future, you would first get a challenge context from
            // the provisioning object and use that to re-provision the card.
            return smartCardProvisioning.getChallengeContextAsync();
        }).then(
        function (smartCardChallengeContext) {
            var response = SdkSample.calculateChallengeResponse(
                smartCardChallengeContext.challenge,
                SdkSample.adminKey);
            return smartCardChallengeContext.provisionAsync(response, true);
        }).done(
        function () {
            WinJS.log(
                "TPM virtual smart card is provisioned and ready for use.",
                "sample",
                "status");
            button.disabled = false;
        },
        function (error) {
            if ("Canceled" === error.name) {
                WinJS.log && WinJS.log(
                    "TPM virtual smart card creation was canceled by the user.",
                    "sample",
                    "status");
            } else {
                WinJS.log && WinJS.log(
                    "TPM virtual smart card creation failed with exception: " +
                    error.toString(),
                    "sample",
                    "error");
            }
            button.disabled = false;
        });
    }

    function handleCardAdded(eventArgs) {
        // eventArgs has two properties: smartCard, the card added to the
        // reader, and target, the reader it was added to.
        WinJS.log("Card added to reader " + eventArgs.target.name + ".",
                  "sample",
                  "status");
    }

    function parsePinPolicy() {
        var pinPolicy = new Windows.Devices.SmartCards.SmartCardPinPolicy();

        // If parseInt fails, it will return NaN, which will cause an
        // exception to be thrown when the minLength or maxLength is
        // assigned.  That exception will be caught by the caller to
        // notify the user of the error.
        pinPolicy.minLength =
            parseInt(document.getElementById("PinMinLength").value);
        pinPolicy.maxLength =
            parseInt(document.getElementById("PinMaxLength").value);

        switch (document.getElementById("PinUppercase").value) {
            case pinPolicyDisallowed:
                pinPolicy.uppercaseLetters = Windows.Devices.SmartCards
                                            .SmartCardPinCharacterPolicyOption
                                            .disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.uppercaseLetters = Windows.Devices.SmartCards
                                             .SmartCardPinCharacterPolicyOption
                                             .allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.uppercaseLetters = Windows.Devices.SmartCards
                                             .SmartCardPinCharacterPolicyOption
                                             .requireAtLeastOne;
                break;
            default:
                throw new Error("Uppercase letters PIN policy option " +
                                "is invalid.");
        }

        switch (document.getElementById("PinLowercase").value) {
            case pinPolicyDisallowed:
                pinPolicy.lowercaseLetters = Windows.Devices.SmartCards
                                            .SmartCardPinCharacterPolicyOption
                                            .disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.lowercaseLetters = Windows.Devices.SmartCards
                                             .SmartCardPinCharacterPolicyOption
                                             .allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.lowercaseLetters = Windows.Devices.SmartCards
                                             .SmartCardPinCharacterPolicyOption
                                             .requireAtLeastOne;
                break;
            default:
                throw new Error("Lowercase letters PIN policy option " +
                                "is invalid.");
        }

        switch (document.getElementById("PinDigits").value) {
            case pinPolicyDisallowed:
                pinPolicy.digits = Windows.Devices.SmartCards
                                   .SmartCardPinCharacterPolicyOption
                                   .disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.digits = Windows.Devices.SmartCards
                                   .SmartCardPinCharacterPolicyOption
                                   .allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.digits = Windows.Devices.SmartCards
                                   .SmartCardPinCharacterPolicyOption
                                   .requireAtLeastOne;
                break;
            default:
                throw new Error("Digits PIN policy option is invalid.");
        }

        switch (document.getElementById("PinSpecial").value) {
            case pinPolicyDisallowed:
                pinPolicy.specialCharacters = Windows.Devices.SmartCards
                                              .SmartCardPinCharacterPolicyOption
                                              .disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.specialCharacters = Windows.Devices.SmartCards
                                              .SmartCardPinCharacterPolicyOption
                                              .allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.specialCharacters = Windows.Devices.SmartCards
                                              .SmartCardPinCharacterPolicyOption
                                              .requireAtLeastOne;
                break;
            default:
                throw new Error("Special characters PIN policy option " +
                                "is invalid.");
        }

        return pinPolicy;
    }

})();
