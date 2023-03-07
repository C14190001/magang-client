#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
using namespace std;

string getIPv4Address() {
	string line = "";
	ifstream ipFile;
	int offset;
	system("ipconfig > ip.txt");
	ipFile.open("ip.txt");
	if (ipFile.is_open()) {
		while (!ipFile.eof()) {
			getline(ipFile, line);
			if ((offset = line.find("IPv4 Address. . . . . . . . . . . :")) != string::npos) {
				line.erase(0, 39);
				//cout << line;
				ipFile.close();
				remove("ip.txt");
				break;
			}
		}
	}
	return line;
}
