#include <iostream>
#include <winsock.h>
#include <string>
#include <ctime>
using namespace std;

#define PORT 9909
#define bufferSize 768
int nClientSocket;
struct sockaddr_in srv;

int main()
{
	srand(time(0));
	int nRet = 0, result = 0;
	string SrvIP, ID;
	cout << "Input Server IP: ";
	getline(cin, SrvIP);
	cout << "Input ID Komputer: ";
	getline(cin, ID);
	const char* ServerIP = SrvIP.c_str();

	//Initialize WSA
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
		cout << "Failed to initialize WSA.\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	//Create Socket
	nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nClientSocket < 0) {
		cout << "Socket isn't opened.\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	//Inisialisasi enviroment untuk sockaddr
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = inet_addr(ServerIP);
	memset(&(srv.sin_zero), 0, 8);

	//Connect ke Server
	nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
	if (nRet < 0) {
		cout << "Connection failed.\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else {
		cout << "Connected to Server!\n";
		char buff[bufferSize] = { 0, };

		//Loop send & receive messages
		int startuptime = time(0);//waktu program menyala (buat uptime)
		int refreshTime = (rand() % (15 - 5 + 1)) + 5;
		cout << "INFO: Refresh every " << refreshTime << " seconds.\n\n";
		while (1) {

			//Menunggu pesan dari server + Cek Koneksi dengan kirim Uptime..
			int currTime = time(0) - startuptime; //Output seconds
			int d = currTime / (24 * 3600);
			int h = (currTime - (d * 3600)) / 3600;
			int m = (currTime - (d * 3600) - (h * 3600)) / 60;
			currTime = currTime - (d * 3600) - (h * 3600) - (m * 60);

			string total = to_string(d) + ':' + to_string(h) + ':' + to_string(m) + ':' + to_string(currTime);
			string msg = ID + "/Uptime/" + total + "/";
			const char* cstr = msg.c_str();

			cout << "INFO: Updating Uptime.";

			result = send(nClientSocket, cstr, bufferSize, 0);
			result = recv(nClientSocket, buff, bufferSize, 0);
			cout << "\nSERVER: " << buff << endl;
			if (result < 0) {
				cout << "Disconnected from Server.\n";
				WSACleanup();
				exit(EXIT_FAILURE);
			}
			else {
				string buff_s(buff);
				//Lakukan update berdasarkan 
				//Command dari Server.

				/*cout << "Input pesan baru ke Server: ";
				string msg;
				getline(cin, msg);
				const char* cstr = msg.c_str();
				result = send(nClientSocket, cstr, bufferSize, 0);*/
			}

			//ide: supaya tidak bersamaan, coba random delay???
			Sleep(refreshTime * 1000);
		}
	}

}