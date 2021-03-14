#include "SignUp.h"
#include "resource.h"

BOOL CALLBACK SignUpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	case WM_COMMAND:
		//switch (LOWORD(wParam))
		//{
		//	/*case IDOK:
		//	case IDCANCEL:
		//		EndDialog(hDlgMain, 0);
		//		return TRUE;*/
		//}

		break;
		return FALSE;

		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}