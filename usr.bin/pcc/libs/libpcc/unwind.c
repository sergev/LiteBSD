#include <stdlib.h>
#include <inttypes.h>

struct _Unwind_Context;
struct _Unwind_Exception;

typedef unsigned int _Unwind_Ptr;
typedef unsigned int _Unwind_Word;

typedef enum
{
	_URC_NO_REASON,
	_URC_FOREIGN_EXCEPTION_CAUGHT = 1,
	_URC_FATAL_PHASE2_ERROR = 2,
	_URC_FATAL_PHASE1_ERROR = 3,
	_URC_NORMAL_STOP = 4,
	_URC_END_OF_STACK = 5,
	_URC_HANDLER_FOUND = 6,
	_URC_INSTALL_CONTEXT = 7,
	_URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;

typedef enum {
	_UA_SEARCH_PHASE = 1,
	_UA_CLEANUP_PHASE = 2,
	_UA_HANDLER_FRAME = 4,
	_UA_FORCE_UNWIND = 8,
	_UA_END_OF_STACK = 16
} _Unwind_Action;

typedef void (*_Unwind_Trace_Fn)(void);


void
_Unwind_Resume(struct _Unwind_Exception *object)
{
	abort();
}

_Unwind_Ptr
_Unwind_GetIP(struct _Unwind_Context *context)
{
	abort();

	return 0;
}

_Unwind_Word
_Unwind_GetGR(struct _Unwind_Context *context, int index)
{
	abort();

	return 0;
}

_Unwind_Reason_Code
_Unwind_Backtrace(_Unwind_Trace_Fn trace, void * trace_argument)
{
	abort();

	return _URC_NO_REASON;
}

_Unwind_Word
_Unwind_GetCFA(struct _Unwind_Context * context)
{
	abort();

	return 0;
}

_Unwind_Reason_Code
_Unwind_RaiseException(struct _Unwind_Exception *object)
{
	abort();
	return 0;
}

_Unwind_Reason_Code
_Unwind_Resume_or_Rethrow(struct _Unwind_Exception *object)
{
	abort();
	return 0;
}

void
_Unwind_DeleteException (struct _Unwind_Exception *object)
{
	abort();
}

void
_Unwind_SetIP(struct _Unwind_Context *context, _Unwind_Ptr val)
{
	abort();
}

void
_Unwind_SetGR(struct _Unwind_Context *context, int index, _Unwind_Word val) 
{
	abort();
}

void *
_Unwind_GetLanguageSpecificData(struct _Unwind_Context *context)
{
	abort();
	return 0;
}

_Unwind_Ptr
_Unwind_GetRegionStart(struct _Unwind_Context *context)
{
	abort();
	return 0;
}

_Unwind_Ptr
_Unwind_GetDataRelBase(struct _Unwind_Context *context)
{
	abort();
	return 0;
}

_Unwind_Ptr
_Unwind_GetTextRelBase(struct _Unwind_Context *context)
{
	abort();
	return 0;
}

_Unwind_Reason_Code
__gcc_personality_v0(int version, _Unwind_Action actions, uint64_t exceptionClass, struct _Unwind_Exception *exceptionObject, struct _Unwind_Context *context)
{
	abort();

	return _URC_NO_REASON;
}
