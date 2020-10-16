﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using SDKTemplate;
using System;
using Windows.ApplicationModel;

namespace PackageSample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario1 : SDKTemplate.Common.LayoutAwarePage
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        public Scenario1()
        {
            this.InitializeComponent();
        }

        String versionString(PackageVersion version)
        {
            return String.Format("{0}.{1}.{2}.{3}",
                                 version.Major, version.Minor, version.Build, version.Revision);
        }

        String architectureString(Windows.System.ProcessorArchitecture architecture)
        {
            switch (architecture)
            {
                case Windows.System.ProcessorArchitecture.X86:
                    return "x86";
                case Windows.System.ProcessorArchitecture.Arm:
                    return "arm";
                case Windows.System.ProcessorArchitecture.X64:
                    return "x64";
                case Windows.System.ProcessorArchitecture.Neutral:
                    return "neutral";
                case Windows.System.ProcessorArchitecture.Unknown:
                    return "unknown";
                default:
                    return "???";
            }
        }

        void GetPackage_Click(Object sender, RoutedEventArgs e)
        {
            Package package = Package.Current;
            PackageId packageId = package.Id;

            OutputTextBlock.Text = String.Format("Name: \"{0}\"\n" +
                                                 "Version: {1}\n" +
                                                 "Architecture: {2}\n" +
                                                 "ResourceId: \"{3}\"\n" +
                                                 "Publisher: \"{4}\"\n" +
                                                 "PublisherId: \"{5}\"\n" +
                                                 "FullName: \"{6}\"\n" +
                                                 "FamilyName: \"{7}\"\n" +
                                                 "IsFramework: {8}\n" +
                                                 "IsResourcePackage: {9}\n" +
                                                 "IsBundle: {10}\n" +
                                                 "IsDevelopmentMode: {11}\n" +
                                                 "DisplayName: \"{12}\"\n" +
                                                 "PublisherDisplayName: \"{13}\"\n" +
                                                 "Description: \"{14}\"\n" +
                                                 "Logo: \"{15}\"\n",
                                                 packageId.Name,
                                                 versionString(packageId.Version),
                                                 architectureString(packageId.Architecture),
                                                 packageId.ResourceId,
                                                 packageId.Publisher,
                                                 packageId.PublisherId,
                                                 packageId.FullName,
                                                 packageId.FamilyName,
                                                 package.IsFramework,
                                                 package.IsResourcePackage,
                                                 package.IsBundle,
                                                 package.IsDevelopmentMode,
                                                 package.DisplayName,
                                                 package.PublisherDisplayName,
                                                 package.Description,
                                                 package.Logo.AbsoluteUri);
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }
    }
}