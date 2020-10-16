//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using SDKTemplate;

using System;
using System.Collections.Generic;
using Windows.Graphics.Display;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using Windows.UI.Xaml.Media.Imaging;

namespace Scaling
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario1 : SDKTemplate.Common.LayoutAwarePage
    {
        private static readonly Dictionary<ResolutionScale, string> MinDPI = new Dictionary<ResolutionScale, string>
        {
            { ResolutionScale.Invalid,         "Unknown" },
            { ResolutionScale.Scale100Percent, "No minimum DPI for this scale" },
            { ResolutionScale.Scale140Percent, "174 DPI" },
            { ResolutionScale.Scale180Percent, "240 DPI" },
        };

        private static readonly Dictionary<ResolutionScale, string> MinResolution = new Dictionary<ResolutionScale, string>
        {
            { ResolutionScale.Invalid,         "Unknown" },
            { ResolutionScale.Scale100Percent, "1024x768 (min resolution needed to run apps)" },
            { ResolutionScale.Scale140Percent, "1440x1080" },
            { ResolutionScale.Scale180Percent, "1920x1440" },
        };

        private readonly Dictionary<ResolutionScale, BitmapImage> Images;

        public Scenario1()
        {
            this.InitializeComponent();
            DisplayInformation displayInformation = DisplayInformation.GetForCurrentView();
            displayInformation.DpiChanged += DisplayProperties_DpiChanged;
            Images = new Dictionary<ResolutionScale, BitmapImage>
            {
                { ResolutionScale.Invalid,         new BitmapImage(new Uri(BaseUri, "/Assets/projector.scale-100.png")) },
                { ResolutionScale.Scale100Percent, new BitmapImage(new Uri(BaseUri, "/Assets/projector.scale-100.png")) },
                { ResolutionScale.Scale140Percent, new BitmapImage(new Uri(BaseUri, "/Assets/projector.scale-140.png")) },
                { ResolutionScale.Scale180Percent, new BitmapImage(new Uri(BaseUri, "/Assets/projector.scale-180.png")) },
            };
        }

        private void ResetOutput()
        {
            DisplayInformation displayInformation = DisplayInformation.GetForCurrentView();
            ResolutionScale scale = displayInformation.ResolutionScale;
            ScalingText.Text = ((int)scale).ToString() + "%";
            MinDPIText.Text = MinDPI[scale];
            MinScreenResolutionText.Text = MinResolution[scale];
            LogicalDPIText.Text = displayInformation.LogicalDpi.ToString() + " DPI";
            ManualLoadImage.Source = Images[scale];
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            ResetOutput();
        }

        void DisplayProperties_DpiChanged(DisplayInformation sender, object args)
        {
            ResetOutput();
        }
    }
}
