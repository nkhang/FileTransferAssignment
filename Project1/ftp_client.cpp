#include "ftp_client.h"

/*Initialize ftp_client
> Create new socket
> Set tranmission mode to active as default*/
ftp_client::ftp_client()
{
	_pSocket = new CSocket;
	memset(buf, 0, BUFSIZE);
	_pSocket->Create();
	isPassive = 0;
	m_IPAddr = NULL;
}
/*Process user interface
> Receive user commands
> Make function calls*/
void ftp_client::UserHandler()
{
	char command[STR_LENGTH];
	this->Help();
	do
	{
		cout << "ftp> ";
		cin >> command;
		if (command == NULL) continue;
		int flag = this->CommandHandler(command);
		switch (flag)
		{
		case CODE_ERROR:
			cout << "Command invalid!\n";
			continue;
			break;
		case CODE_OPEN:
			this->_pSocket->Close();
			this->_pSocket->Create();
			this->Login();
			this->getIPAddr();
			break;
		case CODE_LS:
			this->Ls();
			break;
		case CODE_CD:
			this->Cd();
			break;
		case CODE_LCD:
			this->Lcd();
			break;
		case CODE_DIR:
			this->Dir();
			break;
		case CODE_GET:
			this->Get();
			break;
		case CODE_PUT:
			this->Put();
			break;
		case CODE_MGET:
			this->MGet();
			break;
		case CODE_MPUT:
			this->MPut();
			break;
		case CODE_DELETE:
			this->Del();
			break;
		case CODE_MDELETE:
			this->MDel();
			break;
		case CODE_MKDIR:
			this->Mkdir();
			break;
		case CODE_RMDIR:
			this->Rmdir();
			break;
		case CODE_PWD:
			this->Pwd();
			break;
		case CODE_PASSIVE:
			this->Passive();
			break;
		case CODE_ACTIVE:
			this->Active();
			break;
		case CODE_HELP:
			this->Help();
			break;
		case CODE_QUIT:
			this->Quit();
			system("pause");
			return;
		}
	} while (TRUE);
}

/*Process user commands
> return CODE of each command*/
int ftp_client::CommandHandler(char command[])
{
	
	if (!strcmp(command, "open"))
		return CODE_OPEN;
	if (!strcmp(command, "ls"))
		return CODE_LS;
	if (!strcmp(command, "dir"))
		return CODE_DIR;
	if (!strcmp(command, "cd"))
		return CODE_CD;
	if (!strcmp(command, "lcd"))
		return CODE_LCD;
	if (!strcmp(command, "get"))
		return CODE_GET;
	if (!strcmp(command, "put"))
		return CODE_PUT;
	if (!strcmp(command, "mget"))
		return CODE_MGET;
	if (!strcmp(command, "mput"))
		return CODE_MPUT;
	if (!strcmp(command, "delete"))
		return CODE_DELETE;
	if (!strcmp(command, "mdelete"))
		return CODE_MDELETE;
	if (!strcmp(command, "mkdir"))
		return CODE_MKDIR;
	if (!strcmp(command, "rmdir"))
		return CODE_RMDIR;
	if (!strcmp(command, "pwd"))
		return CODE_PWD;
	if (!strcmp(command, "passive"))
		return CODE_PASSIVE;
	if (!strcmp(command, "active"))
		return CODE_ACTIVE;
	if (!strcmp(command, "quit"))
		return CODE_QUIT;
	if (!strcmp(command, "exit"))
		return CODE_QUIT;
	if (!strcmp(command, "help"))
		return CODE_HELP;
	return CODE_ERROR;
}

