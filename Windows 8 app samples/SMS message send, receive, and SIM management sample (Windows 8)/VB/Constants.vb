'*********************************************************
'
' Copyright (c) Microsoft. All rights reserved.
' THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
' IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
' PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
'
'*********************************************************

Namespace Global.SDKTemplate
    Partial Public Class MainPage
        Inherits SDKTemplate.Common.LayoutAwarePage
        Public Const FEATURE_NAME As String = "SMS Send/Recieve Sample"

        Public Shared _ScenarioList As New List(Of Scenario)() From {New Scenario With {.Title = "Sending a text message", .ClassType = GetType(SmsSendReceive.SendMessage)},
                                                               New Scenario With {.Title = "Receiving a text message", .ClassType = GetType(SmsSendReceive.ReceiveMessage)},
                                                               New Scenario With {.Title = "Sending a message in PDU format", .ClassType = GetType(SmsSendReceive.SendPduMessage)},
                                                               New Scenario With {.Title = "Reading a message from SIM", .ClassType = GetType(SmsSendReceive.ReadMessage)},
                                                               New Scenario With {.Title = "Deleting a message from SIM", .ClassType = GetType(SmsSendReceive.DeleteMessage)}
                                                              }

    End Class

    Public Class Scenario
        Public Property Title() As String
            Get
                Return m_Title
            End Get
            Set(value As String)
                m_Title = value
            End Set
        End Property
        Private m_Title As String

        Public Property ClassType() As Type
            Get
                Return m_ClassType
            End Get
            Set(value As Type)
                m_ClassType = value
            End Set
        End Property
        Private m_ClassType As Type

        Public Overrides Function ToString() As String
            Return Title
        End Function
    End Class
End Namespace

