//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using SDKTemplate;
using System;
using System.Threading;
using Windows.Devices.Enumeration;
using Windows.Devices.SmartCards;
using Windows.Foundation;
using Windows.Security.Cryptography;
using Windows.Security.Cryptography.Core;
using Windows.Storage.Streams;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace SmartCardSample
{

/// <summary>
/// An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Scenario1 : SDKTemplate.Common.LayoutAwarePage
{
    // A pointer back to the main page.  This is needed if you want to call
    // methods in MainPage such as NotifyUser()
    MainPage rootPage = MainPage.Current;

    private const String pinPolicyDisallowed = "Disallowed";
    private const String pinPolicyAllowed = "Allowed";
    private const String pinPolicyRequireOne = "Require At Least One";

    private SmartCardReader reader = null;
    private SynchronizationContext uiContext = null;

    public Scenario1()
    {
        this.InitializeComponent();

        // Since this class is a page, it will be instantiated on the UI
        // thread.  We need to keep a reference to the UI thread's
        // synchronization context as a member so that we can access it
        // from other threads for event handlers.  See the
        // HandleCardAdded method for more information.
        uiContext = SynchronizationContext.Current;
    }

    /// <summary>
    /// Invoked when this page is about to be displayed in a Frame.
    /// </summary>
    /// <param name="e">Event data that describes how this page was reached.
    /// The parameter property is typically used to configure the page.</param>
    protected override void OnNavigatedTo(NavigationEventArgs e)
    {
    }

    /// <summary>
    /// Click handler for the 'create' button. 
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    private async void Create_Click(object sender, RoutedEventArgs e)
    {
        Button b = sender as Button;
        b.IsEnabled = false;
        rootPage.NotifyUser("Creating TPM virtual smart card...",
                            NotifyType.StatusMessage);

        try
        {
            SmartCardPinPolicy pinPolicy = ParsePinPolicy();

            IBuffer adminkey = CryptographicBuffer.GenerateRandom(
                MainPage.ADMIN_KEY_LENGTH_IN_BYTES);

            SmartCardProvisioning provisioning = await
                SmartCardProvisioning.RequestVirtualSmartCardCreationAsync(
                    FriendlyNameText.Text,
                    adminkey,
                    pinPolicy);

            // If card creation is cancelled by the user,
            // RequestVirtualSmartCard will return null instead of a
            // SmartCardProvisioning object.
            if (null == provisioning)
            {
                rootPage.NotifyUser(
                    "TPM virtual smart card creation was canceled by " +
                    "the user.",
                    NotifyType.StatusMessage);
                b.IsEnabled = true;
                return;
            }

            // The following two lines are not directly related to TPM virtual
            // smart card creation, but are used to demonstrate how to handle
            // CardAdded events by registering an event handler with a
            // SmartCardReader object.  Since we are using a TPM virtual smart
            // card in this case, the card cannot actually be added to or
            // removed from the reader, but a CardAdded event will fire as
            // soon as the event handler is added, since the card is already
            // inserted.
            //
            // We must retain a reference to the SmartCardReader object to
            // which we are adding the event handler.  Hence we assign the
            // reader object associated with the SmartCardProvisioning we
            // received from RequestVirtualSmartCardCreationAsync to the
            // class's "reader" member.  Then, we use += to add the 
            // HandleCardAdded method as an event handler.  The function
            // will be automatically boxed in a TypedEventHandler, but
            // the function signature match the template arguments for
            // the specific event - in this case,
            // <SmartCardReader, CardAddedEventArgs>
            reader = provisioning.SmartCard.Reader;
            reader.CardAdded += HandleCardAdded;

            // Store the reader's device ID and admin key to enable the
            // following scenarios in the sample.
            rootPage.SmartCardReaderDeviceId =
                provisioning.SmartCard.Reader.DeviceId;
            rootPage.AdminKey = adminkey;

            // Once RequestVirtualSmartCardCreationAsync has returned, the card
            // is already provisioned and ready to use.  Therefore, the steps
            // in this using block are not actually necessary at this point.
            // However, should you want to re-provision the card in the future,
            // you would follow this sequence: acquire a challenge context,
            // calculate a response, and then call ProvisionAsync on the
            // challenge context with the calculated response.
            using (var context = await provisioning.GetChallengeContextAsync())
            {
                IBuffer response =
                    ChallengeResponseAlgorithm.CalculateResponse(
                        context.Challenge,
                        adminkey);
                await context.ProvisionAsync(response, true);
            }

            rootPage.NotifyUser(
                "TPM virtual smart card is provisioned and ready for use.",
                NotifyType.StatusMessage);
        }
        catch (Exception ex)
        {
            rootPage.NotifyUser(
                "TPM virtual smart card creation failed with exception: " +
                ex.ToString(),
                NotifyType.ErrorMessage);
        }
        finally
        {
            b.IsEnabled = true;
        }
    }

    void HandleCardAdded(SmartCardReader sender, CardAddedEventArgs args)
    {
        // This event handler will not be invoked on the UI thread.  Hence,
        // to perform UI operations we need to post a lambda to be executed
        // back on the UI thread; otherwise we may access objects which
        // are not marshalled for the current thread, which will result in an
        // exception due to RPC_E_WRONG_THREAD.
        uiContext.Post((object ignore) =>
        {
            rootPage.NotifyUser(
                "Card added to reader " + reader.Name + ".",
                NotifyType.StatusMessage);
        }, null);
    }

    private SmartCardPinPolicy ParsePinPolicy()
    {
        SmartCardPinPolicy pinPolicy = new SmartCardPinPolicy();
        pinPolicy.MinLength = uint.Parse(PinMinLength.Text);
        pinPolicy.MaxLength = uint.Parse(PinMaxLength.Text);

        switch (PinUppercase.SelectedValue.ToString())
        {
            case pinPolicyDisallowed:
                pinPolicy.UppercaseLetters =
                    SmartCardPinCharacterPolicyOption.Disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.UppercaseLetters =
                    SmartCardPinCharacterPolicyOption.Allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.UppercaseLetters =
                    SmartCardPinCharacterPolicyOption.RequireAtLeastOne;
                break;
            default:
                throw new InvalidOperationException();
        }

        switch (PinLowercase.SelectedValue.ToString())
        {
            case pinPolicyDisallowed:
                pinPolicy.LowercaseLetters =
                    SmartCardPinCharacterPolicyOption.Disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.LowercaseLetters =
                    SmartCardPinCharacterPolicyOption.Allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.LowercaseLetters =
                    SmartCardPinCharacterPolicyOption.RequireAtLeastOne;
                break;
            default:
                throw new InvalidOperationException();
        }

        switch (PinDigits.SelectedValue.ToString())
        {
            case pinPolicyDisallowed:
                pinPolicy.Digits =
                    SmartCardPinCharacterPolicyOption.Disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.Digits =
                    SmartCardPinCharacterPolicyOption.Allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.Digits =
                    SmartCardPinCharacterPolicyOption.RequireAtLeastOne;
                break;
            default:
                throw new InvalidOperationException();
        }

        switch (PinSpecial.SelectedValue.ToString())
        {
            case pinPolicyDisallowed:
                pinPolicy.SpecialCharacters =
                    SmartCardPinCharacterPolicyOption.Disallow;
                break;
            case pinPolicyAllowed:
                pinPolicy.SpecialCharacters =
                    SmartCardPinCharacterPolicyOption.Allow;
                break;
            case pinPolicyRequireOne:
                pinPolicy.SpecialCharacters =
                    SmartCardPinCharacterPolicyOption.RequireAtLeastOne;
                break;
            default:
                throw new InvalidOperationException();
        }

        return pinPolicy;
    }

    }

}
