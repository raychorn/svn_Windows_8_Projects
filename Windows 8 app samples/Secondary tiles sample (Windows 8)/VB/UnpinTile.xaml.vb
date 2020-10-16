﻿'*********************************************************
'
' Copyright (c) Microsoft. All rights reserved.
' THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
' IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
' PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
'
'*********************************************************

Imports System
Imports SDKTemplate
Imports Windows.UI.StartScreen
Imports Windows.UI.Xaml
Imports Windows.UI.Xaml.Controls
Imports Windows.UI.Xaml.Navigation

''' <summary>
''' An empty page that can be used on its own or navigated to within a Frame.
''' </summary>
Partial Public NotInheritable Class UnpinTile
    Inherits SDKTemplate.Common.LayoutAwarePage
    ' A pointer back to the main page.  This is needed if you want to call methods in MainPage such
    ' as NotifyUser()
    Private rootPage As MainPage = MainPage.Current
    Private Appbar As AppBar

    Public Sub New()
        Me.InitializeComponent()
    End Sub

    ''' <summary>
    ''' Invoked when this page is about to be displayed in a Frame.
    ''' </summary>
    ''' <param name="e">Event data that describes how this page was reached.  The Parameter
    ''' property is typically used to configure the page.</param>
    Protected Overrides Sub OnNavigatedTo(e As NavigationEventArgs)
        Appbar = rootPage.BottomAppBar
        rootPage.BottomAppBar = Nothing
    End Sub

    ''' <summary>
    ''' Invoked when this page is about to be navigated out in a Frame
    ''' </summary>
    ''' <param name="e">Event data that describes how this page was reached.  The Parameter
    ''' property is typically used to configure the page.</param>
    Protected Overrides Sub OnNavigatedFrom(e As NavigationEventArgs)
        rootPage.BottomAppBar = AppBar
    End Sub


    ''' <summary>
    ''' This is the click handler for the 'Unpin' button.
    ''' </summary>
    ''' <param name="sender"></param>
    ''' <param name="e"></param>
    Private Async Sub UnpinSecondaryTile_Click(sender As Object, e As RoutedEventArgs)
        Dim button As Button = TryCast(sender, Button)
        If button IsNot Nothing Then
            If Windows.UI.StartScreen.SecondaryTile.Exists(MainPage.logoSecondaryTileId) Then
                ' First prepare the tile to be unpinned
                Dim secondaryTile As New SecondaryTile(MainPage.logoSecondaryTileId)
                ' Now make the delete request.
                Dim isUnpinned As Boolean = Await secondaryTile.RequestDeleteForSelectionAsync(MainPage.GetElementRect(DirectCast(sender, FrameworkElement)), Windows.UI.Popups.Placement.Below)
                If isUnpinned Then
                    rootPage.NotifyUser("Secondary tile successfully unpinned.", NotifyType.StatusMessage)
                Else
                    rootPage.NotifyUser("Secondary tile not unpinned.", NotifyType.ErrorMessage)
                End If
            Else
                rootPage.NotifyUser(MainPage.logoSecondaryTileId + " is not currently pinned.", NotifyType.ErrorMessage)
            End If
        End If
    End Sub
End Class
