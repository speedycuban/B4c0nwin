#include "stdafx.h"
#include "B4c0nwin.h"


HINSTANCE hInst;
TCHAR *szTitle			= _T("Gyazo");
TCHAR *szWindowClass	= _T("GYAZOWIN");
TCHAR *szWindowClassL	= _T("GYAZOWINL");
HWND hLayerWnd;

int ofX, ofY;


ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	LayerWndProc(HWND, UINT, WPARAM, LPARAM);

int					GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

BOOL				isPng(LPCTSTR fileName);
VOID				drawRubberband(HDC hdc, LPRECT newRect, BOOL erase);
VOID				execUrl(const char* str);
VOID				setClipBoardText(const char* str);
BOOL				convertPNG(LPCTSTR destFile, LPCTSTR srcFile);
BOOL				savePNG(LPCTSTR fileName, HBITMAP newBMP);
BOOL				uploadFile(HWND hwnd, LPCTSTR fileName);
std::string			getId();
BOOL				saveId(const WCHAR* str);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	TCHAR	szThisPath[MAX_PATH];
	DWORD   sLen;


	sLen = GetModuleFileName(NULL, szThisPath, MAX_PATH);
	for(unsigned int i = sLen; i >= 0; i--) {
		if(szThisPath[i] == _T('\\')) {
			szThisPath[i] = _T('\0');
			break;
		}
	}

	SetCurrentDirectory(szThisPath);


	if ( 2 == __argc )
	{

		if (isPng(__targv[1])) {

			uploadFile(NULL, __targv[1]);
		}else {

			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (convertPNG(tmpFile, __targv[1])) {

				uploadFile(NULL, tmpFile);
			} else {

				MessageBox(NULL, _T("Cannot convert this image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}
			DeleteFile(tmpFile);
		}
		return TRUE;
	}


	MyRegisterClass(hInstance);


	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}


BOOL isPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];
	
	FILE *fp = NULL;
	
	if (0 != _tfopen_s(&fp, fileName, _T("rb")) ||
		8 != fread(readHead, 1, 8, fp)) {

		return FALSE;
	}
	fclose(fp);

	for(unsigned int i=0;i<8;i++)
		if(pngHead[i] != readHead[i]) return FALSE;

	return TRUE;

}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;


	wc.style         = 0;					
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
	wc.hbrBackground = 0;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	RegisterClass(&wc);


	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = LayerWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClassL;

	return RegisterClass(&wc);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance;

	int x, y, w, h;

	x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	h = GetSystemMetrics(SM_CYVIRTUALSCREEN);


	ofX = x; ofY = y;


	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowClass, NULL, WS_POPUP,
		0, 0, 0, 0,
		NULL, NULL, hInstance, NULL);


	if (!hWnd) return FALSE;
	

	MoveWindow(hWnd, x, y, w, h, FALSE);
	

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);


	SetTimer(hWnd, 1, 100, NULL);



	hLayerWnd = CreateWindowEx(
	 WS_EX_TOOLWINDOW
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowClassL, NULL, WS_POPUP,
		100, 100, 300, 300,
		hWnd, NULL, hInstance, NULL);

    SetLayeredWindowAttributes(hLayerWnd, RGB(255, 0, 0), 100, LWA_COLORKEY|LWA_ALPHA);

	


	
	return TRUE;
}


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;         
   UINT  size = 0;         

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}


VOID drawRubberband(HDC hdc, LPRECT newRect, BOOL erase)
{
	
	static BOOL firstDraw = TRUE;
	static RECT lastRect  = {0};
	static RECT clipRect  = {0};
	
	if(firstDraw) {

		ShowWindow(hLayerWnd, SW_SHOW);
		UpdateWindow(hLayerWnd);

		firstDraw = FALSE;
	}

	if (erase) {

		ShowWindow(hLayerWnd, SW_HIDE);
		
	}


	clipRect = *newRect;
	if ( clipRect.right  < clipRect.left ) {
		int tmp = clipRect.left;
		clipRect.left   = clipRect.right;
		clipRect.right  = tmp;
	}
	if ( clipRect.bottom < clipRect.top  ) {
		int tmp = clipRect.top;
		clipRect.top    = clipRect.bottom;
		clipRect.bottom = tmp;
	}
	MoveWindow(hLayerWnd,  clipRect.left, clipRect.top, 
			clipRect.right-  clipRect.left + 1, clipRect.bottom - clipRect.top + 1,true);

	
	return;

}


