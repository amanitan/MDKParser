/**
 * TJS2 の字句抽出器をベースにMDKParser用の字句抽出器を作る。
 */

//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Block Management
//---------------------------------------------------------------------------
#ifndef tjsScriptBlockH
#define tjsScriptBlockH


#ifdef _WIN32
#include <windows.h>
#endif
#include "tp_stub.h"
#include "Token.h"

#include "tjsLex.h"

#include <list>
#include <memory>

/**
 * tagは以下のような辞書形式で格納されている
%[
	name : "tag name",	// tag名
	command : [
		"name",	// 属性値がない場合は、コマンドとして格納される
	]
	attribute : %[		// 存在しない時はattribute要素自体ない(属性)
		attrname : "value",
		attrname : %[ ref : "name" ],		// 変数参照の時
		attrname : %[  file : "name", prop : "name" ],	// ファイル参照の時
	],
	parameter : %[		// 存在しない時はparameter要素自体ない
		name :  "value",
	],
]

%[
	lines : [
		[ 1 ],		// 記号
		[ "text" ],	// テキストはそのまま格納
		[ %[name:"tag"] ],	// タグ
		[ %[name:"ruby",text:"きりきり"], "吉里吉里", %[name:"endruby"] ],
	]
]

 */
 
//---------------------------------------------------------------------------
// tTJSScriptBlock - a class for managing the script block
//---------------------------------------------------------------------------
class tTJSScriptBlock
{
	ttstr __endtrans_name(TJSMapGlobalStringMap(TJS_W("endtrans")));
	ttstr __begintrans_name(TJSMapGlobalStringMap(TJS_W("begintrans")));

	ttstr __storage_name(TJSMapGlobalStringMap(TJS_W("storage")));
	ttstr __name_name(TJSMapGlobalStringMap(TJS_W("name")));
	ttstr __value_name(TJSMapGlobalStringMap(TJS_W("value")));

	ttstr __attribute_name(TJSMapGlobalStringMap(TJS_W("attribute")));
	ttstr __ref_name(TJSMapGlobalStringMap(TJS_W("ref")));
	ttstr __file_name(TJSMapGlobalStringMap(TJS_W("file")));
	ttstr __prop_name(TJSMapGlobalStringMap(TJS_W("prop")));

	ttstr __trans_name(TJSMapGlobalStringMap(TJS_W("trans")));

	enum class LogType {
		Warning,
		Error,
	};
public:
	tTJSScriptBlock();
	virtual ~tTJSScriptBlock();

private:
	tjs_int RefCount;
	tjs_char *Script;
	tjs_char *Name;
	tjs_int LineOffset;
	bool PrevSelectLine = false;	// 直前の行に選択肢があった
	bool LineAttribute = false;		// 1行で属性を書くスタイルの状態時true

	iTJSDispatch2* CurrentTagDic = nullptr; // DictionaryObject
	iTJSDispatch2* CurrentAttributeDic = nullptr;
	iTJSDispatch2* CurrentParameterDic = nullptr;
	iTJSDispatch2* CurrentCommandArray = nullptr;

	std::unique_ptr<tTJSLexicalAnalyzer> LexicalAnalyzer;

	std::vector<tjs_int> LineVector;
	std::vector<tjs_int> LineLengthVector;

	tTJSString FirstError;
	tjs_int FirstErrorPos;

public:
	tjs_int CompileErrorCount;

	void AddRef();
	void Release();

	const tjs_char * GetLine(tjs_int line, tjs_int *linelength) const;
	tjs_int SrcPosToLine(tjs_int pos) const;
	tjs_int LineToSrcPos(tjs_int line) const;

	ttstr GetLineDescriptionString(tjs_int pos) const;

	const tjs_char *GetScript() const { return Script; }

	void Parse(const tjs_char *script, bool isexpr, bool resultneeded);
	void ParseLine( tjs_int line );

	void SetFirstError(const tjs_char *error, tjs_int pos);

	tTJSLexicalAnalyzer * GetLexicalAnalyzer() { return LexicalAnalyzer.get(); }

	const tjs_char *GetName() const { return Name; }
	void SetName(const tjs_char *name, tjs_int lineofs);
	ttstr GetNameInfo() const;

	tjs_int GetLineOffset() const { return LineOffset; }

	void WarningLog( const tjs_char* message );
	void ErrorLog( const tjs_char* message );
	void Log( LogType type, const tjs_char* message );

private:
	static void ConsoleOutput(const tjs_char *msg, void *data);

	void CreateCurrentTagDic();
	void CrearCurrentTag();
	void SetCurrentTagName( const ttstr& name );
	void PushCurrentTag();
	void PushNameTag( const ttstr& name );
	void PushValueCurrentLine( const tTJSVariant& val );

public:
	void SetText(tTJSVariant *result, const tjs_char *text, iTJSDispatch2 * context, bool isexpression);
};
//---------------------------------------------------------------------------

#endif