#include "SecurityManager.h"
#include <ServiceLogger.h>

using namespace std;

static INT_PTR CALLBACK PasswordProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

SecurityManager::SecurityManager() {

}

SecurityManager::~SecurityManager() {

}

void SecurityManager::CreateSecureDesktop(const std::string& transId) {
	HINSTANCE hInst = GetModuleHandle(NULL);
	TCHAR curDirectory[500];
	GetCurrentDirectory(500, curDirectory);
	LPCWSTR commandLine = GetCommandLine();
	std::wstring rem(commandLine);
	std::string::size_type fSymb = rem.find('\"');
	while (fSymb != std::string::npos) {
		rem.erase(fSymb, 1);
		fSymb = rem.find('\"');
	}
	wchar_t* found = wcsstr((wchar_t*)rem.c_str(), L"keychain_service_win.exe");
	wchar_t dst[400] = { 0 };
	wchar_t args[400] = { 0 };
	wcsncpy_s(dst, 400, rem.c_str(), (found - (wchar_t*)rem.c_str()));
	wcscat_s(dst, 400, L"keychain_pass_entry_app.exe");
	wcscat_s(args, 400, L"-transId ");
	std::wstring _tId(400, L'#');
	size_t outSize;
	mbstowcs_s(&outSize, &_tId[0], 400, transId.c_str(), 400);
	wcscat_s(args, 400, _tId.c_str());
	LPCWSTR appToStart = dst;
	LPTSTR app_args = args;
	ServiceLogger::getLogger().Log("CreateSecureDescktop function StartInteractiveClientProcess to enter credentials");
	if (!StartInteractiveClientProcess(appToStart, (LPTSTR)app_args))
	{
		throw std::runtime_error("Could not create child client process");
	}
}
