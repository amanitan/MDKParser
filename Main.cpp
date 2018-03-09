

#ifdef _WIN32
#include <windows.h>
#endif
#include "tp_stub.h"
#include "MDKParser.h"

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#define STDCALL __stdcall
#else
typedef tjs_error HRESULT;
#define DLL_EXPORT
#endif


#define TJS_NATIVE_CLASSID_NAME ClassID_MDKParser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_MDKParser() {
	return new tTJSNI_MDKParser();
}
//---------------------------------------------------------------------------
iTJSDispatch2 * TVPCreateNativeClass_MDKParser() {
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("MDKParser"), Create_NI_MDKParser);

	TJS_BEGIN_NATIVE_MEMBERS( MDKParser )
	TJS_DECL_EMPTY_FINALIZE_METHOD

	//----------------------------------------------------------------------
	// constructor/methods
	//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_MDKParser, /*TJS class name*/MDKParser ) {
		return TJS_S_OK;
	}
	TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/MDKParser )

	//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/loadScenario ) {
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MDKParser );
		if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;
		if( result ) {
			iTJSDispatch2* ret = _this->ParseMDKScenario( *param[0] );
			*result = tTJSVariant( ret, ret );
			ret->Release();
		}
		return TJS_S_OK;
	}
	TJS_END_NATIVE_METHOD_DECL(/*func. name*/loadScenario )
//----------------------------------------------------------------------

//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS
	return classobj;
}
//---------------------------------------------------------------------------
#ifdef _WIN32
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved) {
	return 1;
}
#endif
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" DLL_EXPORT HRESULT STDCALL V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	{
		//-----------------------------------------------------------------------
		iTJSDispatch2 * tjsclass = TVPCreateNativeClass_MDKParser();
		val = tTJSVariant(tjsclass);
		tjsclass->Release();
		global->PropSet( TJS_MEMBERENSURE, TJS_W("MDKParser"), nullptr, &val, global );
		//-----------------------------------------------------------------------
	}
	// - global を Release する
	global->Release();

	// val をクリアする。
	// これは必ず行う。そうしないと val が保持しているオブジェクト
	// が Release されず、次に使う TVPPluginGlobalRefCount が正確にならない。
	val.Clear();


	// この時点での TVPPluginGlobalRefCount の値を
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	// として控えておく。TVPPluginGlobalRefCount はこのプラグイン内で
	// 管理されている tTJSDispatch 派生オブジェクトの参照カウンタの総計で、
	// 解放時にはこれと同じか、これよりも少なくなってないとならない。
	// そうなってなければ、どこか別のところで関数などが参照されていて、
	// プラグインは解放できないと言うことになる。

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
extern "C" DLL_EXPORT HRESULT STDCALL V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合は
	// この時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも
	// 大きくなっていれば失敗ということにする。
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return TJS_E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	/*
		ただし、クラスの場合、厳密に「オブジェクトが使用中である」ということを
		知るすべがありません。基本的には、Plugins.unlink によるプラグインの解放は
		危険であると考えてください (いったん Plugins.link でリンクしたら、最後ま
		でプラグインを解放せず、プログラム終了と同時に自動的に解放させるのが吉)。
	*/

	// プロパティ開放
	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// メニューは解放されないはずなので、明示的には解放しない

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if(global)
	{
		// TJS 自体が既に解放されていたときなどは
		// global は NULL になり得るので global が NULL でない
		// ことをチェックする

		global->DeleteMember( 0, TJS_W("MDKParser"), nullptr, global );
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
