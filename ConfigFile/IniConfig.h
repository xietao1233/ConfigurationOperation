#pragma once

namespace ConfigOperation
{
	
	public ref class KeyValue sealed
	{
	public:
		KeyValue() {};
		KeyValue(Platform::String^ strKeyValueContent);

		property Platform::String^ Key;
		property Platform::String^ Value;

		bool HasContent();
	};

	public ref class CIniConfig sealed
	{
	public:
		CIniConfig();
		CIniConfig(Platform::String^ strIniFile);

// 		property Windows::Foundation::Collections::IMap<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>^ IniAppMap;
		void SetAndLoadConfigFile(Platform::String^ strFilePath);
		void SaveAsConfigurationFile();
		void SaveConfigurationFile();
		Platform::String^ GetValue(Platform::String^ strAppName, Platform::String^ strKeyName);
		void SetValue(Platform::String^ strAppName, Platform::String^ strKeyName, Platform::String^ strValue);

	private:
		Windows::Foundation::Collections::IMap<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>^ IniAppMap;
		Platform::String^ StorageIniFilePath;

		Concurrency::task<Platform::String^> LoadConfigFile();			//load configuration file to map
		Concurrency::task<bool> StoreConfigData(Windows::Storage::StorageFile^ curStorageFile);
		Concurrency::task<Windows::Storage::StorageFile^> GetSaveAsLocation();						//save map data to configuration file
		Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^ ParseSection(Platform::String^ strSectionContent);
		Platform::String^ ConstructSection(std::pair<Platform::String^, Platform::Collections::Map<Platform::String^, Platform::String^>^> pairSection);
		std::pair<Platform::String^, Platform::String^> ParseKeyValue(Platform::String^ strKeyValueContent);
		Platform::String^ ConstructKeyValue(std::pair<Platform::String^, Platform::String^> pairKeyValueItem);
	};

}