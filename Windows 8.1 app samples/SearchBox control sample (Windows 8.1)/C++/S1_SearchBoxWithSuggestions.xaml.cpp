﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S1_SearchBoxWithSuggestions.xaml.cpp
// Implementation of the S1_SearchBoxWithSuggestions class
//

#include "pch.h"
#include "S1_SearchBoxWithSuggestions.xaml.h"
#include "MainPage.xaml.h"

using namespace std; 
using namespace Platform;
using namespace SDKSample;
using namespace SDKSample::SearchControl;
using namespace Windows::ApplicationModel::Search;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;


S1_SearchBoxWithSuggestions::S1_SearchBoxWithSuggestions()
{
    InitializeComponent();
}

void S1_SearchBoxWithSuggestions::OnSearchBoxEventsSuggestionsRequested(Object^ sender, SearchBoxSuggestionsRequestedEventArgs^ e) 
{ 
	// App provided suggestions list
    static wstring suggestionList [] = 
        { 
            L"shanghai", L"istanbul", L"karachi", L"delhi", L"mumbai", L"moscow", L"s�o paulo", L"seoul", L"beijing", L"jakarta", 
            L"tokyo", L"mexico city", L"kinshasa", L"new york city", L"lagos", L"london", L"lima", L"bogota", L"tehran", L"ho chi minh city", 
            L"hong kong", L"bangkok", L"dhaka", L"cairo", L"hanoi", L"rio de janeiro", L"lahore", L"chonquing", L"bengaluru", L"tianjin", 
            L"baghdad", L"riyadh", L"singapore", L"santiago", L"saint petersburg", L"surat", L"chennai", L"kolkata", L"yangon", L"guangzhou", 
            L"alexandria", L"shenyang", L"hyderabad", L"ahmedabad", L"ankara", L"johannesburg", L"wuhan", L"los angeles", L"yokohama", 
            L"abidjan", L"busan", L"cape town", L"durban", L"pune", L"jeddah", L"berlin", L"pyongyang", L"kanpur", L"madrid", L"jaipur", 
            L"nairobi", L"chicago", L"houston", L"philadelphia", L"phoenix", L"san antonio", L"san diego", L"dallas", L"san jose", 
            L"jacksonville", L"indianapolis", L"san francisco", L"austin", L"columbus", L"fort worth", L"charlotte", L"detroit", 
            L"el paso", L"memphis", L"baltimore", L"boston", L"seattle washington", L"nashville", L"denver", L"louisville", L"milwaukee", 
            L"portland", L"las vegas", L"oklahoma city", L"albuquerque", L"tucson", L"fresno", L"sacramento", L"long beach", L"kansas city", 
            L"mesa", L"virginia beach", L"atlanta", L"colorado springs", L"omaha", L"raleigh", L"miami", L"cleveland", L"tulsa", L"oakland", 
            L"minneapolis", L"wichita", L"arlington", L"bakersfield", L"new orleans", L"honolulu", L"anaheim", L"tampa", L"aurora", 
            L"santa ana", L"st. louis", L"pittsburgh", L"corpus christi", L"riverside", L"cincinnati", L"lexington", L"anchorage", 
            L"stockton", L"toledo", L"st. paul", L"newark", L"greensboro", L"buffalo", L"plano", L"lincoln", L"henderson", L"fort wayne", 
            L"jersey city", L"st. petersburg", L"chula vista", L"norfolk", L"orlando", L"chandler", L"laredo", L"madison", L"winston-salem", 
            L"lubbock", L"baton rouge", L"durham", L"garland", L"glendale", L"reno", L"hialeah", L"chesapeake", L"scottsdale", 
            L"north las vegas", L"irving", L"fremont", L"irvine", L"birmingham", L"rochester", L"san bernardino", L"spokane", 
            L"toronto", L"montreal", L"vancouver", L"ottawa-gatineau", L"calgary", L"edmonton", L"quebec city", L"winnipeg", L"hamilton" 
        }; 
 
    auto queryText = e->QueryText; 
    wstring query = wstring(queryText->Data()); 
    // convert query string to lower case. 
    transform(query.begin(), query.end(), query.begin(), tolower); 
    if (queryText->Length() == 0) 
    {
        MainPage::Current->NotifyUser("", NotifyType::StatusMessage); 
    } 
    else 
    { 
        auto request = e->Request; 
        for (const auto& suggestion : suggestionList) 
        { 
            if (suggestion.find(query) == 0) 
            { 
                // if the string starts with the queryText (ignoring case), add suggestion to search pane. 
                request->SearchSuggestionCollection->AppendQuerySuggestion(ref new String(suggestion.c_str())); 
            } 
 
        } 
 
        if (request->SearchSuggestionCollection->Size > 0) 
        { 
            MainPage::Current->NotifyUser("Suggestions provided for query: " + queryText, NotifyType::StatusMessage); 
        } 
        else 
        { 
            MainPage::Current->NotifyUser("No suggestions provided for query: " + queryText, NotifyType::StatusMessage); 
        } 
    } 
}

void S1_SearchBoxWithSuggestions::OnSearchBoxEventsQuerySubmitted(Object^ sender, SearchBoxQuerySubmittedEventArgs^ e) 
{
	MainPage::Current->NotifyUser(e->QueryText, NotifyType::StatusMessage);
}