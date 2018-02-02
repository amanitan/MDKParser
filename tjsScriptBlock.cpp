//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Block Management
//---------------------------------------------------------------------------
#include "tjsScriptBlock.h"

static void TJSReportExceptionSource( const ttstr &msg, tTJSScriptBlock *block, tjs_int srcpos ) {
	//if( TJSEnableDebugMode )
	{
		TVPAddLog( ( msg + TJS_W( " at " ) + block->GetLineDescriptionString( srcpos ) ).c_str() );
	}
}
//---------------------------------------------------------------------------
void TJS_eTJSCompileError( const ttstr & msg, tTJSScriptBlock *block, tjs_int srcpos ) {
	TJSReportExceptionSource( msg, block, srcpos );
	TVPThrowExceptionMessage( ( msg + TJS_W( " at " ) + block->GetLineDescriptionString( srcpos ) ).c_str() );
}
//---------------------------------------------------------------------------
void TJS_eTJSCompileError( const tjs_char *msg, tTJSScriptBlock *block, tjs_int srcpos ) {
	TJSReportExceptionSource( msg, block, srcpos );
	TVPThrowExceptionMessage( ( ttstr(msg) + TJS_W( " at " ) + block->GetLineDescriptionString( srcpos ) ).c_str() );
}
//---------------------------------------------------------------------------
// tTJSScriptBlock
//---------------------------------------------------------------------------
tTJSScriptBlock::tTJSScriptBlock() : RefCount(1), Script(NULL), Name(NULL), LineOffset(0) {
}
//---------------------------------------------------------------------------
tTJSScriptBlock::~tTJSScriptBlock()
{
	if(Script) delete [] Script;
	if(Name) delete [] Name;
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::AddRef(void)
{
	RefCount ++;
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::Release(void)
{
	if(RefCount <= 1)
		delete this;
	else
		RefCount--;
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::SetName(const tjs_char *name, tjs_int lineofs)
{
	if(Name) delete [] Name, Name = NULL;
	if(name)
	{
		LineOffset = lineofs;
		Name = new tjs_char[ TJS_strlen(name) + 1];
		TJS_strcpy(Name, name);
	}
}
//---------------------------------------------------------------------------
const tjs_char * tTJSScriptBlock::GetLine(tjs_int line, tjs_int *linelength) const
{
	if( Script == NULL ) {
		*linelength = 10;
		return TJS_W("Bytecode.");
	}
	// note that this function DOES matter LineOffset
	line -= LineOffset;
	if(linelength) *linelength = LineLengthVector[line];
	return Script + LineVector[line];
}
//---------------------------------------------------------------------------
tjs_int tTJSScriptBlock::SrcPosToLine(tjs_int pos) const
{
	tjs_uint s = 0;
	tjs_uint e = (tjs_uint)LineVector.size();
	while(true)
	{
		if(e-s <= 1) return s + LineOffset; // LineOffset is added
		tjs_uint m = s + (e-s)/2;
		if(LineVector[m] > pos)
			e = m;
		else
			s = m;
	}
}
//---------------------------------------------------------------------------
tjs_int tTJSScriptBlock::LineToSrcPos(tjs_int line) const
{
	// assumes line is added by LineOffset
	line -= LineOffset;
	return LineVector[line];
}
//---------------------------------------------------------------------------
ttstr tTJSScriptBlock::GetLineDescriptionString(tjs_int pos) const
{
	// get short description, like "mainwindow.tjs(321)"
	// pos is in character count from the first of the script
	tjs_int line =SrcPosToLine(pos)+1;
	ttstr name;
	if(Name)
	{
		name = Name;
	}
	else
	{
		tjs_char ptr[128];
		TJS_snprintf(ptr, sizeof(ptr)/sizeof(tjs_char), TJS_W("0x%p"), this);
		name = ttstr(TJS_W("anonymous@")) + ptr;
	}

	return name + TJS_W("(") + ttstr(line) + TJS_W(")");
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::ConsoleOutput(const tjs_char *msg, void *data)
{
	TVPAddLog( msg );
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::SetText(tTJSVariant *result, const tjs_char *text,
	iTJSDispatch2 * context, bool isexpression)
{
	TJS_F_TRACE("tTJSScriptBlock::SetText");


	// compiles text and executes its global level scripts.
	// the script will be compiled as an expression if isexpressn is true.
	if(!text) return;
	if(!text[0]) return;

	TJS_D((TJS_W("Counting lines ...\n")))

	Script = new tjs_char[TJS_strlen(text)+1];
	TJS_strcpy(Script, text);

	// calculation of line-count
	tjs_char *ls = Script;
	tjs_char *p = Script;
	while(*p)
	{
		if(*p == TJS_W('\r') || *p == TJS_W('\n'))
		{
			LineVector.push_back(int(ls - Script));
			LineLengthVector.push_back(int(p - ls));
			if(*p == TJS_W('\r') && p[1] == TJS_W('\n')) p++;
			p++;
			ls = p;
		}
		else
		{
			p++;
		}
	}

	if(p!=ls)
	{
		LineVector.push_back(int(ls - Script));
		LineLengthVector.push_back(int(p - ls));
	}

	try
	{
		// parse
		Parse(text, isexpression, result != NULL);

	}
	catch(...)
	{
		throw;
	}
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::Parse(const tjs_char *script, bool isexpr, bool resultneeded)
{
	if(!script) return;

	CompileErrorCount = 0;
	LexicalAnalyzer.reset( new tTJSLexicalAnalyzer(this, script, isexpr, resultneeded) );
	try
	{
		yyparse(this);
	}
	catch(...)
	{
		LexicalAnalyzer.reset();
		throw;
	}
	LexicalAnalyzer.reset();

	if(CompileErrorCount)
	{
		TJS_eTJSCompileError(FirstError, this, FirstErrorPos);
	}
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::SetFirstError(const tjs_char *error, tjs_int pos)
{
	if(CompileErrorCount == 0)
	{
		FirstError = error;
		FirstErrorPos = pos;
	}
}
//---------------------------------------------------------------------------
ttstr tTJSScriptBlock::GetNameInfo() const
{
	if(LineOffset == 0)
	{
		return ttstr(Name);
	}
	else
	{
		return ttstr(Name) + TJS_W("(line +") + ttstr(LineOffset) + TJS_W(")");
	}
}
//---------------------------------------------------------------------------
