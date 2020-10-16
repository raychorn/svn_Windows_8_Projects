﻿// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using SDKTemplateCS;
using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace PushAndPeriodicNotificationsSampleCS
{
	public sealed partial class ScenarioOutput5 : Page
	{
		// A pointer back to the main page which is used to gain access to the input and output frames and their content.
		MainPage rootPage = null;

		public ScenarioOutput5()
		{
			InitializeComponent();

		}

		#region Template-Related Code - Do not remove
		protected override void OnNavigatedTo(NavigationEventArgs e)
		{
			// Get a pointer to our main page.
			rootPage = e.Parameter as MainPage;

			// We want to be notified with the OutputFrame is loaded so we can get to the content.
			rootPage.InputFrameLoaded += new System.EventHandler(rootPage_InputFrameLoaded);
		}

		protected override void OnNavigatedFrom(NavigationEventArgs e)
		{
			rootPage.InputFrameLoaded -= new System.EventHandler(rootPage_InputFrameLoaded);
		}
		#endregion

		#region Use this code if you need access to elements in the input frame - otherwise delete
		void rootPage_InputFrameLoaded(object sender, object e)
		{
			// At this point, we know that the Input Frame has been loaded and we can go ahead
			// and reference elements in the page contained in the Input Frame.

			// Get a pointer to the content within the IntputFrame.
			Page inputFrame = (Page)rootPage.InputFrame.Content;

			// Go find the elements that we need for this scenario
			// ex: flipView1 = inputFrame.FindName("FlipView1") as FlipView;
		}
		#endregion
	}
}
