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
// PeerFinderScenario.xaml.cpp
// Implementation of the PeerFinderScenario class
//



#include "pch.h"
#include "PeerFinderScenario.xaml.h"

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

PeerFinderScenario::PeerFinderScenario()
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

    m_socketHelper = ref new SocketHelper();
    m_socketHelper->SocketError += ref new SocketErrorEventHandler(this, &PeerFinderScenario::SocketErrorHandler);
    m_socketHelper->MessageSent += ref new MessageEventHandler(this, &PeerFinderScenario::MessageHandler);

    if (m_triggeredConnectSupported || m_browseConnectSupported)
    {
        PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void PeerFinderScenario::OnNavigatedTo(NavigationEventArgs^ e)
{
    PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_BrowsePeersButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    PeerFinder_FoundPeersList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SendToPeerList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SendButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_AcceptButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_MessageBox->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_DiscoveryData->Visibility =Windows::UI::Xaml::Visibility::Collapsed;

    if (m_triggeredConnectSupported || m_browseConnectSupported)
    {
        PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
        
        PeerFinder_MessageBox->Text = "Hello World";
        
        PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_DiscoveryData->Visibility =Windows::UI::Xaml::Visibility::Visible;
        PeerFinder_DiscoveryData->Text = "What's happening today?";

        m_rootPage->ClearLog(PeerFinderOutputText);
        if (m_rootPage->IsLaunchedByTap())
        {
            m_rootPage->NotifyUser("Launched by tap", NotifyType::StatusMessage);
            PeerFinder_StartAdvertising(nullptr, nullptr);
        }
        else
        {
            if (!m_triggeredConnectSupported)
            {
                m_rootPage->NotifyUser("Tap based discovery of peers not supported", NotifyType::ErrorMessage);
            }
            else if (!m_browseConnectSupported)
            {
                m_rootPage->NotifyUser("Browsing for peers not supported", NotifyType::ErrorMessage);
            }
        }
    }
    else
    {
        m_rootPage->NotifyUser("Tap based discovery of peers not supported \nBrowsing for peers not supported", NotifyType::ErrorMessage);
    }
}

void PeerFinderScenario::OnNavigatingFrom(NavigatingCancelEventArgs^ e)
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
}

