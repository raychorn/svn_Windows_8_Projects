//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once

namespace SDKSample
{
    public value struct Scenario
    {
        Platform::String^ Title;
        Platform::String^ ClassName;
    };

    partial ref class MainPage
    {
    internal:
        SDKSample::Projection::ViewLifetimeControl^ ProjectionViewPageControl;

        static property Platform::String^ FEATURE_NAME 
        {
            Platform::String^ get() 
            {  
                return "Projection"; 
            }
        }

        static property Platform::Array<Scenario>^ scenarios 
        {
            Platform::Array<Scenario>^ get() 
            { 
                return scenariosInner; 
            }
        }
    private:
        static Platform::Array<Scenario>^ scenariosInner;
    };

    namespace Projection
    {
        // Sample specific types should be in this namespace
    }
}
