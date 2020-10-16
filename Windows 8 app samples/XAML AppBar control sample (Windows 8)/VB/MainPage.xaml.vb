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
Imports Windows.UI.ViewManagement
Imports Windows.UI.Xaml
Imports Windows.UI.Xaml.Controls
Imports Windows.UI.Xaml.Markup
Imports Windows.UI.Xaml.Navigation


Namespace Global.SDKTemplate
    ''' <summary>
    ''' An empty page that can be used on its own or navigated to within a Frame.
    ''' </summary>
    Partial Public NotInheritable Class MainPage
        Inherits SDKTemplate.Common.LayoutAwarePage
        Public Event ScenarioLoaded As System.EventHandler
        Public Event MainPageResized As EventHandler(Of MainPageSizeChangedEventArgs)
        Public AutoSizeInputSectionWhenSnapped As Boolean = True

        Public LaunchArgs As Windows.ApplicationModel.Activation.LaunchActivatedEventArgs

        Public Shared Current As MainPage

        Private HiddenFrame As Frame = Nothing

        Public Sub New()
            Me.InitializeComponent()

            ' This is a static public property that will allow downstream pages to get 
            ' a handle to the MainPage instance in order to call methods that are in this class.
            Current = Me

            ' This frame is hidden, meaning it is never shown.  It is simply used to load
            ' each scenario page and then pluck out the input and output sections and
            ' place them into the UserControls on the main page.
            HiddenFrame = New Windows.UI.Xaml.Controls.Frame()
            HiddenFrame.Visibility = Windows.UI.Xaml.Visibility.Collapsed
            LayoutRoot.Children.Add(HiddenFrame)

            ' Populate the sample title from the constant in the GlobalVariables.cs file.
            SetFeatureName(FEATURE_NAME)

            AddHandler Scenarios.SelectionChanged, AddressOf Scenarios_SelectionChanged

            AddHandler SizeChanged, AddressOf MainPage_SizeChanged

            AddHandler BottomAppBar.Loaded, AddressOf BottomAppBar_Loaded
        End Sub

        Private Sub MainPage_SizeChanged(sender As Object, e As SizeChangedEventArgs)
            InvalidateSize()

            Dim args As New MainPageSizeChangedEventArgs()
            args.ViewState = ApplicationView.Value
            RaiseEvent MainPageResized(Me, args)
        End Sub

        ''' <summary>
        ''' Invoked when this page is about to be displayed in a Frame.
        ''' </summary>
        ''' <param name="e">Event data that describes how this page was reached.  The Parameter
        ''' property is typically used to configure the page.</param>
        Protected Overrides Sub OnNavigatedTo(e As NavigationEventArgs)
            PopulateScenarios()
            InvalidateViewState()
        End Sub

        Private Sub BottomAppBar_Loaded()
            RegisterAppBarButtons()
        End Sub

        Private Sub RegisterAppBarButtons()
            Dim rootGrid As Grid = TryCast(BottomAppBar.Content, Grid)
            For Each e As UIElement In rootGrid.Children
                For Each c As UIElement In TryCast(e, StackPanel).Children
                    If c.GetType() Is GetType(Button) Then
                        MyBase.StartLayoutUpdates(c, New RoutedEventArgs())
                    End If
                Next
            Next
        End Sub

        Private Sub InvalidateSize()
            ' Get the window width
            Dim windowWidth As Double = Me.ActualWidth
            If windowWidth <> 0.0 Then
                ' Get the width of the ListBox.
                Dim listBoxWidth As Double = Scenarios.ActualWidth

                ' Is the ListBox using any margins that we need to consider?
                Dim listBoxMarginLeft As Double = Scenarios.Margin.Left
                Dim listBoxMarginRight As Double = Scenarios.Margin.Right

                ' Figure out how much room is left after considering the list box width
                Dim availableWidth As Double = windowWidth - listBoxWidth

                ' Is the top most child using margins?
                Dim layoutRootMarginLeft As Double = ContentRoot.Margin.Left
                Dim layoutRootMarginRight As Double = ContentRoot.Margin.Right

                ' We have different widths to use depending on the view state
                If ApplicationView.Value <> ApplicationViewState.Snapped Then
                    ' Make us as big as the the left over space, factoring in the ListBox width, the ListBox margins.
                    ' and the LayoutRoot's margins
                    InputSection.Width = availableWidth - (layoutRootMarginLeft + layoutRootMarginRight + listBoxMarginLeft + listBoxMarginRight)
                Else
                    ' Make us as big as the left over space, factoring in just the LayoutRoot's margins.
                    If AutoSizeInputSectionWhenSnapped Then
                        InputSection.Width = windowWidth - (layoutRootMarginLeft + layoutRootMarginRight)
                    End If
                End If
            End If
            InvalidateViewState()
        End Sub

        Private Sub InvalidateViewState()
            If ApplicationView.Value = ApplicationViewState.Snapped Then
                Grid.SetRow(DescriptionText, 3)
                Grid.SetColumn(DescriptionText, 0)

                Grid.SetRow(InputSection, 4)
                Grid.SetColumn(InputSection, 0)

                Grid.SetRow(FooterPanel, 2)
                Grid.SetColumn(FooterPanel, 0)

                Grid.SetRow(RightPanel, 1)
            Else
                Grid.SetRow(DescriptionText, 1)
                Grid.SetColumn(DescriptionText, 1)

                Grid.SetRow(InputSection, 2)
                Grid.SetColumn(InputSection, 1)

                Grid.SetRow(FooterPanel, 1)
                Grid.SetColumn(FooterPanel, 1)

                Grid.SetRow(RightPanel, 0)
                Grid.SetColumn(RightPanel, 1)
            End If
        End Sub

        Private Sub PopulateScenarios()
            Dim ScenarioList As New System.Collections.ObjectModel.ObservableCollection(Of Object)()
            Dim i As Integer = 0

            ' Populate the ListBox with the list of scenarios as defined in Constants.cs.
            For Each s As Scenario In _ScenarioList
                Dim item As New ListBoxItem()
                s.Title = (System.Threading.Interlocked.Increment(i)).ToString() & ") " & s.Title
                item.Content = s
                item.Name = s.ClassType.FullName
                ScenarioList.Add(item)
            Next

            ' Bind the ListBox to the scenario list.
            Scenarios.ItemsSource = ScenarioList

            ' Starting scenario is the first or based upon a previous selection.
            Dim startingScenarioIndex As Integer = -1

            If SuspensionManager.SessionState.ContainsKey("SelectedScenarioIndex") Then
                Dim selectedScenarioIndex As Integer = Convert.ToInt32(SuspensionManager.SessionState("SelectedScenarioIndex"))
                startingScenarioIndex = selectedScenarioIndex
            End If
            Scenarios.SelectedIndex = If(startingScenarioIndex <> -1, startingScenarioIndex, 0)
        End Sub

        ''' <summary>
        ''' This method is responsible for loading the individual input and output sections for each scenario.  This 
        ''' is based on navigating a hidden Frame to the ScenarioX.xaml page and then extracting out the input
        ''' and output sections into the respective UserControl on the main page.
        ''' </summary>
        ''' <param name="scenarioName"></param>
        Public Sub LoadScenario(scenarioClass As Type)
            AutoSizeInputSectionWhenSnapped = True

            ' Load the ScenarioX.xaml file into the Frame.
            HiddenFrame.Navigate(scenarioClass, Me)

            ' Get the top element, the Page, so we can look up the elements
            ' that represent the input and output sections of the ScenarioX file.
            Dim hiddenPage As Page = TryCast(HiddenFrame.Content, Page)

            ' Get each element.
            Dim input As UIElement = TryCast(hiddenPage.FindName("Input"), UIElement)
            Dim output As UIElement = TryCast(hiddenPage.FindName("Output"), UIElement)

            If input Is Nothing Then
                ' Malformed input section.
                NotifyUser(String.Format("Cannot load scenario input section for {0}.  Make sure root of input section markup has x:Name of 'Input'", scenarioClass.Name), NotifyType.ErrorMessage)
                Exit Sub
            End If

            If output Is Nothing Then
                ' Malformed output section.
                NotifyUser(String.Format("Cannot load scenario output section for {0}.  Make sure root of output section markup has x:Name of 'Output'", scenarioClass.Name), NotifyType.ErrorMessage)
                Exit Sub
            End If

            ' Find the LayoutRoot which parents the input and output sections in the main page.
            Dim panel As Panel = TryCast(hiddenPage.FindName("LayoutRoot"), Panel)

            If panel IsNot Nothing Then
                ' Get rid of the content that is currently in the intput and output sections.
                panel.Children.Remove(input)
                panel.Children.Remove(output)

                ' Populate the input and output sections with the newly loaded content.
                InputSection.Content = input
                OutputSection.Content = output

                RaiseEvent ScenarioLoaded(Me, New EventArgs())
            Else
                ' Malformed Scenario file.
                NotifyUser(String.Format("Cannot load scenario: '{0}'.  Make sure root tag in the '{0}' file has an x:Name of 'LayoutRoot'", scenarioClass.Name), NotifyType.ErrorMessage)
            End If
        End Sub

        Private Sub Scenarios_SelectionChanged(sender As Object, e As SelectionChangedEventArgs)
            If Scenarios.SelectedItem IsNot Nothing Then
                NotifyUser("", NotifyType.StatusMessage)

                Dim selectedListBoxItem As ListBoxItem = TryCast(Scenarios.SelectedItem, ListBoxItem)
                SuspensionManager.SessionState("SelectedScenarioIndex") = Scenarios.SelectedIndex

                Dim scenario As Scenario = TryCast(selectedListBoxItem.Content, Scenario)
                LoadScenario(scenario.ClassType)
                InvalidateSize()
            End If
        End Sub

        Public Sub NotifyUser(strMessage As String, type As NotifyType)
            Select Case type
                ' Use the status message style.
                Case NotifyType.StatusMessage
                    StatusBlock.Style = TryCast(Resources("StatusStyle"), Style)
                    Exit Select
                    ' Use the error message style.
                Case NotifyType.ErrorMessage
                    StatusBlock.Style = TryCast(Resources("ErrorStyle"), Style)
                    Exit Select
            End Select
            StatusBlock.Text = strMessage

            ' Collapse the StatusBlock if it has no text to conserve real estate.
            If StatusBlock.Text <> String.Empty Then
                StatusBlock.Visibility = Windows.UI.Xaml.Visibility.Visible
            Else
                StatusBlock.Visibility = Windows.UI.Xaml.Visibility.Collapsed
            End If
        End Sub

        Private Async Sub Footer_Click(sender As Object, e As RoutedEventArgs)
            Await Windows.System.Launcher.LaunchUriAsync(New Uri(DirectCast(sender, HyperlinkButton).Tag.ToString()))
        End Sub

        Private Sub SetFeatureName(str As String)
            FeatureName.Text = str
        End Sub
    End Class

    Public Class MainPageSizeChangedEventArgs
        Inherits EventArgs
        Private m_viewState As ApplicationViewState

        Public Property ViewState() As ApplicationViewState
            Get
                Return m_viewState
            End Get
            Set(value As ApplicationViewState)
                m_viewState = value
            End Set
        End Property
    End Class

    Public Enum NotifyType
        StatusMessage
        ErrorMessage
    End Enum
End Namespace

