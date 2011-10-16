#include <stdafx.h>
#include "ExecProcess.h"

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682499%28v=vs.85%29.aspx

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;

#define BUFSIZE 4096

void ErrorExit(PTSTR)
{
}

void WriteToPipe(const uint8_t* p, size_t bytes) 
// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{ 
   DWORD dwRead, dwWritten; 
   BOOL bSuccess = FALSE;
 
   bSuccess = WriteFile(g_hChildStd_IN_Wr, p, bytes, &dwWritten, NULL);
 
// Close the pipe handle so the child process stops reading. 
 
   if ( ! CloseHandle(g_hChildStd_IN_Wr) ) 
      ErrorExit(TEXT("StdInWr CloseHandle")); 
} 

void ReadFromPipe(std::vector<uint8_t>& outData) 

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 

// Close the write end of the pipe before reading from the 
// read end of the pipe, to control child process execution.
// The pipe is assumed to have enough buffer space to hold the
// data the child process has already written to it.
   if (!CloseHandle(g_hChildStd_OUT_Wr)) 
      ErrorExit(TEXT("StdOutWr CloseHandle")); 

	DWORD dwRead, dwWritten; 
   CHAR chBuf[BUFSIZE]; 
   BOOL bSuccess = FALSE;
   HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
 
   for (;;) 
   { 
      bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
      if( ! bSuccess || dwRead == 0 ) break; 
      outData.insert(outData.end(), chBuf, chBuf+dwRead);
	
	  if (hParentStdOut) {
		  bSuccess = WriteFile(hParentStdOut, chBuf, 
							   dwRead, &dwWritten, NULL);
		  if (! bSuccess ) break; 
	  }
   } 
}

void ErrorExit(PTSTR); 

void CreateChildProcess(LPTSTR cmdLine)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
   PROCESS_INFORMATION piProcInfo; 
   STARTUPINFO siStartInfo;
   BOOL bSuccess = FALSE; 
 
// Set up members of the PROCESS_INFORMATION structure. 
 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = 0; //g_hChildStd_OUT_Wr;
   siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
   siStartInfo.hStdInput = g_hChildStd_IN_Rd;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

// Create the child process. 
    
   bSuccess = CreateProcess(
	NULL, 
      cmdLine,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      CREATE_NO_WINDOW,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   
   // If an error occurs, exit the application. 
   if ( ! bSuccess ) 
      ErrorExit(TEXT("CreateProcess"));
   else 
   {
      // Close handles to the child process and its primary thread.
      // Some applications might keep these handles to monitor the status
      // of the child process, for example. 

      CloseHandle(piProcInfo.hProcess);
      CloseHandle(piProcInfo.hThread);
   }
}

bool ExecProcess(
	LPCTSTR cmdLine,
	const std::vector<uint8_t>& inData,
	std::vector<uint8_t>& outData
	)
{
   SECURITY_ATTRIBUTES saAttr; 
 
   printf("\n->Start of parent execution.\n");

// Set the bInheritHandle flag so pipe handles are inherited. 
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   saAttr.bInheritHandle = TRUE; 
   saAttr.lpSecurityDescriptor = NULL; 

// Create a pipe for the child process's STDOUT. 
   if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, BUFSIZE) ) 
      ErrorExit(TEXT("StdoutRd CreatePipe")); 

// Ensure the read handle to the pipe for STDOUT is not inherited.
   if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdout SetHandleInformation")); 

// Create a pipe for the child process's STDIN. 
   if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, inData.size())) 
      ErrorExit(TEXT("Stdin CreatePipe")); 

// Ensure the write handle to the pipe for STDIN is not inherited. 
   if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdin SetHandleInformation")); 
 
// Create the child process. 
   CreateChildProcess((LPTSTR)cmdLine);

// Write to the pipe that is the standard input for a child process. 
// Data is written to the pipe's buffers, so it is not necessary to wait
// until the child process is running before writing data.
   WriteToPipe(&inData[0], inData.size()); 
 
// Read from pipe that is the standard output for child process. 
   ReadFromPipe(outData); 

   printf("\n->End of parent execution.\n");

// The remaining open handles are cleaned up when this process terminates. 
// To avoid resource leaks in a larger application, close handles explicitly. 

   return true; 
	
}


