/*  PDF_Bookzoom - A simple tool to avoid bookmark zoom changes
 *  Copyright (C) 2012  KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Main.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	FILE * logFile = NULL;
	std::wstring filename(lpCmdLine);
	
	//For some reason, this fixes the problem with path and spaces
	if(lpCmdLine[0] == L'"')
		filename = filename.substr(1,filename.length()-2);
	
	_wfopen_s(&logFile, L"PDF_Bookzoom.log", L"a+, ccs=UTF-8");

	if(logFile != NULL) fwprintf(logFile, L"Opening file: %s\n", filename.c_str());

	std::fstream File;
	File.open(filename.c_str(), std::ios::binary | std::ios::in);

	if(!File.is_open())		
	{
		if(logFile != NULL) fwprintf(logFile, L"Could not open file.\n");		
		return 0;
	}

	char fileID[4] = {0};
	File.read(fileID, 4);
	if(*(unsigned int*)fileID != 0x46445025)
	{
		File.close();
		if(logFile != NULL) fwprintf(logFile, L"Exit: This is not a PDF file.\n");		
		return 0;
	}

	File.seekg(EOF, std::ios_base::end);
	std::streamoff filesize = File.tellg(); filesize++;
	char* filemem = new char[filesize];
			
	File.seekg(0x0, std::ios_base::beg);
	File.read(filemem, filesize);
	File.close();

	char nicebookmark[] = {0x2F, 0x58, 0x59, 0x5A, 0x20, 0x5D};
	int nicebookmark_length = sizeof(nicebookmark)/sizeof(char);

	int counter = 0;
		
	for(unsigned int i = 0; i<(filesize-80); i++)
	{						
		if(filemem[i] == '/') 				
		if(	(filemem[i+1] == 'X' && filemem[i+2] == 'Y' && filemem[i+3] == 'Z')||
			(filemem[i+1] == 'F' && filemem[i+2] == 'i' && filemem[i+3] == 't') )
		{				
			memcpy(&filemem[i], nicebookmark, sizeof(nicebookmark));

			for(unsigned int j = nicebookmark_length; j<(filesize-80); j++)
			{										
				if(filemem[i+j] == 0x0D) break;
				filemem[i+j] = 0x20;
			}

			counter++;
		}								
	}
		
	if(counter > 0)
	{
		filename.replace(filename.length()-4, 8, L"_New.PDF");
		if(logFile != NULL) fwprintf(logFile, L"Saving file: %s\n", filename.c_str());
		
		File.open(filename.c_str(), std::ios::binary | std::ios::out);

		if(!File.is_open())		
		{
			if(logFile != NULL) fwprintf(logFile, L"Exit: Could not open file on save.\n");
			delete [] filemem;
			return 0;
		}
				
		File.write(filemem, filesize);	
		File.close();

		if(logFile != NULL) fwprintf(logFile, L"Success! %d changes total.\n", counter);		
	}
	else
		if(logFile != NULL) fwprintf(logFile, L"No change has been done.\n");
			
	delete [] filemem;
	if(logFile != NULL) fclose(logFile);
	
	return 0;
}