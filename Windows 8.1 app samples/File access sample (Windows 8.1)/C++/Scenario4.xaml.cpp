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
// Scenario4.xaml.cpp
// Implementation of the Scenario4 class
//

#include "pch.h"
#include "Scenario4.xaml.h"

using namespace SDKSample::FileAccess;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

Scenario4::Scenario4()
{
    InitializeComponent();
    rootPage = MainPage::Current;
    rootPage->Initialize();
    rootPage->ValidateFile();
    WriteBytesButton->Click += ref new RoutedEventHandler(this, &Scenario4::WriteBytesButton_Click);
    ReadBytesButton->Click  += ref new RoutedEventHandler(this, &Scenario4::ReadBytesButton_Click);
}

IBuffer^ Scenario4::GetBufferFromString(String^ str)
{
    InMemoryRandomAccessStream^ memoryStream = ref new InMemoryRandomAccessStream();
    DataWriter^ dataWriter = ref new DataWriter(memoryStream);
    dataWriter->WriteString(str);
    return dataWriter->DetachBuffer();
}

void Scenario4::WriteBytesButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    rootPage->ResetScenarioOutput(OutputTextBlock);
    StorageFile^ file = rootPage->SampleFile;
    if (file != nullptr)
    {
        String^ userContent = InputTextBox->Text;
        if (userContent != nullptr && !userContent->IsEmpty())
        {
            IBuffer^ buffer = GetBufferFromString(userContent);
            create_task(FileIO::WriteBufferAsync(file, buffer)).then([this, file, buffer, userContent](task<void> task)
            {
                try
                {
                    task.get();
                    OutputTextBlock->Text = "The following " + buffer->Length.ToString() + " bytes of text were written to '" + file->Name + "':\n\n" + userContent;
                }
                catch (COMException^ ex)
                {
                    rootPage->HandleFileNotFoundException(ex);
                }
            });
        }
        else
        {
            OutputTextBlock->Text = "The text box is empty, please write something and then click 'Write' again.";
        }
    }
    else
    {
        rootPage->NotifyUserFileNotExist();
    }
}

void Scenario4::ReadBytesButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    rootPage->ResetScenarioOutput(OutputTextBlock);
    StorageFile^ file = rootPage->SampleFile;
    if (file != nullptr)
    {
        create_task(FileIO::ReadBufferAsync(file)).then([this, file](task<IBuffer^> task)
        {
            try
            {
                IBuffer^ buffer = task.get();
                DataReader^ dataReader = DataReader::FromBuffer(buffer);
                String^ fileContent = dataReader->ReadString(buffer->Length);
                OutputTextBlock->Text = "The following " + buffer->Length.ToString() + " bytes of text were read from '" + file->Name + "':\n\n" + fileContent;
            }
            catch (COMException^ ex)
            {
                rootPage->HandleFileNotFoundException(ex);
            }
        });
    }
    else
    {
        rootPage->NotifyUserFileNotExist();
    }
}
