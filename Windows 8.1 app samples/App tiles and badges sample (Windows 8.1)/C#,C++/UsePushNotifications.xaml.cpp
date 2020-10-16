//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "UsePushNotifications.xaml.h"

#ifdef BUILD_AZURE_CLIENT_WITHOUT_WIZARD
#include <azuremobile.h>

using namespace azure::mobile;
using namespace concurrency;
using namespace web;
using namespace Windows::Networking::PushNotifications;
using namespace Windows::Security::Cryptography;
using namespace Windows::System::Profile;
#endif

using namespace SDKSample::Tiles;

UsePushNotifications::UsePushNotifications()
{
    InitializeComponent();
}

// The Windows Azure Mobile Service wizard will automatically add the necessary references to
// your app, and set up your service in the cloud. The wizard will also provide a script
// in the cloud that automatically sends an update back to the client whenever a push
// notifications channel is uploaded. The wizard will also add code to your "OnLaunched" handler to 
// automatically request a push notification channel and upload it to your service. Run the wizard, 
// then look at App.xaml.cpp to see the changes.

// You can also set up the Mobile Service without using the wizard using the following steps:
//
// 1. Sign up for a free Azure trial (http://www.windowsazure.com/en-us/pricing/free-trial/) 
// if you do not have an Azure subscription already. This will enable you to host 10 mobile services for free.
// 2. Install Windows Azure command line tools - http://go.microsoft.com/fwlink/?LinkID=275464&clcid=0x409
// 3. Open command line to download the credentials required to talk to Windows Azure. This is a one-time setup for 
// running all subsequent commands to manage Windows Azure Mobile Service:
//     a. Download Windows Azure management credentials.
//         >> azure account download
//         This will open up a web page to login to manage.windowsazure.com site. Once you log in, it will generate and 
//         prompt you to download a publish settings  file for your Azure subscription. Save this file on your machine.
//     b. Import the publishsettings file from this location. This will wire up your command line client to manage all 
//        your Windows Azure services from the command line.
//         >> azure account import [SavedLocation]
// 4. Create a Windows Azure Mobile Service:
//     >> azure mobile create [AzureMobileServiceName] [sqlAdminUsername] [sqlAdminPassword]
// 5. Create a table in the Windows Azure Mobile Service database to store the channels:
//     >> azure mobile table create [AzureMobileServiceName] channels
// 6. In the command line, change directory to the Scripts directory under sample install location and run the following:
//     >> azure mobile script upload [AzureMobileServiceName] table/channels.insert.js
// 7. Get the ApplicationUrl and ApplicationKey for your Mobile service and configure this in your Windows Store app 
// in the App.xaml.cpp:
//     >> azure mobile show [AzureMobileServiceName]
// 8. Register this Windows Store app in the Windows Store by creating a new app here:
// http://go.microsoft.com/fwlink/p/?LinkID=266582&clcid=0x409
// 9. Associate your app with this registered Windows Store app by right-clicking the project and then go to 
// Store -> Associate your app with the Windows Store -> Login and follow the instructions
// 10. Get the client secret and package SID for the registered app from Live Connect Developer center 
// (http://go.microsoft.com/fwlink/p/?LinkId=262039&clcid=0x409 -> Edit Settings -> API Settings)
// and run the following from command line to configure your mobile service 
// to send push notifications:
//     >> azure mobile config set [AzureMobileServiceName] microsoftAccountClientSecret [ClientSecret]
//     >> azure mobile config set [AzureMobileServiceName] microsoftAccountPackageSID [PackageSID]
// 11. Add this code to App.xaml.cpp to automatically request a push notifications channel and upload it to your service:
void UsePushNotifications::OpenAndUploadChannel()
{
    #ifdef BUILD_AZURE_CLIENT_WITHOUT_WIZARD
    typedef task_continuation_context ctx;

    std::shared_ptr<client> sampleClient(new client(L"<YOUR APP URL>", L"<YOUR MOBILE SERVICE KEY>"));

    create_task(PushNotificationChannelManager::CreatePushNotificationChannelForApplicationAsync()).
        then([sampleClient](PushNotificationChannel^ newChannel)
    {
        auto channelObject = json::value::object();

        channelObject[L"channelUri"] = json::value::string(newChannel->Uri->Data());

        auto token = HardwareIdentification::GetPackageSpecificToken(nullptr);
        auto installationId = CryptographicBuffer::EncodeToBase64String(token->Id);

        channelObject[L"installationId"] = json::value::string(installationId->Data());

        table channelsTable(*sampleClient, L"channels");

        channelsTable.insert(channelObject).then([](task<value> result)
        {
            // http://go.microsoft.com/fwlink/?LinkID=290991&clcid=0x409        
            try
            {
                result.wait();
            }
            catch (...)
            {
                // Handle exceptions coming from the server
            }
        }, ctx::use_current());
    }, ctx::use_current());
    #endif
}