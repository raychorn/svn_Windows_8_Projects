'*********************************************************
'
' Copyright (c) Microsoft. All rights reserved.
' THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
' IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
' PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
'
'*********************************************************

Imports Windows.UI.Xaml
Imports Windows.UI.Xaml.Controls
Imports Windows.UI.Xaml.Navigation
Imports SDKTemplate
Imports System

Imports System.Linq
Imports System.Collections.Generic
Imports Windows.Foundation
Imports Windows.Foundation.Collections
Imports Windows.Graphics.Display
Imports Windows.UI.ViewManagement
Imports Windows.UI.Xaml.Controls.Primitives
Imports Windows.UI.Xaml.Data
Imports Windows.UI.Xaml.Input
Imports Windows.UI.Xaml.Media
Imports Windows.UI.Core
Imports Windows.Storage.Pickers
Imports Windows.Storage
Imports Windows.Storage.Streams
Imports Windows.UI.Xaml.Media.Imaging
Imports Windows.System.UserProfile

Partial Public NotInheritable Class SetAccountPictureAndListen
    Inherits SDKTemplate.Common.LayoutAwarePage
    ' A pointer back to the main page.  This is needed if you want to call methods in MainPage such
    ' as NotifyUser()
    Private rootPage As MainPage = MainPage.Current

    Public Sub New()
        InitializeComponent()
    End Sub

    ''' <summary>
    ''' Invoked when this page is about to be displayed in a Frame.
    ''' </summary>
    ''' <param name="e">Event data that describes how this page was reached.  The Parameter
    ''' property is typically used to configure the page.</param>
    Protected Overrides Sub OnNavigatedTo(e As NavigationEventArgs)
        'Listen to AccountPictureChanged event
        AddHandler UserInformation.AccountPictureChanged, AddressOf Me.PictureChanged
    End Sub

    Protected Overrides Sub OnNavigatedFrom(e As NavigationEventArgs)
        'Remove listener to AccountPictureChanged event
        RemoveHandler UserInformation.AccountPictureChanged, AddressOf Me.PictureChanged
    End Sub

    Private Async Sub SetImage_Click(sender As Object, e As RoutedEventArgs)
        Dim imagePicker As New FileOpenPicker() With { _
            .ViewMode = PickerViewMode.Thumbnail, _
            .SuggestedStartLocation = PickerLocationId.PicturesLibrary
        }


        imagePicker.FileTypeFilter.Add(".jpg")
        imagePicker.FileTypeFilter.Add(".jpeg")
        imagePicker.FileTypeFilter.Add(".png")
        imagePicker.FileTypeFilter.Add(".bmp")
        
        Dim imageFile As StorageFile = Await imagePicker.PickSingleFileAsync()
        If imageFile IsNot Nothing Then
            ' SetAccountPictureAsync() accepts 3 storageFile objects for setting the small image, large image, and video.
            ' More than one type can be set in the same call, but a small image must be accompanied by a large image and/or video.
            ' If only a large image is passed, the small image will be autogenerated.
            ' If only a video is passed, the large image and small will be autogenerated.
            ' Videos must be convertable to mp4, <=5MB, and height and width >= 448 pixels.
            Try
                Dim result As SetAccountPictureResult = Await UserInformation.SetAccountPicturesAsync(Nothing, imageFile, Nothing)
                If result = SetAccountPictureResult.Success Then
                    rootPage.NotifyUser("Video account picture was successfully changed.", NotifyType.StatusMessage)
                Else
                    rootPage.NotifyUser("Account picture could not be changed.", NotifyType.StatusMessage)
                    accountPic.Visibility = Visibility.Collapsed
                End If
            Catch ex As Exception
                rootPage.NotifyUser("Error setting video account picture: " & ex.ToString, NotifyType.StatusMessage)
                accountPic.Visibility = Visibility.Collapsed
            End Try
        End If
    End Sub

    Private Async Sub SetVideo_Click(sender As Object, e As RoutedEventArgs)
        Dim videoPicker As New FileOpenPicker() With { _
            .ViewMode = PickerViewMode.Thumbnail, _
            .SuggestedStartLocation = PickerLocationId.VideosLibrary
        }

        videoPicker.FileTypeFilter.Add(".mp4")
        videoPicker.FileTypeFilter.Add(".mpg")
        videoPicker.FileTypeFilter.Add(".mpeg")
        videoPicker.FileTypeFilter.Add(".wmv")
        videoPicker.FileTypeFilter.Add(".mov")


        Dim videoFile As StorageFile = Await videoPicker.PickSingleFileAsync()
        If videoFile IsNot Nothing Then
            ' SetAccountPictureAsync() accepts 3 storageFile objects for setting the small image, large image, and video.
            ' More than one type can be set in the same call, but small image must be accompanied by a large image and/or video.
            ' If only a large image is passed, the small image will be autogenerated.
            ' If only a video is passed, the large image and small will be autogenerated.
            ' Videos must be convertable to mp4, <=5MB, and height and width >= 448 pixels.
            Try
                Dim result As SetAccountPictureResult = Await UserInformation.SetAccountPicturesAsync(Nothing, Nothing, videoFile)
                If result = SetAccountPictureResult.Success Then
                    rootPage.NotifyUser("Video account picture was successfully changed.", NotifyType.StatusMessage)
                Else
                    rootPage.NotifyUser("Video account picture could not be changed.", NotifyType.StatusMessage)
                    accountPic.Visibility = Visibility.Collapsed
                End If
            Catch ex As Exception
                rootPage.NotifyUser("Error setting video account picture: " & ex.ToString, NotifyType.StatusMessage)
                accountPic.Visibility = Visibility.Collapsed
            End Try
        End If
    End Sub

    Private Async Sub PictureChanged(sender As Object, e As Object)
        ' The large picture returned by GetAccountPicture() is 448x448 pixels in size.
        Dim image As StorageFile = TryCast(UserInformation.GetAccountPicture(AccountPictureKind.LargeImage), StorageFile)
        If image IsNot Nothing Then
            Try
                Dim imageStream As IRandomAccessStream = Await image.OpenReadAsync()
                Await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, Sub()
                                                                             Dim bitmapImage As New BitmapImage()
                                                                             bitmapImage.SetSource(imageStream)
                                                                             rootPage.NotifyUser("LargeImage path = " & image.Path, NotifyType.StatusMessage)
                                                                             accountPic.Source = bitmapImage
                                                                             accountPic.Visibility = Visibility.Visible
                                                                         End Sub)
            Catch ex As Exception
                rootPage.NotifyUser("Error opening stream: " & ex.ToString, NotifyType.ErrorMessage)
            End Try
        Else
            Await Me.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, Sub()
                                                                            rootPage.NotifyUser("Large Account Picture is not available", NotifyType.StatusMessage)
                                                                        End Sub)
        End If
    End Sub
End Class
