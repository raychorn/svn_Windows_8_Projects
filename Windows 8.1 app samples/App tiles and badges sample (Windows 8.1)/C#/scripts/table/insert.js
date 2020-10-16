//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

// This is an example script you might use in your Windows Azure Mobile Service
// to receive channels from the client.
// The wizard will automatically add such a script to your service. 
//
// See documentation at http://go.microsoft.com/fwlink/?LinkId=290986&clcid=0x409
function insert(item, user, request) {

    // The following call is for illustration purpose only
    // The call and function body should be moved to a script in your app
    // where you want to send a notification
    sendNotifications(item.channelUri);

    // The following code manages channels and should be retained in this script
    var ct = tables.getTable("channels");
    ct.where({ installationId: item.installationId }).read({
        success: function (results) {
            if (results.length > 0) {
                // we already have a record for this user/installation id - if the 
                // channel is different, update it otherwise just respond
                var existingItem = results[0];
                if (existingItem.channelUri !== item.channelUri) {
                    existingItem.channelUri = item.channelUri;
                    ct.update(existingItem, {
                        success: function () {
                            request.respond(200, existingItem);
                        }
                    });
                } else {
                    // no change necessary, just respond
                    request.respond(200, existingItem);
                }
            } else {
                // no matching installation, insert the record
                request.execute();
            }
        }
    })

    // The following code should be moved to appropriate script in your app where notification is sent
    function sendNotifications(uri) {
        console.log("Uri: ", uri);
        push.wns.sendToastText01(uri, {
            text1: "Sample toast from sample insert"
        }, {
            success: function (pushResponse) {
                console.log("Sent push:", pushResponse);
            }
        });
    }
}