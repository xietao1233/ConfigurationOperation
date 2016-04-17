//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "IniConfig.h"

namespace ConfigFile
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

		property ConfigOperation::CIniConfig^ appConfigFile;

	protected:
		void OpenConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	private:
		void GetConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SetConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
