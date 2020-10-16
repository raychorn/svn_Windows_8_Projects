//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// MainPage.xaml.cpp
// Implementation of the MainPage.xaml class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "App.xaml.h"

#include <collection.h>

using namespace Platform;
using namespace Clipboard;
using namespace CppSamplesUtils;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::ViewManagement;

MainPage^ MainPage::Current = nullptr;

MainPage::MainPage()
{
    InitializeComponent();

    HiddenFrame = ref new Windows::UI::Xaml::Controls::Frame();
    HiddenFrame->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    LayoutRoot->Children->Append(HiddenFrame);

    SetFeatureName(FEATURE_NAME);

    SizeChanged += ref new SizeChangedEventHandler(this, &MainPage::MainPage_SizeChanged);
    Scenarios->SelectionChanged += ref new SelectionChangedEventHandler(this, &MainPage::Scenarios_SelectionChanged);

    MainPage::Current = this;
}

MainPage::~MainPage()
{
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    PopulateScenarios();
    InvalidateLayout();
}

void MainPage::MainPage_SizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
    InvalidateSize();
    MainPageSizeChangedEventArgs^ args = ref new MainPageSizeChangedEventArgs();
    args->Width = this->ActualWidth;
    MainPageResized(this, args);
}

void MainPage::InvalidateSize()
{
    // Get the window width
    double windowWidth = ActualWidth;

    if (windowWidth != 0.0)
    {
        // Get the width of the ListBox.
        double listBoxWidth = Scenarios->ActualWidth;

        // Is the ListBox using any margins that we need to consider?
        double listBoxMarginLeft = Scenarios->Margin.Left;
        double listBoxMarginRight = Scenarios->Margin.Right;

        // Figure out how much room is left after considering the list box width
        double availableWidth = windowWidth - listBoxWidth;

        // Is the top most child using margins?
        double layoutRootMarginLeft = ContentRoot->Margin.Left;
        double layoutRootMarginRight = ContentRoot->Margin.Right;

        // We have different widths to use depending on the layout
        if (windowWidth >= 768)
        {
            // Make us as big as the the left over space, factoring in the ListBox width, the ListBox margins.
            // and the LayoutRoot's margins
            InputSection->Width = ((availableWidth) - (layoutRootMarginLeft + layoutRootMarginRight + listBoxMarginLeft + listBoxMarginRight));
        }
        else
        {
            // Make us as big as the left over space, factoring in just the LayoutRoot's margins.
                InputSection->Width = (windowWidth - (layoutRootMarginLeft + layoutRootMarginRight));
        }
    }

    InvalidateLayout();
}

void MainPage::InvalidateLayout()
{
    if (this->ActualWidth < 768)
    {
        Grid::SetRow(DescriptionText, 3);
        Grid::SetColumn(DescriptionText, 0);

        Grid::SetRow(InputSection, 4);
        Grid::SetColumn(InputSection, 0);

        Grid::SetRow(FooterPanel, 2);
        Grid::SetColumn(FooterPanel, 0);
    }
    else
    {
        Grid::SetRow(DescriptionText, 1);
        Grid::SetColumn(DescriptionText, 1);

        Grid::SetRow(InputSection, 2);
        Grid::SetColumn(InputSection, 1);

        Grid::SetRow(FooterPanel, 1);
        Grid::SetColumn(FooterPanel, 1);
    }
}

#pragma optimize("", off)
void MainPage::PopulateScenarios()
{
    ScenarioList = ref new Platform::Collections::Vector<Object^>();

    // Populate the ListBox with the list of scenarios as defined in Constants.cpp
    for (unsigned int i = 0; i < scenarios->Length; ++i)
    {
        Scenario s = scenarios[i];
        ListBoxItem^ item = ref new ListBoxItem();
        item->Name = s.ClassName;
        item->Content = (i + 1).ToString() + ") " + s.Title;
        ScenarioList->Append(item);
    }

    // Bind the ListBox to the scenario list.
    Scenarios->ItemsSource = ScenarioList;

    // Starting scenario is the first or based upon a previous selection.
    ListBoxItem^ startingScenario = nullptr;

    // Starting scenario is the first or based upon a previous selection.
    int startingScenarioIndex = -1;

    auto ps = SuspensionManager::SessionState();
    if (ps->HasKey("SelectedScenarioIndex"))
    {
        int selectedScenarioIndex = safe_cast<int>(ps->Lookup("SelectedScenarioIndex"));
        startingScenarioIndex = selectedScenarioIndex;
    }

    Scenarios->SelectedIndex = startingScenarioIndex != -1 ? startingScenarioIndex : 0;
}