// This gets called when we receive a connect request from a Peer
void PeerFinderScenario::ConnectionRequestedEventHandler(Object^ sender, ConnectionRequestedEventArgs^ connectionEventArgs) 
{
    m_rootPage->NotifyUser("Connection requested from peer " + connectionEventArgs->PeerInformation->DisplayName, NotifyType::StatusMessage);
    m_requestingPeer = connectionEventArgs->PeerInformation;
    PeerFinder_AcceptButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
    PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_BrowsePeersButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SendButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_MessageBox->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_DiscoveryData->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_FoundPeersList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void PeerFinderScenario::TriggeredConnectionStateChangedEventHandler(Object^ sender, TriggeredConnectionStateChangedEventArgs^ args) 
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

void PeerFinderScenario::PeerFinder_StartAdvertising(Object^ sender, RoutedEventArgs^ e) 
{
    // If PeerFinder is started, stop it, so that new properties
    // selected by the user (Role/DiscoveryData) can be updated.
    if (m_peerFinderStarted)
    {
        PeerFinder::Stop();
        m_peerFinderStarted = false;
    }

    // Then start listening for proximate peers
    m_rootPage->NotifyUser("", NotifyType::ErrorMessage);
    if (m_peerFinderStarted == false)
    {
        // First attach the callback handler
        m_triggerToken = PeerFinder::TriggeredConnectionStateChanged += ref new TypedEventHandler<Object^, TriggeredConnectionStateChangedEventArgs^>(this,
            &PeerFinderScenario::TriggeredConnectionStateChangedEventHandler, CallbackContext::Same);
        m_connectionRequestedToken = PeerFinder::ConnectionRequested += ref new TypedEventHandler<Object^, Windows::Networking::Proximity::ConnectionRequestedEventArgs^>(this, 
            &PeerFinderScenario::ConnectionRequestedEventHandler, CallbackContext::Same);

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
        PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Visible;

        if (m_browseConnectSupported && m_triggeredConnectSupported)
        {
            m_rootPage->NotifyUser("Tap another device to connect to a peer or click Browse for Peers button.", NotifyType::StatusMessage);
            PeerFinder_BrowsePeersButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
            PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Visible;
            PeerFinder_DiscoveryData->Visibility = Windows::UI::Xaml::Visibility::Visible;
        }
        else if (m_triggeredConnectSupported)
        {
            m_rootPage->NotifyUser("Tap another device to connect to a peer.", NotifyType::StatusMessage);
            PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Visible;
        }
        else
        {
            m_rootPage->NotifyUser("Click Browse for Peers button.", NotifyType::StatusMessage);
            PeerFinder_BrowsePeersButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
            PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Visible;
            PeerFinder_DiscoveryData->Visibility = Windows::UI::Xaml::Visibility::Visible;
        }
    }
}

void PeerFinderScenario::PeerFinder_BrowsePeers(Object^ sender, RoutedEventArgs^ e) 
{
    // Find all discoverable peers with compatible roles
    m_rootPage->NotifyUser("Finding Peers...", NotifyType::StatusMessage);
    auto op = PeerFinder::FindAllPeersAsync();
    concurrency::task<IVectorView<PeerInformation^>^> findAllPeersTask(op);

    findAllPeersTask.then([this](concurrency::task<IVectorView<PeerInformation^>^> resultTask)
    {
        try
        {
            // Clear the list containing the previous discovery results
            PeerFinder_FoundPeersList->Items->Clear();

            m_peerInformationList = resultTask.get();
            if (m_peerInformationList && m_peerInformationList->Size > 0)
            {
                for (unsigned int i = 0; i < m_peerInformationList->Size; i++)
                {
                    ComboBoxItem^ item = ref new ComboBoxItem();
                    String^ DisplayName = m_peerInformationList->GetAt(i)->DisplayName;

                     // Append the DiscoveryData text to the DisplayName
                    if (m_peerInformationList->GetAt(i)->DiscoveryData != nullptr)
                    {
                        Windows::Storage::Streams::DataReader^ discoveryDataReader = Windows::Storage::Streams::DataReader::FromBuffer(m_peerInformationList->GetAt(i)->DiscoveryData);
                        String^ DiscoveryData = discoveryDataReader->ReadString(m_peerInformationList->GetAt(i)->DiscoveryData->Length);
                        DisplayName += " '" + DiscoveryData + "'";
                    }

                    item->Content = DisplayName;
                    item->Tag = m_peerInformationList->GetAt(i);
                    PeerFinder_FoundPeersList->Items->Append(item);
                }
                PeerFinder_FoundPeersList->SelectedIndex = 0;
                PeerFinder_FoundPeersList->Visibility = Windows::UI::Xaml::Visibility::Visible;
                PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
                m_rootPage->NotifyUser("Finding Peers Done", NotifyType::StatusMessage);
            }
            else
            {
                // Indicate that no peers were found by adding a "None Found"
                // item in the peer list.
                ComboBoxItem^ item = ref new ComboBoxItem();
                item->Content = "None Found";
                PeerFinder_FoundPeersList->Items->Append(item);
                PeerFinder_FoundPeersList->SelectedIndex = 0;
                PeerFinder_FoundPeersList->Visibility = Windows::UI::Xaml::Visibility::Visible;

                m_rootPage->NotifyUser("No peers found", NotifyType::StatusMessage);
                PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
            }
        }
        catch (Exception^ e)
        {
            m_rootPage->NotifyUser("Exception occurred while finding peer: " + e->Message, NotifyType::ErrorMessage);
        }
    });
}

void PeerFinderScenario::PeerFinder_Accept(Object^ sender, RoutedEventArgs^ e)
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

void PeerFinderScenario::PeerFinder_Connect(Object^ sender, RoutedEventArgs^ e)
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
            peerToConnect = safe_cast<PeerInformation^>(safe_cast<ComboBoxItem^>(PeerFinder_FoundPeersList->SelectedItem)->Tag);
        }
        catch (InvalidCastException^)
        {
            peerToConnect = nullptr;
        }
        if (peerToConnect == nullptr)
        {
            peerToConnect = safe_cast<PeerInformation^>(safe_cast<ComboBoxItem^>(PeerFinder_FoundPeersList->Items->GetAt(0))->Tag);
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

void PeerFinderScenario::SocketErrorHandler(SocketHelper^ sender, Platform::String^ errMessage)
{
    m_rootPage->NotifyUser(errMessage, NotifyType::ErrorMessage);
    PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Visible;

    // Browse and DiscoveryData controls are valid for Browse support
    if (m_browseConnectSupported)
    {
        PeerFinder_BrowsePeersButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }
    PeerFinder_SendButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_MessageBox->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    // Clear the SendToPeerList
    PeerFinder_SendToPeerList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SendToPeerList->Items->Clear();

    m_socketHelper->CloseSocket();
}

void PeerFinderScenario::MessageHandler(SocketHelper^ sender, Platform::String^ message)
{
    m_rootPage->NotifyUser(message, NotifyType::StatusMessage);
}

// Send message to the selected peer(s)
void PeerFinderScenario::PeerFinder_Send(Object^ sender, RoutedEventArgs^ e)
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
void PeerFinderScenario::PeerFinder_StartSendReceive(Windows::Networking::Sockets::StreamSocket^ socket, Windows::Networking::Proximity::PeerInformation^ peerInformation)
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
    PeerFinder_ConnectButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_AcceptButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_FoundPeersList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_BrowsePeersButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_AdvertiseButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_SelectRole->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    PeerFinder_DiscoveryData->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    connectedPeer->SetSocketClosedState(false);
    m_socketHelper->StartReader(ref new DataReader(connectedPeer->GetSocket()->InputStream), connectedPeer->IsSocketClosed());
}
