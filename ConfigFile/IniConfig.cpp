#include "pch.h"
#include "IniConfig.h"


using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;


ConfigOperation::KeyValue::KeyValue(Platform::String^ strKeyValueContent)
{
	std::wstring strContent(strKeyValueContent->Data());
	int iStart = 0;

	iStart = strContent.find(L"=");
	if (iStart != -1)
	{
		Key = ref new String(strContent.substr(0, iStart).c_str());
		Value = ref new String(strContent.substr(iStart + 1).c_str());
	}
}

bool ConfigOperation::KeyValue::HasContent()
{
	return !Key->IsEmpty() & !Value->IsEmpty();
}

ConfigOperation::CIniConfig::CIniConfig()
{

}

void ConfigOperation::CIniConfig::SetAndLoadConfigFile(Platform::String^ strFilePath)
{
	StorageIniFilePath = strFilePath;
	LoadConfigFile().then([this](String^ strConfigContent) {
		std::map<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^> mapIniSections;
		std::wstring strContent(strConfigContent->Data());
		String^ strAppName;
		String^ strSection;
		int iStart = 0, iEnd = 0;

		//1. Find first App section name
		if (strContent.at(0) == '[')
		{
			iStart = 1;
			iEnd = strContent.find(L"]", iStart);
			if (iEnd == -1)
			{
				throw ref new Platform::Exception(0x80000001, L"Ini config file error! ");
				return;
			}
			strAppName = ref new String(strContent.substr(iStart, iEnd - iStart).c_str());
			iStart = iEnd + 1;

			//2. Find next app section name
			iEnd = strContent.find(L"\r\n[", iStart);
			if (iEnd != -1)
			{
				//succeed to find next app section name
				strSection = ref new String(strContent.substr(iStart, iEnd - iStart).c_str());
			}
			else
			{
				//Failed to find
				strSection = ref new String(strContent.substr(iStart).c_str());
			}
			auto mapKeyValue = ParseSection(strSection);
			if (!strAppName->IsEmpty() && mapKeyValue->Size)
				mapIniSections.insert(std::make_pair(strAppName, mapKeyValue));
		}

		while (iEnd != -1)
		{
			//1. find app section name
			iStart = strContent.find(L"[", iEnd);
			if (iStart == -1)
				break;
			iStart++;
			iEnd = strContent.find(L"]", iStart);
			if (iEnd == -1)
				break;
			strAppName = ref new String(strContent.substr(iStart, iEnd - iStart).c_str());
			iStart = iEnd + 1;

			//2. find app section end
			iEnd = strContent.find(L"\r\n[", iStart);
			if (iEnd != -1)
			{
				//succeed to find
				strSection = ref new String(strContent.substr(iStart, iEnd - iStart).c_str());
			}
			else
			{
				//failed to find
				strSection = ref new String(strContent.substr(iStart).c_str());
			}
			auto mapKeyValue = ParseSection(strSection);
			if (!strAppName->IsEmpty() && mapKeyValue->Size)
				mapIniSections.insert(std::make_pair(strAppName, mapKeyValue));
		}

		if (mapIniSections.size())
			IniAppMap = ref new Platform::Collections::Map<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>(std::move(mapIniSections));
	});
}

void ConfigOperation::CIniConfig::SaveAsConfigurationFile()
{
	GetSaveAsLocation().then([this](Windows::Storage::StorageFile^ curStorageFile) {
		StoreConfigData(curStorageFile);
	});
}

