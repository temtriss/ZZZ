// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.
//

#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <string>

#include <Winsock2.h>
#include <mswsock.h>
#include <crtdbg.h>
#include <windows.h>

// C의 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>

#include "MDebug.h"
#include "MXml.h"
#include "MTypes.h"
#include "MTString.h"

#include <tchar.h>
#include <atlconv.h>
#include <atlexcept.h>

#include "crlLib.h"
using namespace crl;
