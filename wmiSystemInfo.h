#define _WIN32_DCOM
#include <iostream>
#include <string>  
#include <iostream> 
#include <sstream>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
using namespace std;

//------------------------------------------------------
//Konversi BSTR ke STRING (https://stackoverflow.com/questions/6284524/bstr-to-stdstring-stdwstring-and-vice-versa)
string ConvertWCSToMBS(const wchar_t* pstr, long wslen) {
	int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);
	string dblstr(len, '\0');
	len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, &dblstr[0], len, NULL, NULL);

	return dblstr;
}

std::string ConvertBSTRToMBS(BSTR bstr) {
	int wslen = ::SysStringLen(bstr);
	return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}
//------------------------------------------------------

//------------------------------------------------------
//Kode dari: https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer

//[ AMBIL INFORMASI SYSTEM ]
//Variable inputQuery + inputProperty
//Output: String dengan '/' Tiap element
// 
//OS Name
//	"SELECT * FROM Win32_OperatingSystem" + "Name"
// 
//All GPU Name
//	"SELECT * FROM Win32_VideoController" + "Name"
// 
//CPU Name
//	"SELECT * FROM Win32_Processor" + "Name"
// 
//HDD / SSD Capacity
//	"SELECT * FROM Win32_DiskDrive" + "Size"
// 
//RAM Capacity
//	"SELECT * FROM Win32_PhysicalMemory" + "Capacity"
// 
//PC Name
//	"SELECT * FROM Win32_ComputerSystem" + "Name"
// 
//MAC Address
//	"SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled = True" + "MACAddress"


string getSystemInfo(string inputQuery, string inputProperty) {
	string outputString = "";
	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) {
		return "-1";
	}

	hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hres)) {
		CoUninitialize();
		return "-1";
	}

	IWbemLocator* pLoc = NULL;
	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return "-1";
	}

	IWbemServices* pSvc = NULL;
	hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return "-1";
	}

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "-1";
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(inputQuery.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "-1";
	}

	IWbemClassObject* pclsObj = NULL; ULONG uReturn = 0;
	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn) { break; }

		VARIANT vtProp;
		VariantInit(&vtProp);

		//------------------------------------------------------
		//String to LPCWSTR
		//https://www.geeksforgeeks.org/convert-stdstring-to-lpcwstr-in-c/
		wstring temp = wstring(inputProperty.begin(), inputProperty.end());
		LPCWSTR wideString = temp.c_str();
		//------------------------------------------------------

		hr = pclsObj->Get(wideString, 0, &vtProp, 0, 0);
		//wcout << " Response : " << vtProp.bstrVal << endl;
		outputString += ConvertBSTRToMBS(vtProp.bstrVal) + "/";

		VariantClear(&vtProp);
		pclsObj->Release();
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();
	//cout << "String Output: " << outputString;
	return outputString;
}
//------------------------------------------------------