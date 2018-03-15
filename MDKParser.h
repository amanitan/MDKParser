
#ifndef __MDK_PARSER_H__
#define __MDK_PARSER_H__


#ifdef _WIN32
#include <windows.h>
#endif
#include "tp_stub.h"
#include <memory>


//---------------------------------------------------------------------------
// tTJSNI_MDKParser
//---------------------------------------------------------------------------
class tTJSNI_MDKParser : public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

	std::unique_ptr<class Parser> Script;

public:
	tTJSNI_MDKParser();
	virtual ~tTJSNI_MDKParser();
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

	iTJSDispatch2 * ParseMDKScenario( const ttstr& storage );

private:
	iTJSDispatch2 * Owner = nullptr; // owner object

};


#endif // __MDK_PARSER_H__