/*Connect to server*/
bool ftp_client::Connect(wchar_t IPAddr[])
{
	if (_pSocket->Connect(IPAddr, PORT) != 0)
	{
		cout << "Successfully connect to FTP Server!\n";
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*Login to FTP server*/
bool ftp_client::Login(wchar_t IPAddr[STR_LENGTH])
{
	if (IPAddr == NULL)
	{
		IPAddr = new wchar_t[STR_LENGTH];
		cout << "To: ";
		wcin >> IPAddr;
	}
	if (this->Connect(IPAddr) == FALSE)
	{
		return FALSE;
	}
	else
	{
		int tmpresult;
		int codeftp;
		char* str;
		memset(buf, 0, BUFSIZE);
		while ((tmpresult = _pSocket->Receive(buf, BUFSIZ, 0)) > 0)
		{
			sscanf(buf, "%d", &codeftp);
			cout << buf;
			if (codeftp != 220) //120, 240, 421: something wrong
			{
				return FALSE;
			}

			str = strstr(buf, "220");//DK dung
			if (str != NULL) {
				break;
			}
			memset(buf, 0, tmpresult);
		}

		//Send username
		char username[50];
		cout<<"User name: ";
		cin >> username;

		memset(buf, 0, BUFSIZE);
		sprintf(buf, "USER %s\r\n", username);
		if (this->SendCommand() == FALSE)
			return FALSE;

		if (!checkFTPCode(331)) return FALSE;
		
		//Send password
		char password[50];
		cout << "Password: ";
		cin >> password;

		memset(buf, 0, BUFSIZE);
		sprintf(buf, "PASS %s\r\n", password);
		if (this->SendCommand() == FALSE)
			return FALSE;
		if (!checkFTPCode(230)) return FALSE;
	}
	return TRUE;
	delete IPAddr;
}

/*List file from selected directory on server
> Open dataBuffer.txt. This buffer is to receive data from server
> Prepare NLST command
> Check if tranmission mode is active or passive then use dataTvAM of dataTvPM
> Put data from dataBuffer to specific file with the right file type*/
bool ftp_client::Ls()
{
	if (dBuffer.is_open() == FALSE)
	{
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	else
	{
		dBuffer.close();
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "NLST\r\n");
	if (isPassive == 0)
	{
		if (!this->dataTvAM(DOWNSTREAM)) return FALSE;
	}
	else
	{
		if (!this->dataTvPM(DOWNSTREAM)) return FALSE;
	}

	dBuffer.clear();
	dBuffer.seekg(0, ios::beg);

	stringstream buffer;
	buffer << dBuffer.rdbuf();
	string contents(buffer.str());
	cout << contents;

	dBuffer.clear();
	dBuffer.close();
	return TRUE;
}

/*The same as ls with LIST command*/
bool ftp_client::Dir()
{
	if (dBuffer.is_open() == FALSE)
	{
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	else
	{
		dBuffer.close();
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "LIST\r\n");
	if (isPassive == 0)
	{
		if (!this->dataTvAM(DOWNSTREAM)) return FALSE;
	}
	else
	{
		if (!this->dataTvPM(DOWNSTREAM)) return FALSE;
	}

	dBuffer.clear();
	dBuffer.seekg(0, ios::beg);

	stringstream buffer;
	buffer << dBuffer.rdbuf();
	string contents(buffer.str());
	cout << contents;

	dBuffer.clear();
	dBuffer.close();
	return TRUE;
}

bool ftp_client::Put(char src_filename[STR_LENGTH], char dest_filename[STR_LENGTH])
{
	if (dBuffer.is_open() == FALSE)
	{
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	else
	{
		dBuffer.close();
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	//Input filename
	if (src_filename == NULL)
	{
		src_filename = new char[STR_LENGTH];
		memset(src_filename, 0, sizeof src_filename);
		cout << "Source filename: ";
		cin >> src_filename;
	}
	if (dest_filename == NULL)
	{
		dest_filename = new char[STR_LENGTH];
		memset(dest_filename, 0, sizeof dest_filename);
		cout << "Destination filename: ";
		cin >> dest_filename;
	}

	//Read file
	fstream fout;
	fout.open(src_filename, ios::in | ios::binary);

	int filesize;
	int maxsize = 1024 * 1024;
	fout.seekg(0, ios::end);
	filesize = fout.tellg();

	fout.clear();
	fout.seekg(0, ios::beg);

	char* buff = new char[1024 * 1024 + 1];
	memset(buff, 0, sizeof buff);
	while (TRUE)
	{
		if (filesize <= maxsize)
		{
			fout.read(buff, filesize);
			dBuffer.write(buff, filesize);
			break;
		}
		fout.read(buff, maxsize);
		dBuffer.write(buff, maxsize);
		filesize = filesize - maxsize;
		memset(buff, 0, sizeof buff);
	}
	cout << "ftp: Read " << dBuffer.tellg() << " bytes\n";
	memset(buf, 0, sizeof buf);
	sprintf(buf, "STOR %s\r\n", dest_filename);
	//Call Transfer function
	if (isPassive == 0)
	{
		if (!this->dataTvAM(UPSTREAM)) return FALSE;
	}
	else
	{
		if (!this->dataTvPM(UPSTREAM)) return FALSE;
	}

	fout.close();

	dBuffer.clear();
	dBuffer.close();
	return TRUE;
}

bool ftp_client::Get(char src_filename[STR_LENGTH], char dest_filename[STR_LENGTH])
{
	if (dBuffer.is_open() == FALSE)
	{
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	else
	{
		dBuffer.close();
		dBuffer.open("dataBuffer.dat", ios::in | ios::out | ios::trunc | ios::binary);
	}
	//Input filename
	if (src_filename == NULL)
	{
		src_filename = new char[STR_LENGTH];
		memset(src_filename, 0, sizeof src_filename);
		cout << "Source filename: ";
		cin >> src_filename;
	}
	if (dest_filename == NULL)
	{
		dest_filename = new char[STR_LENGTH];
		memset(dest_filename, 0, sizeof dest_filename);
		cout << "Destination filename: ";
		cin >> dest_filename;
	}
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "RETR %s\r\n", src_filename);
	//Call transfer function
	if (isPassive == 0)
	{
		if (!this->dataTvAM(DOWNSTREAM)) return FALSE;
	}
	else
	{
		if (!this->dataTvPM(DOWNSTREAM)) return FALSE;
	}
	//Write file
	fstream fin;
	fin.open(dest_filename, ios::out | ios::binary | ios::trunc);

	dBuffer.clear();
	dBuffer.seekg(0, ios::beg);

	fin << dBuffer.rdbuf();
	fin.flush();

	fin.close();
		dBuffer.clear();
	dBuffer.close();
	return TRUE;
}

bool ftp_client::MPut(char filenames[STR_LENGTH])
{
	char* filename[8];

	if (filenames == NULL)
	{
		filenames = new char[STR_LENGTH];
		cin.ignore();
		cout << "Filenames: ";
		cin.getline(filenames, 256);
	}

	filename[0] = strtok(filenames, " ,-");

	int i = 1;
	while (i < 8)
	{
		filename[i] = strtok(NULL, " ,-");
		if (filename[i] == NULL) break;
		i++;
	}

	for (int j = 0; j < i; j++)
	{
		char dst_filename[STR_LENGTH];
		cout << "Destination file for " << filename[j] << ": ";
		cin >> dst_filename;
		this->Put(filename[j], dst_filename);
	}
	delete filenames;	
	return FALSE;
}

bool ftp_client::MGet(char filenames[STR_LENGTH])
{
	char* filename[8];

	if (filenames == NULL)
	{
		filenames = new char[STR_LENGTH];
		cin.ignore();
		cout << "Filenames: ";
		cin.getline(filenames, 256);
	}

	filename[0] = strtok(filenames, " ,-");

	int i = 1;
	while (i < 8)
	{
		filename[i] = strtok(NULL, " ,-");
		if (filename[i] == NULL) break;
		i++;
	}

	for (int j = 0; j < i; j++)
	{
		char dst_filename[STR_LENGTH];
		cout << "Destination file for " << filename[j] << ": ";
		cin >> dst_filename;
		this->Get(filename[j], dst_filename);
	}
	delete filenames;
	return FALSE;
}

bool ftp_client::Del(char filename[STR_LENGTH])
{
	if (filename == NULL)
	{
		filename = new char[STR_LENGTH];
		memset(filename, 0, sizeof filename);
		cout << "Filename: ";
		cin >> filename;
	}

	memset(buf, 0, BUFSIZE);
	sprintf(buf, "DELE %s\r\n", filename);
	if (this->SendCommand() == FALSE)
		return FALSE;
	if (!checkFTPCode(250)) return FALSE;
	delete filename;
	return TRUE;
}

bool ftp_client::MDel(char filenames[STR_LENGTH])
{
	char* filename[8];

	if (filenames == NULL)
	{
		filenames = new char[STR_LENGTH];
		cin.ignore();
		cout << "Filenames: ";
		cin.getline(filenames, 256);
	}

	filename[0] = strtok(filenames, " ,-");

	int i = 1;
	while (i < 8)
	{
		filename[i] = strtok(NULL, " ,-");
		if (filename[i] == NULL) break;
		i++;
	}

	for (int j = 0; j < i; j++)
	{
		char permission;
		cout << "Delete " << filename[j] << "? (Y/n): ";
		permission = _getch();
		if (permission == 'Y' || permission == 'y')
		{
			memset(buf, 0, BUFSIZE);
			sprintf(buf, "DELE %s\r\n", filename[j]);
			cout << endl;
			if (this->SendCommand() == FALSE) continue;
			if (!checkFTPCode(250)) return FALSE;
			memset(buf, 0, BUFSIZE);
		}
		else
		{
			continue;
		}
	}
	delete filenames;
	return FALSE;
}

bool ftp_client::Cd(char directory[STR_LENGTH])
{
	if (directory == NULL)
	{
		directory = new char[STR_LENGTH];
		cout << "Directory ";
		cin >> directory;
	}

	memset(buf, 0, BUFSIZE);
	sprintf(buf, "CWD %s\r\n", directory);
	if (this->SendCommand() == FALSE) return FALSE;
	if (!checkFTPCode(250)) return FALSE;
	return false;
}

bool ftp_client::Lcd(char directory[STR_LENGTH])
{
	char dirbuf[BUFSIZ + 1];
	if (directory == NULL)
	{
		_getcwd(dirbuf, sizeof dirbuf);
		cout << dirbuf << endl;

		directory = new char[BUFSIZ];
		cout << "Directory ";
		cin >> directory;
		_chdir(directory);
		_getcwd(dirbuf, sizeof dirbuf);
	}

	cout << dirbuf << endl;
	delete[]directory;
	return TRUE;
}

bool ftp_client::Mkdir(char directory[STR_LENGTH])
{
	if (directory == NULL)
	{
		directory = new char[STR_LENGTH];
		cout << "Directory name ";
		cin >> directory;
	}

	memset(buf, 0, BUFSIZE);
	sprintf(buf, "MKD %s\r\n", directory);
	if (this->SendCommand() == FALSE) return FALSE;
	if (!checkFTPCode(257)) return FALSE;
	return TRUE;
}

bool ftp_client::Rmdir(char directory[STR_LENGTH])
{
	if (directory == NULL)
	{
		directory = new char[STR_LENGTH];
		cout << "Directory name ";
		cin >> directory;
	}

	memset(buf, 0, BUFSIZE);
	sprintf(buf, "RMD %s\r\n", directory);
	if (this->SendCommand() == FALSE) return FALSE;
	if (!checkFTPCode(250)) return FALSE;
	return TRUE;
}

bool ftp_client::Pwd()
{
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "PWD\r\n");
	if (this->SendCommand() == FALSE) return FALSE;
	if (!checkFTPCode(257)) return FALSE;
	return TRUE;
}

void ftp_client::Help()
{
	cout << ">> commands:\n";
	cout << setw(30) << left << "open" << setw(30) << left << "mget" << setw(30) << left << "rmdir" << endl;
	cout << setw(30) << left << "ls" << setw(30) << left << "cd" << setw(30) << left << "pwd" << endl;
	cout << setw(30) << left << "dir" << setw(30) << left << "lcd" << setw(30) << left << "passive" << endl;
	cout << setw(30) << left << "put" << setw(30) << left << "delete" << setw(30) << left << "active" << endl;
	cout << setw(30) << left << "get" << setw(30) << left << "mdelete" << setw(30) << left << "quit" << endl;
	cout << setw(30) << left << "mput" << setw(30) << left << "mkdir" << setw(30) << left << "exit" << endl;
}

void ftp_client::Passive()
{
	this->isPassive = 1;
}

void ftp_client::Active()
{
	this->isPassive = 0;
}

bool ftp_client::Quit()
{
	sprintf(buf, "QUIT\r\n");
	if (this->SendCommand() == FALSE)
		return FALSE;
	return TRUE;
}

ftp_client::~ftp_client()
{
	_pSocket->Close();
	delete _pSocket;
	if(m_IPAddr != NULL)
		delete[]m_IPAddr;
}

bool ftp_client::SendCommand()
{
	if (_pSocket->Send(buf, BUFLEN, 0) == 0)
		return FALSE;
	memset(buf, 0, BUFSIZE);
	if (_pSocket->Receive(buf, BUFSIZ, 0) == 0)
		return FALSE;
	cout << buf;
	return TRUE;
}

bool ftp_client::dataTvAM(bool stream)
{
	CSocket dtSocket, tmpSock;
	CString dtIPAddr; UINT dtPort;
	int LcIP[4];

	char t_buf[BUFSIZ + 1];
	sprintf(t_buf, "%s", buf);

	if (dtSocket.Create() == 0) return FALSE;
	if (dtSocket.Listen(5) == 0) return FALSE;

	if (dtSocket.GetSockName(dtIPAddr, dtPort) == 0) return FALSE;
	//CT2A ascii(dtIPAddr);
	//TRACE(_T("ASCII: %S\n"), ascii.m_psz);
	//cout << ascii << endl;
	int x = dtPort / 256;
	int y = dtPort % 256;
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", m_IPAddr[0], m_IPAddr[1], m_IPAddr[2], m_IPAddr[3], x, y);
	if (this->SendCommand() == FALSE)
	{
		dtSocket.Close();
		return FALSE;
	}
	if (!checkFTPCode(200))
	{
		dtSocket.Close();
		return FALSE;
	}
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "%s", t_buf);
	_pSocket->Send(buf, BUFLEN, 0);
	_pSocket->Receive(buf, BUFSIZ, MSG_PEEK);
	if (checkFTPCode(150) == FALSE)
	{
		memset(buf, 0, BUFSIZE);
		_pSocket->Receive(buf, BUFSIZ, 0);
		cout << buf;
		cout << "ftp: Error! Check your command or filename(s)!\n";
		dtSocket.Close();
		return FALSE;
	}

	memset(buf, 0, BUFSIZE);
	if (dtSocket.Accept(tmpSock) == 0)
	{
		tmpSock.Close();
		dtSocket.Close();
		return FALSE;
	}

	if (stream == DOWNSTREAM)
	{
		int buf_count;
		buf_count = tmpSock.Receive(buf, BUFSIZ, 0);
		while (buf_count != 0)
		{
			dBuffer.write(buf, buf_count);
			memset(buf, 0, BUFSIZE);
			buf_count = tmpSock.Receive(buf, BUFSIZ, 0);
		}
	}
	else
	{
		int filesize = dBuffer.tellg();
		dBuffer.clear();
		dBuffer.seekg(0, ios::beg);
		memset(buf, 0, BUFSIZE);
		while(filesize != 0)
		{
			
			if (filesize <= 512)
			{
				dBuffer.read(buf, filesize);
				tmpSock.Send(buf, filesize, 0);
				break;
			}
			dBuffer.read(buf, BUFSIZ);
			tmpSock.Send(buf, BUFSIZ, 0);
			filesize = filesize - 512;
			memset(buf, 0, BUFSIZE);
		}
	}

	tmpSock.Close();
	dtSocket.Close();
	memset(buf, 0, BUFSIZE);
	Sleep(200);
	if (_pSocket->Receive(buf, BUFSIZ, 0) == SOCKET_ERROR) return FALSE;
	cout << buf;
	return TRUE;
}

bool ftp_client::dataTvPM(bool stream)
{
	CSocket dtSocket;
	wchar_t SrvIP[16];
	int IPAddr[4];
	int Port[2];

	char t_buf[BUFSIZ + 1];
	sprintf(t_buf, "%s", buf);

	sprintf(buf, "PASV\r\n");
	if (this->SendCommand() == FALSE) return FALSE;
	if (checkFTPCode(227) == FALSE) return FALSE;
	
	sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", &IPAddr[0], &IPAddr[1], &IPAddr[2], &IPAddr[3], &Port[0], &Port[1]);
	wsprintf(SrvIP, _T("%d.%d.%d.%d"), IPAddr[0], IPAddr[1], IPAddr[2], IPAddr[3]);
	int nPort = Port[0] * 256 + Port[1];
	if (dtSocket.Create() == 0) return FALSE;
	if (dtSocket.Connect(SrvIP, nPort) == 0) return FALSE;

	memset(buf, 0, BUFSIZE);
	sprintf(buf, "%s", t_buf);
	_pSocket->Send(buf, BUFLEN, 0);
	_pSocket->Receive(buf, BUFSIZ, MSG_PEEK);
	if (checkFTPCode(150) == FALSE)
	{
		memset(buf, 0, BUFSIZE);
		_pSocket->Receive(buf, BUFSIZ, 0);
		cout << buf;
		cout << "ftp: Faltal! Check your command or filename(s)!\n";
		dtSocket.Close();
		return FALSE;
	}

	if (stream == DOWNSTREAM)
	{
		int buf_count;
		buf_count = dtSocket.Receive(buf, BUFSIZ, 0);
		while (buf_count != 0)
		{
			dBuffer.write(buf, buf_count);
			memset(buf, 0, BUFSIZE);
			buf_count = dtSocket.Receive(buf, BUFSIZ, 0);
		}
	}
	else
	{
		int filesize = dBuffer.tellg();
		dBuffer.clear();
		dBuffer.seekg(0, ios::beg);
		memset(buf, 0, BUFSIZE);
		while (filesize != 0)
		{
			if (filesize <= 512)
			{
				dBuffer.read(buf, filesize);
				dtSocket.Send(buf, filesize, 0);
				break;
			}
			dBuffer.read(buf, BUFSIZ);
			dtSocket.Send(buf, BUFSIZ, 0);
			filesize = filesize - 512;
			memset(buf, 0, BUFSIZE);
		}
	}
	dtSocket.Close();

	Sleep(200);
	memset(buf, 0, BUFSIZE);
	_pSocket->Receive(buf, BUFSIZ, 0);
	cout << buf;
	return TRUE;
}

bool ftp_client::checkFTPCode(int code)
{
	int codeftp;
	sscanf(buf, "%d", &codeftp);
	if (codeftp != code)
	{
		return FALSE;
	}
	return TRUE;
}

void ftp_client::getIPAddr()
{
	CString IPAddr; UINT Port;
	int * iIPAddr = new int[4];
	_pSocket->GetSockName(IPAddr, Port);
	CT2A cIPAddr(IPAddr);
	TRACE(_T("ASCII: %S\n"), cIPAddr.m_psz);
	sscanf(cIPAddr, "%d.%d.%d.%d", &iIPAddr[0], &iIPAddr[1], &iIPAddr[2], &iIPAddr[3]);
	this->m_IPAddr = iIPAddr;
}
