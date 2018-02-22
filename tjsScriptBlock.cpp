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
void tTJSScriptBlock::WarningLog( const tjs_char* message ) {
	tjs_int line = CurrentLine;
	if( Name ) {
		TVPAddLog( ttstr(TJS_W("warning: ")) + Name + TJS_W("(") + ttstr(line) + TJS_W(")") );
	} else {
		tjs_char ptr[128];
		TJS_snprintf(ptr, sizeof(ptr)/sizeof(tjs_char), TJS_W("0x%p"), this);
		ttstr name = ttstr(TJS_W("anonymous@")) + ptr;
		TVPAddLog( ttstr(TJS_W("warning: ")) + name + TJS_W("(") + ttstr(line) + TJS_W(")") );
	}
}
void tTJSScriptBlock::CreateCurrentTagDic() {
	if( CurrentTagDic ) {
		CurrentTagDic->Release();
	}
	CurrentTagDic = TJSCreateArrayObject();
}
void tTJSScriptBlock::CrearCurrentTag() {
	if( CurrentTagDic ) {
		CurrentTagDic->Release();
		CurrentTagDic = nullptr;
	}
}
void tTJSScriptBlock::SetCurrentTagName( const ttstr& name ) {
	if( !CurrentTagDic ) {
		CurrentTagDic = TJSCreateArrayObject();
	}
	tTJSVariant tmp(name);
	CurrentTagDic->PropSetByVS( TJS_MEMBERENSURE, __name_name.AsVariantStringNoAddRef(), &tmp, CurrentTagDic );
}
void tTJSScriptBlock::PushCurrentTag() {
	if( CurrentTagDic ) {
		tTJSVariant tmp(CurrentTagDic, CurrentTagDic);
		CurrentTagDic->Release();
		CurrentTagDic = nullptr;
		PushValueCurrentLine( tmp );
	}
}
void tTJSScriptBlock::PushNameTag( const ttstr& name ) {
	SetCurrentTagName( name );
	PushCurrentTag();
}
void tTJSScriptBlock::ParseTransition() {
	CreateTagDic();
	SetCurrentTagName( __endtrans_name );

	LineAttribute = true;
	try {
		tjs_int value;
		Token token = LexicalAnalyzer.GetInTagToken( value );
		if( token == Token::SYMBOL ) {
			GetValue
			token = LexicalAnalyzer.GetInTagToken( value );
		}
		do {
			token
			
		} while( true );

	} catch(...) {
		LineAttribute = false;
		throw;
	}
	LineAttribute = false;
}
bool ParseTag( tjs_int token, tjs_int value ) {
	if( token == Token::EOL ) return false;

	switch( token ) {
	case Token::TEXT:
		PushValueCurrentLine( LexicalAnalyzer.GetValue(value) );
		return true;

	case Token::BEGIN_TAG:
		ParseTag();
		return true;

	case Token::LINE_COMMENTS:
		
		return false;
	}
}
//---------------------------------------------------------------------------
/**
 * 返却するデータ形式
 * 文字列は文字列型でそのまま、記号については記号番号を、タグや属性は辞書型で
 * Array型に1行ずつ格納する
 * 各行もArray型で続く
 *
 * 文字列はそのまま文字列として
 * 数値型の時は、意味を
 * タグや属性は辞書型で
 */
void tTJSScriptBlock::ParseLine( tjs_int line ) {
	if( line < LineVector.size() ) {
		tjs_int length;
		const tjs_char *str = GetLine( line, &length );
		if( length == 0 ) {
			// 改行のみ
		} else {
			LexicalAnalyzer.reset( str, length );
			tjs_int value;
			tjs_int token = LexicalAnalyzer.GetFirstToken( value );
			if( PrevSelectLine ) {
				
			}

			// first token
			switch( token ) {
			case Token::BEGIN_TRANS:	// >>> 
				// トランジション開始以降の文字列は無視し、begintransタグを格納するのみ
				PushNameTag( __begintrans_name );
				return;
			case Token::END_TRANS: {	// <<<
				ParseTransition();
				break;
				}
			case Token::AT:	// @
				ParseCharacter();
				break;
			case Token::LABEL:	// # タグ
				ParseLabel();
				break;
			case Token::SELECT:	// [0-9]+\.
				// value : select number.
				ParseSelect();
				HasSelectLine = true;
				break;
			case Token::NEXT_SCENARIO:	// >
				ParseNextScenario();
				break;
			default:
				if( ParseTag( token, value ) == false ) return;
				break;
			}
		}
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
