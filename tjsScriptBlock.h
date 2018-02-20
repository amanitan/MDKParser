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

//---------------------------------------------------------------------------
// tTJSScriptBlock - a class for managing the script block
//---------------------------------------------------------------------------
class tTJSScriptBlock
{
public:
	tTJSScriptBlock();
	virtual ~tTJSScriptBlock();

private:
	tjs_int RefCount;
	tjs_char *Script;
	tjs_char *Name;
	tjs_int LineOffset;

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

private:
	static void ConsoleOutput(const tjs_char *msg, void *data);

public:
	void SetText(tTJSVariant *result, const tjs_char *text, iTJSDispatch2 * context, bool isexpression);
};
//---------------------------------------------------------------------------

#endif