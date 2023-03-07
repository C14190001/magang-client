#include <iostream>
#include <winsock.h>
#include <string>
#include <ctime>
#include <fstream>
#include"wmiSystemInfo.h"
#include"ipconfigScript.h"
using namespace std;

#define bufferSize 768
int nClientSocket;
struct sockaddr_in srv;

void viewSpecs() {
	string substring = "";
	bool igpu = false; int b = 0; double c = 0;
	//PC Name
	substring = getSystemInfo("SELECT * FROM Win32_ComputerSystem", "Name");
	for (int i = 0; i < substring.length(); i++) {
		if (substring[i] == '/') {
			cout << "PC Name: " << substring.substr(0, i) << endl;
			break;
		}
	}
	//CPU
	substring = getSystemInfo("SELECT * FROM Win32_Processor", "Name");
	for (int i = 0; i < substring.length(); i++) {
		if (substring[i] == '/') {
			cout << "CPU: " << substring.substr(0, i) << endl;
			break;
		}
	}
	//GPU
	substring = getSystemInfo("SELECT * FROM Win32_VideoController", "Name");
	for (int i = 0; i < substring.length(); i++) {
		if (!igpu) {
			if (substring[i] == '/') {
				if (i == substring.length() - 1) {
					cout << "Intergrated GPU: " << substring.substr(b + 1, i - 1 - b) << endl;
					break;
				}
				else {
					cout << "Dedicated GPU: " << substring.substr(0, i) << endl;
					b = i;
					igpu = true;
				}
			}
		}
		else {
			if (substring[i] == '/') {
				cout << "Intergrated GPU: " << substring.substr(b + 1, i - 1 - b) << endl;
				break;
			}
		}
	}
	//RAM
	substring = getSystemInfo("SELECT * FROM Win32_PhysicalMemory", "Capacity");
	b = 0;
	for (int i = 0; i < substring.length(); i++) {
		if (substring[i] == '/') {
			c += stod(substring.substr(b, i));
			b = i + 1;
		}
	}
	cout << "RAM: " << int(c / 1073741824) << " GB\n";
	//HDD
	substring = getSystemInfo("SELECT * FROM Win32_DiskDrive", "Size");
	b = 0; c = 0;
	for (int i = 0; i < substring.length(); i++) {
		if (substring[i] == '/') {
			c += stod(substring.substr(b, i));
			b = i + 1;
		}
	}
	cout << "HDD: " << int(c / 1073741824) << " GB\n";
	//IP Address
	substring = getIPv4Address();
	cout << "IP Address: " << substring << "\n";
	//MAC Address
	substring = getSystemInfo("SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled = True", "MACAddress");
	for (int i = 0; i < substring.length(); i++) {
		if (substring[i] == '/') {
			cout << "MAC Address: " << substring.substr(0, i) << endl;
			break;
		}
	}
}

int exitFail() {
	WSACleanup();
	system("PAUSE");
	exit(EXIT_FAILURE);
	return 0;
}

int main() {
	//Initialization --------------------------
	srand(time(0));
	int nRet = 0, result = 0;
	string SrvIP, ID, PORT;
	bool connected = false;
	//1. Read ClientSettings.txt --------------
	cout << "[INFO] Reading ClientSettings.txt...\n";
	ifstream in("ClientSettings.txt");
	in >> SrvIP >> PORT >> ID;
	in.close();
	if (SrvIP == "" || PORT == "" || ID == "") {
		cout << "[INFO] ClientSettings.txt not found.\n";
		cout << "[SETUP] Input Server IP: ";
		getline(cin, SrvIP);
		cout << "[SETUP] Input Connection Port: ";
		getline(cin, PORT);
		cout << "[SETUP] Input Client ID: ";
		getline(cin, ID);
		cout << "[INFO] Saving to ClientSettings.txt...\n";
		ofstream out("ClientSettings.txt");
		out << SrvIP << endl << PORT << endl << ID;
		out.close();
	}
	const char* ServerIP = SrvIP.c_str();
	cout << "\n------------------------------\n";
	cout << "[ Client Settings ]\n\n";
	cout << "Server IP: " << SrvIP << endl;
	cout << "Connection Port: " << PORT << endl;
	cout << "Client ID: " << ID << endl;
	cout << "------------------------------\n\n";
	cout << "[INFO] Waiting 30 seconds for Client to finish starting up...\n"; Sleep(30000); cout << "\n------------------------------\n";
	cout << "[ Client Specs ]\n\n";
	viewSpecs();
	cout << "------------------------------\n\n";

	cout << "[INFO] Starting up...\n";
	//2. Initialize WSA -----------------------
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
		cout << "[ERROR] Error initializing WSA.\n";
		exitFail();
	}
	//3. Create Socket ------------------------
	nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nClientSocket < 0) {
		cout << "[ERROR] Error creating socket.\n";
		exitFail();
	}
	//4. Initialize Enviroment ----------------
	srv.sin_family = AF_INET;
	srv.sin_port = htons(stoi(PORT));
	srv.sin_addr.s_addr = inet_addr(ServerIP);
	memset(&(srv.sin_zero), 0, 8);
	//5. Connect ke Server --------------------
	nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
	while (nRet < 0) {
		cout << "[WARN] Failed to connect. Reconnecting...\n";
		nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
	}
	cout << "[INFO] Connected to Server!\n";
	char buff[bufferSize] = { 0, };
	result = send(nClientSocket, ID.c_str(), 10, 0); //Sends Client ID
	//-----------------------------------------

	int startuptime = time(0);
	int refreshTime = (rand() % (15 - 5 + 1)) + 5; //Random 5 - 15 Seconds
	cout << "[INFO] Uploads uptime every " << refreshTime << " seconds.\n";
	while (1) {
		//Upload uptime ---------------------------
		int currTime = time(0) - startuptime;
		int d = currTime / (24 * 3600);
		int h = (currTime - (d * 3600)) / 3600;
		int m = (currTime - (d * 3600) - (h * 3600)) / 60;
		currTime = currTime - (d * 3600) - (h * 3600) - (m * 60);
		string msg = ID + "/Uptime/" + to_string(d) + ':' + to_string(h) + ':' + to_string(m) + ':' + to_string(currTime) + "/";
		const char* cstr = msg.c_str();
		cout << "[INFO] Updating Uptime...\n";
		result = send(nClientSocket, cstr, 30, 0);
		//-----------------------------------------

		//Receive command + Check connection ------
		//result = recv(nClientSocket, buff, 30, 0);
		if (result < 0) {
			cout << "[WARN] Disconnected from Server. Reconnecting...\n";
			exitFail();

			//KODE BELUM BISA AUTO RECCONECT
			//do {
			//	cout << "[WARN] Failed to connect. Reconnecting...\n";
			//	nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
			//} while (nRet < 0);
			//cout << "[INFO] Connected to Server!\n";
		}
		//-----------------------------------------

		//Upload Data -----------------------------
		//cout << "[INFO] SERVER: " << buff << endl;
		//string buff_s(buff);

		//( Lakukan update berdasarkann Command dari Server )

		////Kirim perintah ke Server
		//string msg;
		//getline(cin, msg);
		//const char* cstr = msg.c_str();
		//send(nClientSocket, cstr, bufferSize, 0);
		//-----------------------------------------
		Sleep(refreshTime * 1000);
	}
}