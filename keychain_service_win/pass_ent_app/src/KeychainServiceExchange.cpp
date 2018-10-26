#include <C:\array-io-keychain\keychain_service_win\pass_ent_app\include\KeychainServiceExchange.h>

KeychainServiceExchange::KeychainServiceExchange() 
{

}

KeychainServiceExchange::~KeychainServiceExchange()
{

}

void KeychainServiceExchange::EncodeError(const std::wstring &mes, unsigned short len)
{
	_sendToPipe(L"empty_password", std::wcslen(L"empty_password"));
};

void KeychainServiceExchange::EncodeSuccess(const std::wstring &mes, unsigned short len)
{
	_sendToPipe(mes, len);
};

bool KeychainServiceExchange::Decode(std::string &outString, int &result)
{
	return true;
};

void KeychainServiceExchange::_sendToPipe(const std::wstring &mes, unsigned short count)
{
	HANDLE hPipe;
	DWORD dwWritten;
	int len;
	hPipe = CreateFile(TEXT("\\\\.\\pipe\\keychainpass"),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	DWORD error = GetLastError();
	len = WideCharToMultiByte(CP_ACP, 0, mes.c_str(), count, 0, 0, 0, 0);
	std::string testStr(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, mes.c_str(), count, &testStr[0], len, 0, 0);
	DATA_BLOB DataIn;
	DATA_BLOB DataOut;


	DataIn.pbData = (BYTE*)testStr.c_str();
	DataIn.cbData = len;

	if (CryptProtectData(
		&DataIn,
		L"This is the description string.", // A description string to be included with the encrypted data. 
		NULL,                               // Optional entropy not used.
		NULL,                               // Reserved.
		NULL,                               // Pass NULL for the prompt structure.
		CRYPTPROTECT_LOCAL_MACHINE,
		&DataOut))
	{
		printf("The encryption phase worked.\n");
	}
	else
	{
		printf("Encryption error using CryptProtectData.\n");
	}


	if (hPipe != INVALID_HANDLE_VALUE)
	{
		WriteFile(hPipe,
			DataOut.pbData,//testStr.c_str(),
			DataOut.cbData + 1,//len+1,   // = length of string + terminating '\0' !!!
			&dwWritten,
			NULL);
	}
	error = GetLastError();
	CloseHandle(hPipe);
	testStr.~basic_string();
};