void ConfigOperation::CIniConfig::SaveConfigurationFile()
{
	Concurrency::create_task(Windows::Storage::StorageFile::GetFileFromApplicationUriAsync(ref new Windows::Foundation::Uri(StorageIniFilePath))).then([this](Windows::Storage::StorageFile^ saveFile) {
		Concurrency::create_task(saveFile->GetParentAsync()).then([this](Windows::Storage::StorageFolder^ storageFolder) {
			auto GetFileName = [](Platform::String^ strFilePath) ->String^{
				Platform::String^ strResult = nullptr;
				std::wstring strPath(strFilePath->Data());
				//int iStart = strPath.find_first_of('/');
				LPWSTR pChar = (LPWSTR)wcsrchr(strPath.c_str(), '/');

				if (pChar)
				{
					strResult = ref new Platform::String(pChar + 1);
				}

				return strResult;
			};
			Platform::String^ strFileName = GetFileName(StorageIniFilePath);
			if (!strFileName->IsEmpty())
			{
				Concurrency::create_task(storageFolder->CreateFileAsync(strFileName, Windows::Storage::CreationCollisionOption::ReplaceExisting)).then([this](Windows::Storage::StorageFile^ saveFile) {
					StoreConfigData(saveFile);
				});
			}
		});
		//StoreConfigData(saveFile);
	});
}

Platform::String^ ConfigOperation::CIniConfig::GetValue(Platform::String^ strAppName, Platform::String^ strKeyName)
{
	Platform::String^ strResult = nullptr;
	auto mapIniApp = safe_cast<Platform::Collections::Map<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>^>(IniAppMap);

	if (mapIniApp->HasKey(strAppName))
	{
		auto itemKeyValue = mapIniApp->Lookup(strAppName);
		if (itemKeyValue->HasKey(strKeyName))
		{
			strResult = itemKeyValue->Lookup(strKeyName);
		}
	}

	return strResult;
}

void ConfigOperation::CIniConfig::SetValue(Platform::String^ strAppName, Platform::String^ strKeyName, Platform::String^ strValue)
{
	auto mapIniApp = safe_cast<Platform::Collections::Map<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>^>(IniAppMap);

	if (mapIniApp->HasKey(strAppName))
	{
		auto itemKeyValue = mapIniApp->Lookup(strAppName);
		itemKeyValue->Insert(strKeyName, strValue);
	}
	else
	{
		auto itemKeyValue = ref new Platform::Collections::Map<Platform::String^, Platform::String^>;
		itemKeyValue->Insert(strKeyName, strValue);
		mapIniApp->Insert(strAppName, itemKeyValue);
	}
}

Concurrency::task<bool> ConfigOperation::CIniConfig::StoreConfigData(Windows::Storage::StorageFile^ curStorageFile)
{
	return Concurrency::create_task([this, curStorageFile]() ->bool {
		bool bResult = false;

		if(curStorageFile)
		{
			Platform::String^ strConfigContent = ref new Platform::String();
			Platform::String^ strSection;
// 			Platform::Collections::Map<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>^ mapIniApp = safe_cast<Platform::Collections::Map<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>^>(IniAppMap);
			auto mapIniApp = safe_cast<Platform::Collections::Map<Platform::String^, Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^>^>(IniAppMap);

			for each (auto varMapItem in mapIniApp)
			{
// 				Platform::Collections::Map<Platform::String^, Platform::String^>^ mapKeyValue = safe_cast<Platform::Collections::Map<Platform::String^, Platform::String^>^>(varMapItem->Value);
				auto mapKeyValue = safe_cast<Platform::Collections::Map<Platform::String^, Platform::String^>^>(varMapItem->Value);

				//convert section data to string
				strSection = ConstructSection(std::make_pair(varMapItem->Key, mapKeyValue));
				if (!strSection->IsEmpty())
					strConfigContent += strSection;
			}

			Windows::Storage::FileIO::WriteTextAsync(curStorageFile, strConfigContent);			//write configuration string to file
			bResult = true;
		}

		return bResult;
	});
}

ConfigOperation::CIniConfig::CIniConfig(Platform::String^ strIniFile)
{
	SetAndLoadConfigFile(strIniFile);
}

