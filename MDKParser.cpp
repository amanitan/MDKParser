

#include "MDKParser.h"




tTJSNI_MDKParser::tTJSNI_MDKParser() {
}


//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_MDKParser::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
	tjs_error hr = inherited::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;

	Owner = tjs_obj;
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_MDKParser::Invalidate() {
	Owner = nullptr;
	inherited::Invalidate();
}

