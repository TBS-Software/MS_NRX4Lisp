// NRX4Lisp.h : main header file for the NRX4Lisp DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "pch.h"		// main symbols
#include "Actions.hpp"

const TCHAR* cstrCommandGroup = _T("MS_NRX");

extern "C" __declspec(dllexport)AcRx::AppRetCode
acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
    switch (msg) {
    case AcRx::kInitAppMsg:
        //unlock the application
        acrxDynamicLinker->unlockApplication(pkt);
        acrxDynamicLinker->registerAppMDIAware(pkt);

    case AcRx::kInvkSubrMsg:
        Actions::RecognizeFunction();
        break;
    case AcRx::kLoadDwgMsg:
        Actions::LoadFunctions();
        break;
    case AcRx::kUnloadAppMsg:
        acedRegCmds->removeGroup(cstrCommandGroup);
        break;
    }
    return AcRx::kRetOK;
}