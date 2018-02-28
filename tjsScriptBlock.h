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
	type : "tag",
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
 * label の時は以下のような辞書形式で行に直接格納されている
%[
	type : "label",
	name : "name",
	description : "desc"
]
 * > の時は以下のような形で行に直接格納されている。
%[
	type : "next",
	target : "filename",
	cond : "flag == true",
]
 * コメントの時
%[
	type : "comment"
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
	ttstr __type_name(TJSMapGlobalStringMap(TJS_W("type")));
	ttstr __name_name(TJSMapGlobalStringMap(TJS_W("name")));
	ttstr __value_name(TJSMapGlobalStringMap(TJS_W("value")));

	ttstr __tag_name(TJSMapGlobalStringMap(TJS_W("tag")));
	ttstr __label_name(TJSMapGlobalStringMap(TJS_W("label")));
	ttstr __select_name(TJSMapGlobalStringMap(TJS_W("select")));
	ttstr __next_name(TJSMapGlobalStringMap(TJS_W("next")));

	ttstr __attribute_name(TJSMapGlobalStringMap(TJS_W("attribute")));
	ttstr __parameter_name(TJSMapGlobalStringMap(TJS_W("parameter")));
	ttstr __command_name(TJSMapGlobalStringMap(TJS_W("command")));
	ttstr __ref_name(TJSMapGlobalStringMap(TJS_W("ref")));
	ttstr __file_name(TJSMapGlobalStringMap(TJS_W("file")));
	ttstr __prop_name(TJSMapGlobalStringMap(TJS_W("prop")));

	ttstr __trans_name(TJSMapGlobalStringMap(TJS_W("trans")));
	ttstr __charname_name(TJSMapGlobalStringMap(TJS_W("charname")));
	ttstr __alias_name(TJSMapGlobalStringMap(TJS_W("alias")));
	ttstr __description_name(TJSMapGlobalStringMap(TJS_W("description")));
	ttstr __text_name(TJSMapGlobalStringMap(TJS_W("text")));
	ttstr __image_name(TJSMapGlobalStringMap(TJS_W("image")));
	ttstr __target_name(TJSMapGlobalStringMap(TJS_W("target")));
	ttstr __if_name(TJSMapGlobalStringMap(TJS_W("if")));
	ttstr __cond_name(TJSMapGlobalStringMap(TJS_W("cond")));
	ttstr __comment_name(TJSMapGlobalStringMap(TJS_W("comment")));

	ttstr __voice_name(TJSMapGlobalStringMap(TJS_W("voice")));
	ttstr __time_name(TJSMapGlobalStringMap(TJS_W("time")));
	ttstr __wait_name(TJSMapGlobalStringMap(TJS_W("time")));
	ttstr __fade_name(TJSMapGlobalStringMap(TJS_W("time")));

	enum class LogType {
		Warning,
		Error,
	};

	static std::map<Token,ttstr>	TagCommandPair;
	static std::map<tjs_char,Token>		SignToToken;
public:
	tTJSScriptBlock();
	virtual ~tTJSScriptBlock();

private:
	tjs_int RefCount;
	tjs_char *Script;
	tjs_char *Name;
	tjs_int LineOffset;
	tjs_int CurrentLine;
	bool PrevSelectLine = false;	// 直前の行に選択肢があった
	bool LineAttribute = false;		// 1行で属性を書くスタイルの状態時true
	bool MultiLineTag = false;

	iTJSDispatch2* CurrentDic = nullptr; // DictionaryObject
	iTJSDispatch2* CurrentAttributeDic = nullptr;
	iTJSDispatch2* CurrentParameterDic = nullptr;
	iTJSDispatch2* CurrentCommandArray = nullptr;

	iTJSDispatch2* ScenarioLines = nullptr;

	iTJSDispatch2* ArrayAddFunc = nullptr;

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

	static void Initialize();
	static void AddSignWord( tjs_char sign, const ttstr& word );
private:
	static void ConsoleOutput(const tjs_char *msg, void *data);

	void CreateCurrentTagDic();
	void CreateCurrentLabelDic();
	void CrearCurrentTag();
	void SetCurrentTagName( const ttstr& name );
	void SetCurrentLabelName( const tTJSVariant& val );
	void SetValueToCurrentDic( const ttstr& name, tTJSVariant& val );
	void SetCurrentLabelDescription( const ttstr& desc );
	void PushCurrentTag();
	void PushCurrentLabel();
	void PushNameTag( const ttstr& name );
	void PushValueCurrentLine( const tTJSVariant& val );
	void AddValueToLine( const tTJSVariant& val );
	void AddCurrentDicToLine();
	void PushAttribute( const ttstr& name, const tTJSVariant& value, bool isparameter=false );
	void PushAttribute( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter=false );
	void PushAttributeReference( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter=false );
	void PushAttributeFileProperty( const tTJSVariantString& name, const tTJSVariant& file, const tTJSVariant& prop, bool isparameter=false );
	void PushTagCommand( const ttstr& name );

	void ParseAttributeValueSymbol( const tTJSVariant& symbol, const tTJSVariant& valueSymbol, bool isparameter=false );
	void ParseAttribute( const tTJSVariant& symbol, bool isparameter=false );
	bool ParseSpecialAttribute( Token token, tjs_int value );
	void ParseTag();
	void ParseAttributes();
	void ParseTransition();
	void ParseCharacter();
	void ParseLabel();
	void ParseSelect( tjs_int number );
	void ParseNextScenario();
	bool ParseTag( tjs_int token, tjs_int value );
	void ParseLine( tjs_int line );

	static ttstr* GetTagSignWord( Token token );
public:
	void SetText(tTJSVariant *result, const tjs_char *text, iTJSDispatch2 * context, bool isexpression);
};
//---------------------------------------------------------------------------

#endif