BOOL convertPNG(LPCTSTR destFile, LPCTSTR srcFile)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;


	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Image *b = new Image(srcFile, 0);

	if (0 == b->GetLastStatus()) {
		if (GetEncoderClsid(L"image/png", &clsidEncoder)) {

			if (0 == b->Save(destFile, &clsidEncoder, 0) ) {

					res = TRUE;
			}
		}
	}


	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}


BOOL savePNG(LPCTSTR fileName, HBITMAP newBMP)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;


	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	

	Bitmap *b = new Bitmap(newBMP, NULL);
	
	if (GetEncoderClsid(L"image/png", &clsidEncoder)) {

		if (0 ==
			b->Save(fileName, &clsidEncoder, 0) ) {

				res = TRUE;
		}
	}
	

	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}


LRESULT CALLBACK LayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	RECT clipRect	= {0, 0, 500, 500};
	HBRUSH hBrush;
	HPEN hPen;
	HFONT hFont;


	switch (message)
	{
	case WM_ERASEBKGND:
		 GetClientRect(hWnd, &clipRect);
		
		hdc = GetDC(hWnd);
        hBrush = CreateSolidBrush(RGB(100,100,100));
        SelectObject(hdc, hBrush);
		hPen = CreatePen(PS_DASH,1,RGB(255,255,255));
		SelectObject(hdc, hPen);
		Rectangle(hdc,0,0,clipRect.right,clipRect.bottom);


		int fHeight;
		fHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		hFont = CreateFont(fHeight,
			0,
			0,                
			0,               
			FW_REGULAR,         
			FALSE,
			FALSE,    
			FALSE,         
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			PROOF_QUALITY,
			FIXED_PITCH | FF_MODERN,
			L"Tahoma");  

		SelectObject(hdc, hFont);

		int iWidth, iHeight;
		iWidth  = clipRect.right  - clipRect.left;
		iHeight = clipRect.bottom - clipRect.top;

		wchar_t sWidth[200], sHeight[200];
		swprintf_s(sWidth, L"%d", iWidth);
		swprintf_s(sHeight, L"%d", iHeight);

		int w,h,h2;
		w = -fHeight * 2.5 + 8;
		h = -fHeight * 2 + 8;
		h2 = h + fHeight;

		SetBkMode(hdc,TRANSPARENT);
		SetTextColor(hdc,RGB(0,0,0));
		TextOut(hdc, clipRect.right-w+1,clipRect.bottom-h+1,(LPCWSTR)sWidth,wcslen(sWidth));
		TextOut(hdc, clipRect.right-w+1,clipRect.bottom-h2+1,(LPCWSTR)sHeight,wcslen(sHeight));
		SetTextColor(hdc,RGB(255,255,255));
		TextOut(hdc, clipRect.right-w,clipRect.bottom-h,(LPCWSTR)sWidth,wcslen(sWidth));
		TextOut(hdc, clipRect.right-w,clipRect.bottom-h2,(LPCWSTR)sHeight,wcslen(sHeight));

		DeleteObject(hPen);
		DeleteObject(hBrush);
		DeleteObject(hFont);
		ReleaseDC(hWnd, hdc);

		return TRUE;

        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	
	static BOOL onClip		= FALSE;
	static BOOL firstDraw	= TRUE;
	static RECT clipRect	= {0, 0, 0, 0};
	
	switch (message)
	{
	case WM_RBUTTONDOWN:

		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_TIMER:

		if (GetKeyState(VK_ESCAPE) & 0x8000){
			DestroyWindow(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSEMOVE:
		if (onClip) {

			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;
			
			hdc = GetDC(NULL);
			drawRubberband(hdc, &clipRect, FALSE);

			ReleaseDC(NULL, hdc);
		}
		break;
	

	case WM_LBUTTONDOWN:
		{

			onClip = TRUE;
			

			clipRect.left = LOWORD(lParam) + ofX;
			clipRect.top  = HIWORD(lParam) + ofY;
			



			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		{

			onClip = FALSE;
			

			ReleaseCapture();
		

			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;


			HDC hdc = GetDC(NULL);


			drawRubberband(hdc, &clipRect, TRUE);

			if ( clipRect.right  < clipRect.left ) {
				int tmp = clipRect.left;
				clipRect.left   = clipRect.right;
				clipRect.right  = tmp;
			}
			if ( clipRect.bottom < clipRect.top  ) {
				int tmp = clipRect.top;
				clipRect.top    = clipRect.bottom;
				clipRect.bottom = tmp;
			}
			

			int iWidth, iHeight;
			iWidth  = clipRect.right  - clipRect.left + 1;
			iHeight = clipRect.bottom - clipRect.top  + 1;

			if(iWidth == 0 || iHeight == 0) {

				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}


			HBITMAP newBMP = CreateCompatibleBitmap(hdc, iWidth, iHeight);
			HDC	    newDC  = CreateCompatibleDC(hdc);
			

			SelectObject(newDC, newBMP);


			BitBlt(newDC, 0, 0, iWidth, iHeight, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);
			

			ShowWindow(hWnd, SW_HIDE);

			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (savePNG(tmpFile, newBMP)) {


				if (!uploadFile(hWnd, tmpFile)) {
					
				}
			} else {

				MessageBox(hWnd, _T("Cannot save png image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}


			DeleteFile(tmpFile);
			
			DeleteDC(newDC);
			DeleteObject(newBMP);

			ReleaseDC(NULL, hdc);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


VOID setClipBoardText(const char* str)
{

	HGLOBAL hText;
	char    *pText;
	size_t  slen;

	slen  = strlen(str) + 1;

	hText = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, slen * sizeof(TCHAR));

	pText = (char *)GlobalLock(hText);
	strncpy_s(pText, slen, str, slen);
	GlobalUnlock(hText);
	

	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hText);
	CloseClipboard();


	GlobalFree(hText);
}


VOID execUrl(const char* str)
{
	size_t  slen;
	size_t  dcount;
	slen  = strlen(str) + 1; 

	TCHAR *wcUrl = (TCHAR *)malloc(slen * sizeof(TCHAR));
	

	mbstowcs_s(&dcount, wcUrl, slen, str, slen);
	

	SHELLEXECUTEINFO lsw = {0};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = wcUrl;

	ShellExecuteEx(&lsw);

	free(wcUrl);
}


std::string getId()
{

    TCHAR idFile[_MAX_PATH];
	TCHAR idDir[_MAX_PATH];

    SHGetSpecialFolderPath( NULL, idFile, CSIDL_APPDATA, FALSE );

	 _tcscat_s( idFile, _T("\\Gyazo"));
	 _tcscpy_s( idDir, idFile);
	 _tcscat_s( idFile, _T("\\id.txt"));

	const TCHAR*	 idOldFile			= _T("id.txt");
	BOOL oldFileExist = FALSE;

	std::string idStr;


	std::ifstream ifs;

	ifs.open(idFile);
	if (! ifs.fail()) {

		ifs >> idStr;
		ifs.close();
	} else{		
		std::ifstream ifsold;
		ifsold.open(idOldFile);
		if (! ifsold.fail()) {

			ifsold >> idStr;
			ifsold.close();
		}
	}

	return idStr;
}


BOOL saveId(const WCHAR* str)
{

    TCHAR idFile[_MAX_PATH];
	TCHAR idDir[_MAX_PATH];

    SHGetSpecialFolderPath( NULL, idFile, CSIDL_APPDATA, FALSE );

	 _tcscat_s( idFile, _T("\\Gyazo"));
	 _tcscpy_s( idDir, idFile);
	 _tcscat_s( idFile, _T("\\id.txt"));

	const TCHAR*	 idOldFile			= _T("id.txt");

	size_t  slen;
	size_t  dcount;
	slen  = _tcslen(str) + 1;

	char *idStr = (char *)malloc(slen * sizeof(char));

	wcstombs_s(&dcount, idStr, slen, str, slen);


	CreateDirectory(idDir,NULL);
	std::ofstream ofs;
	ofs.open(idFile);
	if (! ofs.fail()) {
		ofs << idStr;
		ofs.close();


		if (PathFileExists(idOldFile)){
			DeleteFile(idOldFile);
		}
	}else{
		free(idStr);
		return FALSE;
	}

	free(idStr);
	return TRUE;
}


BOOL uploadFile(HWND hwnd, LPCTSTR fileName)
{
	const TCHAR* UPLOAD_SERVER	= _T("b4c0n.com");
	const TCHAR* UPLOAD_PATH	= _T("/b4c0n.php");

	const char*  sBoundary = "----BOUNDARYBOUNDARY----";		
	const char   sCrLf[]   = { 0xd, 0xa, 0x0 };					
	const TCHAR* szHeader  = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream	buf;
	std::string			idStr;
	
	//
	idStr = getId();


	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"id\"";
	buf << sCrLf;
	buf << sCrLf;
	buf << idStr;
	buf << sCrLf;


	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"imagedata\"; filename=\"gyazo\"";
	buf << sCrLf;

	buf << sCrLf;


	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail()) {
		MessageBox(hwnd, _T("PNG open failed"), szTitle, MB_ICONERROR | MB_OK);
		png.close();
		return FALSE;
	}
	buf << png.rdbuf();	
	png.close();


	buf << sCrLf;
	buf << "--";
	buf << sBoundary;
	buf << "--";
	buf << sCrLf;


	std::string oMsg(buf.str());


	HINTERNET hSession    = InternetOpen(szTitle, 
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot configure wininet"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	

	HINTERNET hConnection = InternetConnect(hSession, 
		UPLOAD_SERVER, INTERNET_DEFAULT_HTTP_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot initiate connection"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}


	HINTERNET hRequest    = HttpOpenRequest(hConnection,
		_T("POST"), UPLOAD_PATH, NULL,
		NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot compose post request"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}


	const TCHAR* ua = _T("User-Agent: Gyazowin/1.0\r\n");
	BOOL bResult = HttpAddRequestHeaders(
		hRequest, ua, _tcslen(ua), 
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	if (FALSE == bResult) {
		MessageBox(hwnd, _T("Cannot set user agent"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	

	if (HttpSendRequest(hRequest,
                    szHeader,
					lstrlen(szHeader),
                    (LPVOID)oMsg.c_str(),
					(DWORD) oMsg.length()))
	{
		
		
		DWORD resLen = 8;
		TCHAR resCode[8];

		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, resCode, &resLen, 0);
		if( _ttoi(resCode) != 200 ) {

			MessageBox(hwnd, _T("Failed to upload (unexpected result code, under maintainance?)"),
				szTitle, MB_ICONERROR | MB_OK);
		} else {

			DWORD idLen = 100;
			TCHAR newid[100];
			
			memset(newid, 0, idLen*sizeof(TCHAR));	
			_tcscpy_s(newid, _T("X-Gyazo-Id"));

			HttpQueryInfo(hRequest, HTTP_QUERY_CUSTOM, newid, &idLen, 0);
			if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND && idLen != 0) {

				saveId(newid);
			}

			DWORD len;
			char  resbuf[1024];
			std::string result;
			

			while(InternetReadFile(hRequest, (LPVOID) resbuf, 1024, &len) 
				&& len != 0)
			{
				result.append(resbuf, len);
			}

			result += '\0';


			setClipBoardText(result.c_str());
			

			execUrl(result.c_str()); 

			return TRUE;
		}
	} else {

		MessageBox(hwnd, _T("Failed to upload"), szTitle, MB_ICONERROR | MB_OK);
	}

	return FALSE;

}