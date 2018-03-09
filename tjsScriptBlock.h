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
#include <map>

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
	ttstr __endtrans_name;
	ttstr __begintrans_name;

	ttstr __storage_name;
	ttstr __type_name;
	ttstr __name_name;
	ttstr __value_name;

	ttstr __tag_name;
	ttstr __label_name;
	ttstr __select_name;
	ttstr __next_name;
	ttstr __selopt_name;

	ttstr __attribute_name;
	ttstr __parameter_name;
	ttstr __command_name;
	ttstr __ref_name;
	ttstr __file_name;
	ttstr __prop_name;

	ttstr __trans_name;
	ttstr __charname_name;
	ttstr __alias_name;
	ttstr __description_name;
	ttstr __text_name;
	ttstr __image_name;
	ttstr __target_name;
	ttstr __if_name;
	ttstr __cond_name;
	ttstr __comment_name;

	ttstr __voice_name;
	ttstr __time_name;
	ttstr __wait_name;
	ttstr __fade_name;

	ttstr __lines_name;

	enum class LogType {
		Warning,
		Error,
	};

	std::map<Token,ttstr>			TagCommandPair;
	std::map<tjs_char,Token>		SignToToken;

public:
	tTJSScriptBlock();
	virtual ~tTJSScriptBlock();

private:
	std::unique_ptr<tjs_char[]> Script;
	std::unique_ptr<tjs_char[]> Name;
	tjs_int LineOffset = 0;
	tjs_int CurrentLine = 0;
	bool LineAttribute = false;		// 1行で属性を書くスタイルの状態時true
	bool MultiLineTag = false;
	bool HasSelectLine = false;

	iTJSDispatch2* CurrentDic = nullptr; // DictionaryObject
	iTJSDispatch2* CurrentAttributeDic = nullptr;
	iTJSDispatch2* CurrentParameterDic = nullptr;
	iTJSDispatch2* CurrentCommandArray = nullptr;

	iTJSDispatch2* ScenarioLines = nullptr;
	iTJSDispatch2* CurrentLineArray = nullptr;
	iTJSDispatch2* ArrayAddFunc = nullptr;

	std::unique_ptr<tTJSLexicalAnalyzer> LexicalAnalyzer;

	std::vector<tjs_int> LineVector;
	std::vector<tjs_int> LineLengthVector;

	tTJSString FirstError;
	tjs_int FirstErrorPos;
	tjs_int CompileErrorCount;

public:
	const tjs_char * GetLine(tjs_int line, tjs_int *linelength) const;
	tjs_int SrcPosToLine(tjs_int pos) const;
	tjs_int LineToSrcPos(tjs_int line) const;

	ttstr GetLineDescriptionString(tjs_int pos) const;

	const tjs_char *GetScript() const { return Script.get(); }

	tTJSLexicalAnalyzer * GetLexicalAnalyzer() { return LexicalAnalyzer.get(); }

	const tjs_char *GetName() const { return Name.get(); }
	void SetName(const tjs_char *name, tjs_int lineofs);

	tjs_int GetLineOffset() const { return LineOffset; }

	void WarningLog( const tjs_char* message );
	void ErrorLog( const tjs_char* message );
	void Log( LogType type, const tjs_char* message );

	void Initialize();
	void AddSignWord( tjs_char sign, const ttstr& word );

private:
	static void ConsoleOutput(const tjs_char *msg, void *data);

	/** 指定されたタイプ名の辞書を生成する。 */
	void CreateCurrentDic( const tTJSVariantString& name );
	/** 新たに辞書を生成する。 */
	void CreateCurrentTagDic();
	/** ラベルとして新たに辞書を生成する。 */
	void CreateCurrentLabelDic();

	/** 現在の辞書やタグに関連する要素をクリアする。 */
	void CrearCurrentTag();

	/** 現在の行に直接値を格納する。 */
	void AddValueToLine( const tTJSVariant& val );

	/** 現在の行に配列の要素として指定された値を追加する。 */
	void PushValueCurrentLine( const tTJSVariant& val );
	/** 現在の辞書に名前を設定する。 */
	void SetCurrentTagName( const ttstr& name );
	/** 現在の辞書をタグとして現在の行に追加する */
	void PushCurrentTag();
	/** 現在の辞書を現在の行に直接格納する。(ラベルに使用) */
	void PushCurrentLabel();
	/** 指定された名前のタグを現在の行に追加する。 */
	void PushNameTag( const ttstr& name );
	/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に値を設定する。 */
	void PushAttribute( const ttstr& name, const tTJSVariant& value, bool isparameter = false );
	/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に値を設定する。 */
	void PushAttribute( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter = false );
	/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に参照を設定する。 */
	void PushAttributeReference( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter = false );
	/** 指定された名前で現在の辞書の属性(もしくはパラメータ)にファイルプロパティを設定する。 */
	void PushAttributeFileProperty( const tTJSVariantString& name, const tTJSVariant& file, const tTJSVariant& prop, bool isparameter = false );
	/** 現在のタグにコマンドを追加する。 */
	void PushTagCommand( const ttstr& name );
	//void SetCurrentLabelName( const tTJSVariant& val );
	/** 現在の辞書にラベル詳細として文字列を設定する */
	void SetCurrentLabelDescription( const ttstr& desc );
	/** 現在の辞書に指定された名前で値を設定する */
	void SetValueToCurrentDic( const ttstr& name, const tTJSVariant& val );
	/** 現在の辞書を現在の行に直接格納する。 */
	void AddCurrentDicToLine();

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
	bool ParseTag( Token token, tjs_int value );
	void ParseLine( tjs_int line );

	ttstr* GetTagSignWord( Token token );
public:
	iTJSDispatch2* ParseText( const tjs_char* text );
};
//---------------------------------------------------------------------------

#endif