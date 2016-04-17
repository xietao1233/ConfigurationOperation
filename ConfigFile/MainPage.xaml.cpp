//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "IniConfig.h"

using namespace ConfigFile;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
	appConfigFile = ref new ConfigOperation::CIniConfig();
}


void ConfigFile::MainPage::OpenConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	appConfigFile->SetAndLoadConfigFile(ref new String(L"ms-appx:///Assets/Config.ini"));
}


void ConfigFile::MainPage::SaveConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//appConfigFile->SaveAsConfigurationFile();	
	appConfigFile->SaveConfigurationFile();
}


void ConfigFile::MainPage::GetConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	String^ strValue = appConfigFile->GetValue(ref new String(L"Display"), ref new String(L"fMeshLODLevel2FadeDist"));
	Windows::UI::Popups::MessageDialog msg(strValue);
	msg.ShowAsync();
}


void ConfigFile::MainPage::SetConfigurationButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	appConfigFile->SetValue(ref new String(L"Display"), ref new String(L"fMeshLODLevel2FadeDist"), ref new String(L"0x12345678"));
}
