﻿//
// Scenario9.xaml.cpp
// Implementation of the Scenario9 class
//
#include "pch.h"
#include "Scenario9.xaml.h"
#include "Employee.h"

using namespace SDKSample::DataBinding;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

Scenario9::Scenario9()
{
	InitializeComponent();
	_employee = ref new Employee();
	Output->DataContext = _employee;
	_employee->PropertyChanged += ref new PropertyChangedEventHandler(this, &Scenario9::EmployeeChanged);

	ScenarioReset(nullptr,nullptr);
}

void SDKSample::DataBinding::Scenario9::ScenarioReset(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	_employee->Name = "Jane Doe";
	_employee->Organization = "Contoso";

	tbBoundDataModelStatus->Text = "";
}

void Scenario9::EmployeeChanged(Object^ sender, PropertyChangedEventArgs^ e)
{
	tbBoundDataModelStatus->Text = "The property:'" + e->PropertyName + "' was changed";
}

void Scenario9::UpdateDataBtnClick(Object^ sender, PropertyChangedEventArgs^ e)
{
	auto expression = NameTxtBox->GetBindingExpression(TextBox::TextProperty);
	expression->UpdateSource();
}



