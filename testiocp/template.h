#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "resource.h"
#include <winuser.h>

using namespace std;
// defined with this macro as being exported.
#ifdef DNF_EXPORTS
#define TEMPLATE_API __declspec(dllexport)
#else
#define TEMPLATE_API __declspec(dllimport)
#endif


class TEMPLATE_API CDnf {
public:
	CDnf(void);
	// TODO: add your methods here.
};

extern TEMPLATE_API int nDnf;

TEMPLATE_API int fnDnf(void);




