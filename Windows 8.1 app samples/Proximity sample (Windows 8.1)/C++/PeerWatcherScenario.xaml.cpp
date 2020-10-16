﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// PeerWatcherScenario.xaml.cpp
// Implementation of the PeerWatcherScenario class
//



#include "pch.h"
#include "PeerWatcherScenario.xaml.h"

using namespace ProximityCPP;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Networking::Proximity;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Platform;

PeerWatcherScenario::PeerWatcherScenario()
{
    InitializeComponent();
    // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
    // as NotifyUser()
    m_rootPage = MainPage::Current;

    m_coreDispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;

    m_triggeredConnectSupported = (PeerFinder::SupportedDiscoveryTypes & PeerDiscoveryTypes::Triggered) ==
        PeerDiscoveryTypes::Triggered;
    m_browseConnectSupported = (PeerFinder::SupportedDiscoveryTypes & PeerDiscoveryTypes::Browse) ==
        PeerDiscoveryTypes::Browse;

    m_discoveredPeers = ref new Peers();
    
    m_socketHelper = ref new SocketHelper();
    m_socketHelper->SocketError += ref new SocketErrorEventHandler(this, &PeerWatcherScenario::SocketErrorHandler);
    m_socketHelper->MessageSent += ref new MessageEventHandler(this, &PeerWatcherScenario::MessageHandler);

    if (m_triggeredConnectSupported || m_browseConnectSupported)
    {
        PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_StopAdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Visible;

        peerListCVS->Source = m_discoveredPeers->Items;
    }
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void PeerWatcherScenario::OnNavigatedTo(NavigationEventArgs^ e)
{
    PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_StopAdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    PeerFinder_BrowseGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    PeerFinder_StopPeerWatcherButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_StartPeerWatcherButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    PeerFinder_ConnectionGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SendButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_AcceptButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_MessageBox->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_PeerListNoPeers->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_DiscoveryData->Visibility =Windows::UI::Xaml::Visibility::Collapsed;

    if (m_triggeredConnectSupported || m_browseConnectSupported)
    {
        PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
        
        // These are hidden in the grid
        if (m_browseConnectSupported)
        {
            PeerFinder_StartPeerWatcherButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
        }

        PeerFinder_MessageBox->Text = "Hello World";
        PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_DiscoveryData->Visibility =Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_DiscoveryData->Text = "What's happening today?";

        m_rootPage->ClearLog(PeerFinderOutputText);
        if (!m_triggeredConnectSupported)
        {
            m_rootPage->NotifyUser("Tap based discovery of peers not supported", NotifyType::ErrorMessage);
        }
        else if (!m_browseConnectSupported)
        {
            m_rootPage->NotifyUser("Browsing for peers not supported", NotifyType::ErrorMessage);
        }
    }
    else
    {
        m_rootPage->NotifyUser("Tap based discovery of peers not supported \nBrowsing for peers not supported", NotifyType::ErrorMessage);
    }
}

void PeerWatcherScenario::OnNavigatingFrom(NavigatingCancelEventArgs^ e)
{
    // If PeerFinder was started, stop it when navigating to a different page.
    m_startPeerFinderImmediately = false;
    if (m_peerFinderStarted)
    {
        // unregister the events when the window goes out of scope
        PeerFinder::TriggeredConnectionStateChanged -= m_triggerToken;
        PeerFinder::ConnectionRequested -= m_connectionRequestedToken;
        m_socketHelper->CloseSocket();
        PeerFinder::Stop();
        m_peerFinderStarted = false;
    }
    if (m_peerWatcher != nullptr)
    {
        // unregister for events
        m_peerWatcher->Added -= m_peerAddedToken;
        m_peerWatcher->Removed -= m_peerRemovedToken;
        m_peerWatcher->Updated -= m_peerUpdatedToken;
        m_peerWatcher->EnumerationCompleted -= m_peerWatcherEnumerationCompletedToken;
        m_peerWatcher->Stopped -= m_peerWatcherStoppedToken;

        m_peerWatcher = nullptr;
    }
}

// This gets called when we receive a connect request from a Peer
void PeerWatcherScenario::ConnectionRequestedEventHandler(Object^ sender, ConnectionRequestedEventArgs^ connectionEventArgs) 
{
    m_rootPage->NotifyUser("Connection requested from peer " + connectionEventArgs->PeerInformation->DisplayName, NotifyType::StatusMessage);
    m_requestingPeer = connectionEventArgs->PeerInformation;
    PeerFinder_AcceptButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
    PeerFinder_AdvertiseGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_BrowseGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_ConnectionGrid->Visibility = Windows::UI::Xaml::Visibility::Visible;
    PeerFinder_SendButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_MessageBox->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SendToPeerList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void PeerWatcherScenario::TriggeredConnectionStateChangedEventHandler(Object^ sender, TriggeredConnectionStateChangedEventArgs^ args) 
{
    m_rootPage->LogInfo("TriggeredConnectionStateChangedEventHandler - " +
        args->State.ToString(), PeerFinderOutputText);

    if (args->State == TriggeredConnectState::PeerFound)
    {
        // Use this state to indicate to users that the tap is complete and
        // they can pull there devices away.
        m_rootPage->NotifyUser("Tap complete, socket connection starting!", NotifyType::StatusMessage);
    }

    if (args->State == TriggeredConnectState::Completed) 
    {
        m_rootPage->NotifyUser("Socket connect success!", NotifyType::StatusMessage);
        // Start using the socket that just connected.
        PeerFinder_StartSendReceive(args->Socket, nullptr);
    }

    if (args->State == TriggeredConnectState::Failed)
    {
        // The socket conenction failed
        m_rootPage->NotifyUser("Socket connect failed!", NotifyType::ErrorMessage);
    }
}

void PeerWatcherScenario::PeerFinder_StartAdvertising(Object^ sender, RoutedEventArgs^ e) 
{
    // If PeerFinder is started, stop it, so that new properties
    // selected by the user (Role/DiscoveryData) can be updated.
    PeerFinder_StopAdvertising(sender, e);

    // Then start listening for proximate peers
    m_rootPage->NotifyUser("", NotifyType::ErrorMessage);
    if (m_peerFinderStarted == false)
    {
        // First attach the callback handler
        m_triggerToken = PeerFinder::TriggeredConnectionStateChanged += ref new TypedEventHandler<Object^, TriggeredConnectionStateChangedEventArgs^>(this,
            &PeerWatcherScenario::TriggeredConnectionStateChangedEventHandler, CallbackContext::Same);
        m_connectionRequestedToken = PeerFinder::ConnectionRequested += ref new TypedEventHandler<Object^, Windows::Networking::Proximity::ConnectionRequestedEventArgs^>(this, 
            &PeerWatcherScenario::ConnectionRequestedEventHandler, CallbackContext::Same);

        // Set the PeerFinder.role property
        if (m_rootPage->IsLaunchedByTap())
        {
            PeerFinder::Role = m_rootPage->GetLaunchRole();
        }
        else
        {
            ComboBoxItem^ item = safe_cast<ComboBoxItem^>(PeerFinder_SelectRole->SelectedValue);
        
            if (item->Content->ToString() == "Peer")
            {
                PeerFinder::Role = PeerRole::Peer;
            }
            else if (item->Content->ToString() == "Host")
            {
                PeerFinder::Role = PeerRole::Host;
            }
            else
            {
                PeerFinder::Role = PeerRole::Client;
            }
        }

        // Set DiscoveryData property if the user entered some text
        if ((PeerFinder_DiscoveryData->Text->Length() > 0) && (PeerFinder_DiscoveryData->Text != "What's happening today?"))
        {
            DataWriter^ discoveryDataWriter = ref new Windows::Storage::Streams::DataWriter(ref new Windows::Storage::Streams::InMemoryRandomAccessStream());
            discoveryDataWriter->WriteString(PeerFinder_DiscoveryData->Text);
            PeerFinder::DiscoveryData = discoveryDataWriter->DetachBuffer();
        }

        PeerFinder::Start();
        m_peerFinderStarted = true;
        PeerFinder_StopAdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Visible;

        if (m_browseConnectSupported && m_triggeredConnectSupported)
        {
            PeerFinder_BrowseGrid->Visibility = Windows::UI::Xaml::Visibility::Visible;
            m_rootPage->NotifyUser("Tap another device to connect to a peer or click Start Watching for Peers button.", NotifyType::StatusMessage);
        }
        else if (m_triggeredConnectSupported)
        {
            m_rootPage->NotifyUser("Tap another device to connect to a peer.", NotifyType::StatusMessage);
        }
        else
        {
            PeerFinder_BrowseGrid->Visibility = Windows::UI::Xaml::Visibility::Visible;
            m_rootPage->NotifyUser("Click Start Watching for Peers button.", NotifyType::StatusMessage);
        }
    }
}

void PeerWatcherScenario::PeerFinder_StopAdvertising(Object^ sender, RoutedEventArgs^ e)
{
    if (m_peerFinderStarted)
    {
        PeerFinder::Stop();
        m_peerFinderStarted = false;

        m_rootPage->NotifyUser("Stopped Advertising.", NotifyType::StatusMessage);
        PeerFinder_StopAdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        PeerFinder_BrowseGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    }
}

void PeerWatcherScenario::PeerFinder_StartPeerWatcher(Object^ sender, RoutedEventArgs^ e)
{
    if (m_peerWatcherIsRunning)
    {
        m_rootPage->LogInfo("Can't start PeerWatcher while it is running!", PeerFinderOutputText);
        return;
    }

    m_rootPage->NotifyUser("Starting PeerWatcher...", NotifyType::StatusMessage);
    try
    {
        if (m_peerWatcher == nullptr)
        {
            m_peerWatcher = PeerFinder::CreateWatcher();
            // Hook up events, this should only be done once
            m_peerAddedToken = m_peerWatcher->Added += ref new TypedEventHandler<PeerWatcher^, PeerInformation^>(this,
                &PeerWatcherScenario::PeerWatcher_Added, CallbackContext::Same);
            m_peerRemovedToken = m_peerWatcher->Removed += ref new TypedEventHandler<PeerWatcher^, PeerInformation^>(this,
                &PeerWatcherScenario::PeerWatcher_Removed, CallbackContext::Same);
            m_peerUpdatedToken = m_peerWatcher->Updated += ref new TypedEventHandler<PeerWatcher^, PeerInformation^>(this,
                &PeerWatcherScenario::PeerWatcher_Updated, CallbackContext::Same);
            m_peerWatcherEnumerationCompletedToken = m_peerWatcher->EnumerationCompleted += ref new TypedEventHandler<PeerWatcher^, Object^>(this,
                &PeerWatcherScenario::PeerWatcher_EnumerationCompleted, CallbackContext::Same);
            m_peerWatcherStoppedToken = m_peerWatcher->Stopped += ref new TypedEventHandler<PeerWatcher^, Object^>(this,
                &PeerWatcherScenario::PeerWatcher_Stopped, CallbackContext::Same);
        }

        peerListCVS->Dispatcher->RunAsync(CoreDispatcherPriority::Low,
            ref new DispatchedHandler([this]()
        {
            // Can't use Clear() because the ListView binding doesn't like it!
            while (m_discoveredPeers->Items->Size > 0)
            {
                m_discoveredPeers->Items->RemoveAt(0);
            }
        }));

        PeerFinder_PeerListNoPeers->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Visible;

        m_peerWatcher->Start();
        
        m_peerWatcherIsRunning = true;
        m_rootPage->LogInfo("PeerWatcher is running!", PeerFinderOutputText);

        PeerFinder_StartPeerWatcherButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        PeerFinder_StopPeerWatcherButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }
    catch (Exception^ ex)
    {
        // This could happen if the user clicks start multiple times or tries to start while PeerWatcher is stopping
        m_rootPage->NotifyUser("PeerWatcher.Start throws exception" + ex->Message, NotifyType::ErrorMessage);
    }
}

void PeerWatcherScenario::PeerFinder_StopPeerWatcher(Object^ sender, RoutedEventArgs^ e)
{
    m_rootPage->LogInfo("Stopping PeerWatcher... wait for Stopped Event", PeerFinderOutputText);
    try
    {
        m_peerWatcher->Stop();
    }
    catch (Exception^ ex)
    {
        m_rootPage->NotifyUser("PeerWatcher.Stop throws exception" + ex->Message, NotifyType::ErrorMessage);
    }
}

// Helper for more readable logging
String^ PeerWatcherScenario::GetTruncatedPeerId(String^ id)
{
    std::wstring idStr(id->Data());
    std::wstring temp;
    String^ truncated;
    if (idStr.length() > 10)
    {
        temp = idStr.substr(0, 5);
        temp += L"...";
        temp += idStr.substr(idStr.length() - 5);
        truncated = ref new String(temp.c_str());
    }
    else
    {
        truncated = id;
    }
    return truncated;
}

void PeerWatcherScenario::PeerWatcher_Added(PeerWatcher^ /*sender*/, PeerInformation^ peerInfo)
{
    peerListCVS->Dispatcher->RunAsync(CoreDispatcherPriority::Low,
        ref new DispatchedHandler([this, peerInfo]()
    {
        m_rootPage->LogInfo("Peer added: "
            + GetTruncatedPeerId(peerInfo->Id)
            + ", name:" + peerInfo->DisplayName, PeerFinderOutputText);
        // Update the UI
        PeerFinder_PeerListNoPeers->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        std::lock_guard<std::mutex> lock(m_peerListMutex);
        m_discoveredPeers->Items->Append(peerInfo);
    }));
}

void PeerWatcherScenario::PeerWatcher_Removed(PeerWatcher^ /*sender*/, PeerInformation^ peerInfo)
{
    peerListCVS->Dispatcher->RunAsync(CoreDispatcherPriority::Low,
        ref new DispatchedHandler([this, peerInfo]()
    {
        m_rootPage->LogInfo("Peer removed: "
            + GetTruncatedPeerId(peerInfo->Id)
            + ", name:" + peerInfo->DisplayName, PeerFinderOutputText);
        // Update the UI
        std::lock_guard<std::mutex> lock(m_peerListMutex);
        for (unsigned int i = 0; i < m_discoveredPeers->Items->Size; i++)
        {
            if (m_discoveredPeers->Items->GetAt(i)->Id == peerInfo->Id)
            {
                m_discoveredPeers->Items->RemoveAt(i);
                break;
            }
        }
    }));
}

void PeerWatcherScenario::PeerWatcher_Updated(PeerWatcher^ /*sender*/, PeerInformation^ peerInfo)
{
    peerListCVS->Dispatcher->RunAsync(CoreDispatcherPriority::Low,
        ref new DispatchedHandler([this, peerInfo]()
    {
        m_rootPage->LogInfo("Peer updated: "
            + GetTruncatedPeerId(peerInfo->Id)
            + ", name:" + peerInfo->DisplayName, PeerFinderOutputText);
        // Update the UI
        std::lock_guard<std::mutex> lock(m_peerListMutex);
        for (unsigned int i = 0; i < m_discoveredPeers->Items->Size; i++)
        {
            if (m_discoveredPeers->Items->GetAt(i)->Id == peerInfo->Id)
            {
                m_discoveredPeers->Items->SetAt(i, peerInfo);
                break;
            }
        }
    }));
}

void PeerWatcherScenario::PeerWatcher_EnumerationCompleted(PeerWatcher^ /*sender*/, Object^)
{
    m_rootPage->LogInfo("PeerWatcher Enumeration Completed", PeerFinderOutputText);
    // All peers that were visible at the start of the scan have been found
    // Stopping PeerWatcher here is similar to FindAllPeersAsync

    // Notify the user that no peers were found after we have done an initial scan
    peerListCVS->Dispatcher->RunAsync(CoreDispatcherPriority::Low,
        ref new DispatchedHandler([this]()
    {
        std::lock_guard<std::mutex> lock(m_peerListMutex);
        if (m_discoveredPeers->Items->Size == 0)
        {
            PeerFinder_PeerListNoPeers->Visibility = Windows::UI::Xaml::Visibility::Visible;
        }
    }));
}

void PeerWatcherScenario::PeerWatcher_Stopped(PeerWatcher^ /*sender*/, Object^)
{
    // This indicates that the PeerWatcher was stopped explicitly through PeerWatcher.Stop, or it was aborted
    // The Status property indicates the cause of the event
    m_rootPage->LogInfo("PeerWatcher Stopped. Status: " + m_peerWatcher->Status.ToString(), PeerFinderOutputText);
    // PeerWatcher is now actually stopped and we can start it again, update the UI button state accordingly
    m_peerWatcherIsRunning = false;
    peerListCVS->Dispatcher->RunAsync(CoreDispatcherPriority::Low,
        ref new DispatchedHandler([this]()
    {
        PeerFinder_StartPeerWatcherButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_StopPeerWatcherButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    }));
}


void PeerWatcherScenario::PeerFinder_Accept(Object^ sender, RoutedEventArgs^ e)
{
    m_rootPage->NotifyUser("Connecting to " + m_requestingPeer->DisplayName + "....", NotifyType::StatusMessage);
    PeerFinder_AcceptButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    // Connect to the incoming peer
    auto op = PeerFinder::ConnectAsync(m_requestingPeer);
    concurrency::task<StreamSocket^> connectTask(op);
    connectTask.then([this](concurrency::task<StreamSocket^> resultTask)
    {
        try
        {
            m_rootPage->NotifyUser("Connection succeeded", NotifyType::StatusMessage);
            PeerFinder_StartSendReceive(resultTask.get(), m_requestingPeer);
        }
        catch(Exception^ e)
        {
            m_rootPage->NotifyUser("Connection to " + m_requestingPeer->DisplayName + " failed: " + e->Message, NotifyType::ErrorMessage);
        }
    });
}

void PeerWatcherScenario::PeerFinder_Connect(Object^ sender, RoutedEventArgs^ e)
{
    m_rootPage->NotifyUser("", NotifyType::ErrorMessage);
    Windows::Networking::Proximity::PeerInformation^ peerToConnect = nullptr;

    if (PeerFinder_FoundPeersList->Items->Size == 0)
    {
        m_rootPage->NotifyUser("Cannot connect, there were no peers found!", NotifyType::ErrorMessage);
    }
    else
    {
        try
        {
            peerToConnect = safe_cast<PeerInformation^>(PeerFinder_FoundPeersList->SelectedItem);
        }
        catch (InvalidCastException^)
        {
            peerToConnect = nullptr;
        }
        if (peerToConnect == nullptr)
        {
            peerToConnect = safe_cast<PeerInformation^>(PeerFinder_FoundPeersList->Items->GetAt(0));
        }

        m_rootPage->NotifyUser("Connecting to " + peerToConnect->DisplayName + "....", NotifyType::StatusMessage);
        auto op = PeerFinder::ConnectAsync(peerToConnect);

        concurrency::task<StreamSocket^> connectTask(op);
        connectTask.then([this, peerToConnect](concurrency::task<StreamSocket^> resultTask)
        {
            try
            {
                m_rootPage->NotifyUser("Connection succeeded", NotifyType::StatusMessage);
                PeerFinder_StartSendReceive(resultTask.get(), peerToConnect); 
            }
            catch(Exception ^e)
            {
                m_rootPage->NotifyUser("Connection to " + peerToConnect->DisplayName + " failed: " + e->Message, NotifyType::ErrorMessage);            
            }
        });
    }
}

void PeerWatcherScenario::SocketErrorHandler(SocketHelper^ sender, Platform::String^ errMessage)
{
    m_rootPage->NotifyUser(errMessage, NotifyType::ErrorMessage);
    PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
    PeerFinder_StopAdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_AdvertiseGrid->Visibility = Windows::UI::Xaml::Visibility::Visible;

    // Browse and DiscoveryData controls are valid for Browse support
    if (m_browseConnectSupported)
    {
        PeerFinder_BrowseGrid->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }
    PeerFinder_ConnectionGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    // Clear the SendToPeerList
    PeerFinder_SendToPeerList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SendToPeerList->Items->Clear();

    m_socketHelper->CloseSocket();
}

void PeerWatcherScenario::MessageHandler(SocketHelper^ sender, Platform::String^ message)
{
    m_rootPage->NotifyUser(message, NotifyType::StatusMessage);
}

// Send message to the selected peer(s)
void PeerWatcherScenario::PeerFinder_Send(Object^ sender, RoutedEventArgs^ e)
{
    m_rootPage->NotifyUser("", NotifyType::ErrorMessage);
    String^ message = PeerFinder_MessageBox->Text;
    PeerFinder_MessageBox->Text = ""; // clear the input now that the message is being sent.
    int idx = PeerFinder_SendToPeerList->SelectedIndex - 1;

    if (message->Length() > 0)
    {
        // Send message to all peers
        if ((safe_cast<ComboBoxItem^>(PeerFinder_SendToPeerList->SelectedItem))->Content->ToString() == "All Peers")
        {
            for each (ConnectedPeer^ obj in m_socketHelper->ConnectedPeers)
            {
                m_socketHelper->SendMessageToPeer(message, obj);
            }
        }
        else if ((idx >= 0) && (idx < (int)m_socketHelper->ConnectedPeers->Size))
        {
            // Send message to selected peer
            m_socketHelper->SendMessageToPeer(message, m_socketHelper->ConnectedPeers->GetAt(idx));
        }
    }
    else
    {
        m_rootPage->NotifyUser("Please type a message", NotifyType::ErrorMessage);
    }
}

// Start the send receive operations
void PeerWatcherScenario::PeerFinder_StartSendReceive(Windows::Networking::Sockets::StreamSocket^ socket, Windows::Networking::Proximity::PeerInformation^ peerInformation)
{
    ConnectedPeer^ connectedPeer = ref new ConnectedPeer();
    connectedPeer->SetSocket(socket);
    connectedPeer->SetWriter(ref new Windows::Storage::Streams::DataWriter(socket->OutputStream));
    connectedPeer->SetSocketClosedState(false);
    m_socketHelper->Add(connectedPeer);

    if (!m_peerFinderStarted)
    {
        m_socketHelper->CloseSocket();
        return;
    }

    PeerFinder_ConnectionGrid->Visibility = Windows::UI::Xaml::Visibility::Visible;
    PeerFinder_SendButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
    PeerFinder_MessageBox->Visibility = Windows::UI::Xaml::Visibility::Visible;
    PeerFinder_SendToPeerList->Visibility = Windows::UI::Xaml::Visibility::Visible;

    if (peerInformation != nullptr)
    {
        // Add a new peer to the list of peers.
        ComboBoxItem^ item = ref new ComboBoxItem();
        item->Content = peerInformation->DisplayName;
        PeerFinder_SendToPeerList->Items->Append(item);
        PeerFinder_SendToPeerList->SelectedIndex = 0;
    }

    // Hide the controls related to setting up a connection
    PeerFinder_AcceptButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_BrowseGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_AdvertiseGrid->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    connectedPeer->SetSocketClosedState(false);
    m_socketHelper->StartReader(ref new DataReader(connectedPeer->GetSocket()->InputStream), connectedPeer->IsSocketClosed());
}