#pragma optimize("", on)

void MainPage::LoadScenario(String^ scenarioName)
{

    // Load the ScenarioX.xaml file into the Frame.
    TypeName scenarioType = {scenarioName, TypeKind::Custom};
    HiddenFrame->Navigate(scenarioType, this);

    // Get the top element, the Page, so we can look up the elements
    // that represent the input and output sections of the ScenarioX file.
    Page^ hiddenPage = safe_cast<Page^>(HiddenFrame->Content);

    // Get each element.
    UIElement^ input = safe_cast<UIElement^>(hiddenPage->FindName("Input"));
    UIElement^ output = safe_cast<UIElement^>(hiddenPage->FindName("Output"));

    if (input == nullptr)
    {
        // Malformed input section.
        NotifyUser("Cannot load scenario input section for " + scenarioName +
            " Make sure root of input section markup has x:Name of 'Input'", NotifyType::ErrorMessage);

        return;
    }

    if (output == nullptr)
    {
        // Malformed output section.
        NotifyUser("Cannot load scenario output section for " + scenarioName +
            " Make sure root of output section markup has x:Name of 'Output'", NotifyType::ErrorMessage);

        return;
    }

    // Find the LayoutRoot which parents the input and output sections in the main page.
    Panel^ panel = safe_cast<Panel^>(hiddenPage->FindName("LayoutRoot"));

    if (panel != nullptr)
    {
        unsigned int index = 0;
        UIElementCollection^ collection = panel->Children;

        // Get rid of the content that is currently in the input and output sections.
        collection->IndexOf(input, &index);
        collection->RemoveAt(index);

        collection->IndexOf(output, &index);
        collection->RemoveAt(index);

        // Populate the input and output sections with the newly loaded content.
        InputSection->Content = input;
        OutputSection->Content = output;

        ScenarioLoaded(this, nullptr);
    }
    else
    {
        // Malformed Scenario file.
        NotifyUser("Cannot load scenario: " + scenarioName + ". Make sure root tag in the '" +
            scenarioName + "' file has an x:Name of 'LayoutRoot'", NotifyType::ErrorMessage);
    }
}

void MainPage::Scenarios_SelectionChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
    if (Scenarios->SelectedItem != nullptr)
    {
        NotifyUser("", NotifyType::StatusMessage);

        int selectedListBoxItemIndex = Scenarios->SelectedIndex;
        SuspensionManager::SessionState()->Insert("SelectedScenarioIndex", selectedListBoxItemIndex);

        LoadScenario((safe_cast<ListBoxItem^>(Scenarios->SelectedItem))->Name);
        InvalidateSize();
    }
}

void MainPage::SetFeatureName(String^ strFeature)
{
    Clipboard->Text = strFeature;
}

void MainPage::NotifyUser(String^ strMessage, NotifyType type)
{
    switch (type)
    {
    case NotifyType::StatusMessage:
        {
            // Use the status message style.
            StatusBlock->Style = safe_cast<Windows::UI::Xaml::Style^>(Resources->Lookup("StatusStyle"));
        }
        break;

    case NotifyType::ErrorMessage:
        {
            // Use the error message style.
            StatusBlock->Style = safe_cast<Windows::UI::Xaml::Style^>(Resources->Lookup("ErrorStyle"));
        }
        break;

    default:
        break;
    }

    StatusBlock->Text = strMessage;

    // Collapsed the StatusBlock if it has no text to conserve real estate.
    if (StatusBlock->Text != "")
    {
        StatusBlock->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }
    else
    {
        StatusBlock->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    }
}

void MainPage::NotifyUserBackgroundThread(String^ message, NotifyType type)
{
    Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
        [this, message, type]()
        {
            NotifyUser(message, type);
        }));
}

void MainPage::Footer_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto uri = ref new Uri((String^)((HyperlinkButton^)sender)->Tag);
    Windows::System::Launcher::LaunchUriAsync(uri);
}