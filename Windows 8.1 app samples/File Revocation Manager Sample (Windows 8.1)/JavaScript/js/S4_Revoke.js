//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/S4_Revoke.html", {
        ready: function (element, options) {
            document.getElementById("Revoke").addEventListener("click", doRevoke, false);
            SdkSample.validateFileExistence();
        }
    });

    function doRevoke() {
        if ("" === EnterpiseID.value)
        {
            WinJS.log && WinJS.log("Please enter an Enterpise ID that you want to use.", "sample", "error");
            return;
        }

        try{
            Windows.Security.EnterpriseData.FileRevocationManager.revoke(EnterpiseID.value);
            WinJS.log && WinJS.log("The Enterprise ID " + EnterpiseID.value + " was revoked. The files protected by it will not be accessible anymore.", "sample", "status");
        }
        catch ( e ){
            WinJS.log && WinJS.log("Revoke failed exception " + err, "sample", "error");
        }
    }
})();