Concurrency::task<Platform::String^> ConfigOperation::CIniConfig::LoadConfigFile()
{
	return Concurrency::create_task(Windows::Storage::StorageFile::GetFileFromApplicationUriAsync(ref new Windows::Foundation::Uri(StorageIniFilePath))).then([this](Windows::Storage::StorageFile^ myFile) {
		return Concurrency::create_task(Windows::Storage::FileIO::ReadTextAsync(myFile));
	});
}

Concurrency::task<Windows::Storage::StorageFile^> ConfigOperation::CIniConfig::GetSaveAsLocation()
{
	Windows::Storage::Pickers::FileSavePicker^ myPicker = ref new Windows::Storage::Pickers::FileSavePicker();
	Platform::Collections::Vector<String^>^ listFileChoices = ref new Platform::Collections::Vector<String^>;

	listFileChoices->Append(L".ini");
	myPicker->FileTypeChoices->Insert(L"Configuation File", listFileChoices);
	myPicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::ComputerFolder;
	myPicker->SuggestedFileName = L"MyConfiguration.ini";

	return Concurrency::create_task(myPicker->PickSaveFileAsync());
}

Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^ ConfigOperation::CIniConfig::ParseSection(Platform::String^ strSectionContent)
{
	Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^ result = nullptr;
	std::pair<Platform::String^, Platform::String^> curPair;
	std::map<Platform::String^, Platform::String^> mapContent;
	Platform::String^ strLine = nullptr;
	std::wstring strContent(strSectionContent->Data());
	int iStart = 0, iEnd = 0;

	while (iEnd != -1)
	{
		iStart = strContent.find(L"\r\n", iEnd);
		if (iStart == -1)
			break;
		iStart += 2;
		iEnd = strContent.find(L"\r\n", iStart);
		if (iEnd != -1)
			strLine = ref new String(strContent.substr(iStart, iEnd - iStart).c_str());
		else
			strLine = ref new String(strContent.substr(iStart).c_str());
		curPair = ParseKeyValue(strLine);
		if (curPair.first != nullptr && curPair.second != nullptr)
			mapContent.insert(curPair);
	}

	if (mapContent.size())
		result = ref new Platform::Collections::Map<Platform::String^, Platform::String^>(std::move(mapContent));

	return result;
}

Platform::String^ ConfigOperation::CIniConfig::ConstructSection(std::pair<Platform::String^, Platform::Collections::Map<Platform::String^, Platform::String^>^> pairSection)
{
	Platform::String^ strResult = ref new Platform::String;
	Platform::String^ strItem = nullptr;
	Platform::String^ strKey = pairSection.first;
	auto mapKeyValue = pairSection.second;

	if (!strKey->IsEmpty() && mapKeyValue && mapKeyValue->Size)
		strResult = L"[" + strKey + L"]\r\n";
	for each (auto varKeyValueItem in mapKeyValue)
	{
		strItem = ConstructKeyValue(std::make_pair(varKeyValueItem->Key, varKeyValueItem->Value));
		if (!strItem->IsEmpty())
			strResult += strItem;
	}

	return strResult;
}

std::pair<Platform::String^, Platform::String^> ConfigOperation::CIniConfig::ParseKeyValue(Platform::String^ strKeyValueContent)
{
	std::pair<Platform::String^, Platform::String^> result;
	std::wstring strContent(strKeyValueContent->Data());
	String^ Key = nullptr;
	String^ Value = nullptr;
	int iStart = 0;

	iStart = strContent.find(L"=");
	if (iStart != -1)
	{
		Key = ref new String(strContent.substr(0, iStart).c_str());
		Value = ref new String(strContent.substr(iStart + 1).c_str());
		result = std::make_pair(Key, Value);
	}

	return result;
}

Platform::String^ ConfigOperation::CIniConfig::ConstructKeyValue(std::pair<Platform::String^, Platform::String^> pairKeyValueItem)
{
	Platform::String^ strResult = nullptr;
	Platform::String^ strKey = pairKeyValueItem.first;
	Platform::String^ strValue = pairKeyValueItem.second;

	strResult = strKey + L"=" + strValue + L"\r\n";

	return strResult;
}
