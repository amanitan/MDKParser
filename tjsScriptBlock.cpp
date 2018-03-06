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
void tTJSScriptBlock::Initialize() {
	TagCommandPair.insert(std::make_pair(Token::PLUS, ttstr(TJS_W("add"))));
	TagCommandPair.insert(std::make_pair(Token::MINUS, ttstr(TJS_W("del"))));
	TagCommandPair.insert(std::make_pair(Token::ASTERISK, ttstr(TJS_W("all"))));
	TagCommandPair.insert(std::make_pair(Token::SHARP, ttstr(TJS_W("sync"))));
	TagCommandPair.insert(std::make_pair(Token::EXCRAMATION, ttstr(TJS_W("nosync"))));
	TagCommandPair.insert(std::make_pair(Token::AMPERSAND, ttstr(TJS_W("nowait"))));

	//SignToToken.insert(std::make_pair(TJS_W('>'),Token::GT));
	//SignToToken.insert(std::make_pair(TJS_W('<'),Token::LT));
	SignToToken.insert(std::make_pair(TJS_W('='),Token::EQUAL));
	SignToToken.insert(std::make_pair(TJS_W('!'),Token::EXCRAMATION));
	SignToToken.insert(std::make_pair(TJS_W('&'),Token::AMPERSAND));
	//SignToToken.insert(std::make_pair(TJS_W('.'),Token::DOT));
	SignToToken.insert(std::make_pair(TJS_W('+'),Token::PLUS));
	SignToToken.insert(std::make_pair(TJS_W('-'),Token::MINUS));
	SignToToken.insert(std::make_pair(TJS_W('*'),Token::ASTERISK));
	SignToToken.insert(std::make_pair(TJS_W('/'),Token::SLASH));
	//SignToToken.insert(std::make_pair(TJS_W('\\'),Token::BACKSLASH));
	SignToToken.insert(std::make_pair(TJS_W('%'),Token::PERCENT));
	SignToToken.insert(std::make_pair(TJS_W('^'),Token::CHEVRON));
	//SignToToken.insert(std::make_pair(TJS_W('['),Token::LBRACKET));
	//SignToToken.insert(std::make_pair(TJS_W(']'),Token::RBRACKET));
	//SignToToken.insert(std::make_pair(TJS_W('('),Token::LPARENTHESIS));
	//SignToToken.insert(std::make_pair(TJS_W(')'),Token::RPARENTHESIS));
	SignToToken.insert(std::make_pair(TJS_W('~'),Token::TILDE));
	SignToToken.insert(std::make_pair(TJS_W('?'),Token::QUESTION));
	//SignToToken.insert(std::make_pair(TJS_W(':'),Token::COLON));
	SignToToken.insert(std::make_pair(TJS_W(','),Token::COMMA));
	SignToToken.insert(std::make_pair(TJS_W(';'),Token::SEMICOLON));
	//SignToToken.insert(std::make_pair(TJS_W('{'),Token::LBRACE));
	//SignToToken.insert(std::make_pair(TJS_W('}'),Token::RBRACE));
	SignToToken.insert(std::make_pair(TJS_W('#'),Token::SHARP));
	//SignToToken.insert(std::make_pair(TJS_W('$'),Token::DOLLAR));
	SignToToken.insert(std::make_pair(TJS_W('@'),Token::AT));
	SignToToken.insert(std::make_pair(TJS_W('|'),Token::VERLINE));
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::AddSignWord( tjs_char sign, const ttstr& word ) {
	auto tokenpair = SignToToken.find( sign );
	if( tokenpair != SignToToken.end() ) {
		auto result = TagCommandPair.insert(std::make_pair(tokenpair->second , word));
		if( !result.second ) {
			tjs_char ptr[128];
			TJS_snprintf(ptr, sizeof(ptr)/sizeof(tjs_char), TJS_W("'%c'は登録済みです。"), sign);
			TVPAddLog( ptr );
		}
	} else {
		tjs_char ptr[128];
		TJS_snprintf(ptr, sizeof(ptr)/sizeof(tjs_char), TJS_W("'%c'は登録できない記号です。"), sign);
		TVPAddLog( ptr );
	}
}
//---------------------------------------------------------------------------
ttstr* tTJSScriptBlock::GetTagSignWord( Token token ) {
	auto ret = TagCommandPair.find( token );
	if( ret != TagCommandPair.end() ) {
		return &ret.second;
	}
	return nullptr;
}
//---------------------------------------------------------------------------
tTJSScriptBlock::tTJSScriptBlock() : RefCount(1), LineOffset(0) {
	iTJSDispatch2* arrayClass = nullptr;
	iTJSDispatch2* a = TJSCreateArrayObject( &arrayClass )
	try {
		tTJSVariant val;
		tjs_error er = arrayClass->PropGet( 0, TJS_W("add"), nullptr, %val, arrayClass );
		if(TJS_FAILED(er)) TVPThrowInternalError;
		ArrayAddFunc = val.AsObject();
	} catch(...) {
		a->Release();
		arrayClass->Release();
		if(ArrayAddFunc) ArrayAddFunc->Release(), ArrayAddFunc = nullptr;
		throw;
	}
	a->Release();
	arrayClass->Release();

	LexicalAnalyzer.reset( new tTJSLexicalAnalyzer(this) );
}
//---------------------------------------------------------------------------
tTJSScriptBlock::~tTJSScriptBlock()
{
	if(ArrayAddFunc) ArrayAddFunc->Release(), ArrayAddFunc = nullptr;
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
	if( name ) {
		LineOffset = lineofs;
		Name.reset( new tjs_char[ TJS_strlen(name) + 1] );
		TJS_strcpy(Name.get(), name);
	}
}
//---------------------------------------------------------------------------
const tjs_char * tTJSScriptBlock::GetLine(tjs_int line, tjs_int *linelength) const
{
	// note that this function DOES matter LineOffset
	line -= LineOffset;
	if(linelength) *linelength = LineLengthVector[line];
	return Script.get() + LineVector[line];
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
	tjs_int line = SrcPosToLine(pos)+1;
	ttstr name;
	if( Name )
	{
		name = ttstr( Name.get() );
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
void tTJSScriptBlock::SetText(tTJSVariant *result, const tjs_char *text)
{
	TJS_F_TRACE("tTJSScriptBlock::SetText");

	// compiles text and executes its global level scripts.
	// the script will be compiled as an expression if isexpressn is true.
	if(!text) return;
	if(!text[0]) return;

	TJS_D((TJS_W("Counting lines ...\n")))

	Script.reset( new tjs_char[TJS_strlen(text)+1] );
	TJS_strcpy(Script.get(), text);

	// calculation of line-count
	tjs_char *ls = Script.get();
	tjs_char *p = Script.get();
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

	CompileErrorCount = 0;
	for( tjs_uint i = 0; i < LineVector.size(); i++ ) {
		ParseLine( i );
	}
	if(CompileErrorCount)
	{
		TJS_eTJSCompileError(FirstError, this, FirstErrorPos);
	}
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::WarningLog( const tjs_char* message ) {
	Log( LogType::Warning, message );
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::ErrorLog( const tjs_char* message ) {
	Log( LogType::Error, message );
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::Log( LogType type, const tjs_char* message ) {
	ttstr typemes;
	if( type == LogType::Warning ) {
		typemes = ttstr( TJS_W("warning : ") );
	} else {
		typemes = ttstr( TJS_W("error : ") );
	}
	tjs_int line = CurrentLine;
	if( Name ) {
		TVPAddLog( typemes + Name + TJS_W("(") + ttstr(line) + TJS_W(")") );
	} else {
		tjs_char ptr[128];
		TJS_snprintf(ptr, sizeof(ptr)/sizeof(tjs_char), TJS_W("0x%p"), this);
		ttstr name = ttstr(TJS_W("anonymous@")) + ptr;
		TVPAddLog( typemes + name + TJS_W("(") + ttstr(line) + TJS_W(")") );
	}
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::CreateCurrentDic( const tTJSVariantString& name ) {
	if( CurrentDic ) {
		CurrentDic->Release();
	}
	CurrentDic = TJSCreateDictionaryObject();
	tTJSVariant tmp(name);
	CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __type_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
}
void tTJSScriptBlock::CreateCurrentTagDic() {
	if( CurrentDic ) {
		CurrentDic->Release();
	}
	CurrentDic = TJSCreateDictionaryObject();
	//tTJSVariant tmp(__tag_name);
	//CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __type_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
}
void tTJSScriptBlock::CreateCurrentLabelDic() {
	CreateCurrentDic(__label_name.AsVariantStringNoAddRef);
}
void tTJSScriptBlock::CrearCurrentTag() {
	if( CurrentAttributeDic ) {
		CurrentAttributeDic->Release();
		CurrentAttributeDic = nullptr;
	}
	if( CurrentParameterDic ) {
		CurrentParameterDic->Release();
		CurrentParameterDic = nullptr;
	}
	if( CurrentCommandArray ) {
		CurrentCommandArray->Release();
		CurrentCommandArray = nullptr;
	}
	if( CurrentDic ) {
		CurrentDic->Release();
		CurrentDic = nullptr;
	}
}
void tTJSScriptBlock::AddValueToLine( const tTJSVariant& val ) {
	assert( CurrentCommandArray );
	assert( ScenarioLines );
	ScenarioLines->PropSetByNum( TJS_MEMBERENSURE, CurrentLine, &val, ScenarioLines );
}
void tTJSScriptBlock::SetCurrentTagName( const ttstr& name ) {
	if( !CurrentDic ) {
		CurrentDic = TJSCreateDictionaryObject();
	}
	tTJSVariant tmp(name);
	CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __name_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
}
void tTJSScriptBlock::PushCurrentTag() {
	if( CurrentDic ) {
		tTJSVariant tmp(CurrentDic, CurrentDic);
		CurrentDic->Release();
		CurrentDic = nullptr;
		PushValueCurrentLine( tmp );
	}
}
void tTJSScriptBlock::PushCurrentLabel() {
	if( CurrentDic ) {
		tTJSVariant tmp(CurrentDic, CurrentDic);
		CurrentDic->Release();
		CurrentDic = nullptr;
		AddValueToLine( tmp );
	}
}
void tTJSScriptBlock::PushNameTag( const ttstr& name ) {
	SetCurrentTagName( name );
	PushCurrentTag();
}
void tTJSScriptBlock::PushAttribute( const ttstr& name, const tTJSVariant& value, bool isparameter ) {
	PushAttribute( name.AsVariantStringNoAddRef(), value, isparameter )
}
void tTJSScriptBlock::PushAttribute( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter ) {
	bool checkexist = true;
	if( !CurrentDic ) {
		CurrentDic = TJSCreateDictionaryObject();
		checkexist = false;
	}
	iTJSDispatch2* dest = nullptr;
	if( !isparameter ) {
		if( !CurrentAttributeDic ) {
			CurrentAttributeDic = TJSCreateDictionaryObject();
			tTJSVariant tmp(CurrentAttributeDic,CurrentAttributeDic);
			CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __attribute_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
			checkexist = false;
		}
		dest = CurrentAttributeDic;
	} else {
		if( !CurrentParameterDic ) {
			CurrentParameterDic = TJSCreateDictionaryObject();
			tTJSVariant tmp(CurrentParameterDic,CurrentParameterDic);
			CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __parameter_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
			checkexist = false;
		}
		dest = CurrentParameterDic;
	}
	if( checkexist ) {
		tTJSVariant v;
		tjs_error hr = dest->PropGetByVS( 0, name, &v, dest );
		if( hr != TJS_E_MEMBERNOTFOUND ) {
			if( isparameter ) {
				WarningLog( (ttstr(name) + ttstr(TJS_W(" パラメータが二重に追加されています。"))).c_str() );
			} else {
				WarningLog( (ttstr(name) + ttstr(TJS_W(" 属性が二重に追加されています。"))).c_str() );
			}
		}
	}
	dest->PropSetByVS( TJS_MEMBERENSURE, name, &value, dest );
}
void tTJSScriptBlock::PushAttributeReference( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter ) {
	iTJSDispatch2* ref = TJSCreateDictionaryObject();
	ref->PropSetByVS( TJS_MEMBERENSURE, __ref_name.AsVariantStringNoAddRef(), &value, ref );
	tTJSVariant tmp(ref,ref);
	ref->Release();
	PushAttribute( name, tmp, isparameter );
}
void tTJSScriptBlock::PushAttributeFileProperty( const tTJSVariantString& name, const tTJSVariant& file, const tTJSVariant& prop, bool isparameter ) {
	iTJSDispatch2* ref = TJSCreateDictionaryObject();
	ref->PropSetByVS( TJS_MEMBERENSURE, __file_name.AsVariantStringNoAddRef(), &file, ref );
	ref->PropSetByVS( TJS_MEMBERENSURE, __prop_name.AsVariantStringNoAddRef(), &prop, ref );
	tTJSVariant tmp(ref,ref);
	ref->Release();
	PushAttribute( name, tmp, isparameter );
}
void tTJSScriptBlock::PushTagCommand( const ttstr& name ) {
	if( !CurrentDic ) {
		CurrentDic = TJSCreateDictionaryObject();
	}
	if( !CurrentCommandArray ) {
		CurrentCommandArray = TJSCreateArrayObject();
		tTJSVariant tmp(CurrentCommandArray,CurrentCommandArray);
		CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __command_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
	}
	assert( ArrayAddFunc );
	tTJSVariant value(name);
	tTJSVariant *pval = &value;
	ArrayAddFunc->FuncCall( 0, nullptr, nullptr, nullptr, 1, &pval, CurrentCommandArray );
}
void tTJSScriptBlock::SetCurrentLabelName( const tTJSVariant& val ) {
	SetValueToCurrentDic( __name_name, val );
}
void tTJSScriptBlock::SetCurrentLabelDescription( const ttstr& desc ) {
	tTJSVariant tmp(name);
	SetValueToCurrentDic( __description_name, tmp );
}
void tTJSScriptBlock::SetValueToCurrentDic( const ttstr& name, tTJSVariant& val ) {
	if( !CurrentDic ) {
		CurrentDic = TJSCreateDictionaryObject();
	}
	CurrentDic->PropSetByVS( TJS_MEMBERENSURE, name.AsVariantStringNoAddRef(), &val, CurrentDic );
}
void tTJSScriptBlock::AddCurrentDicToLine() {
	tTJSVariant tmp(CurrentDic,CurrentDic);
	CurrentDic->Release();
	CurrentDic = nullptr;
	AddValueToLine( tmp );
}
/**
 タグ属性に書かれた参照とファイル属性をパースする
 name : 参照
 name.value : 参照
 name::value : ファイル属性
 name.exp::value.value : ファイル属性
 */
void tTJSScriptBlock::ParseAttributeValueSymbol( const tTJSVariant& symbol, const tTJSVariant& valueSymbol, bool isparameter ) {
	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	if( token == Token::DOUBLE_COLON ) {
		//ファイル属性として解釈する
		const tTJSVariantString* filename = valueSymbol.AsStringNoAddRef();
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token == Token::SYMBOL ) {
			ttstr prop( LexicalAnalyzer.GetValue( value ).AsStringNoAddRef() );
			token = LexicalAnalyzer.GetInTagToken( value );
			while( token == Token::DOT ) {
				prop += ttstr(TJS_W("."));
				token = LexicalAnalyzer.GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					prop += ttstr( LexicalAnalyzer.GetValue( value ).AsStringNoAddRef() );
				} else {
					CompileErrorCount++;
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
				}
			}
			LexicalAnalyzer.Unlex( token, value );
			tTJSVariant file(filename);
			tTJSVariant propvalue(prop);
			PushAttributeFileProperty( symbol, file, prop, isparameter );
		} else {
			CompileErrorCount++;
			ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
		}
	} else if( token == Token::DOT ) {
		// 参照かファイル属性のファイル名に拡張子が付いているかのどちらか
		ttstr name( valueSymbol.AsStringNoAddRef() );
		name += ttstr( TJS_W(".") );
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token == Token::SYMBOL ) {
			// まずはどちらかわからないので両方のケースを考慮する
			name += ttstr( LexicalAnalyzer.GetValue( value ).AsStringNoAddRef() );
			token = LexicalAnalyzer.GetInTagToken( value );
			while( token == Token::DOT ) {
				name += ttstr(TJS_W("."));
				token = LexicalAnalyzer.GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					name += ttstr( LexicalAnalyzer.GetValue( value ).AsStringNoAddRef() );
				} else {
					CompileErrorCount++;
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定された参照の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
				}
			}
			if( token == Token::DOUBLE_COLON ) {
				// ファイル属性だった
				token = LexicalAnalyzer.GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					ttstr prop( LexicalAnalyzer.GetValue( value ).AsStringNoAddRef() );
					token = LexicalAnalyzer.GetInTagToken( value );
					while( token == Token::DOT ) {
						prop += ttstr(TJS_W("."));
						token = LexicalAnalyzer.GetInTagToken( value );
						if( token == Token::SYMBOL ) {
							prop += ttstr( LexicalAnalyzer.GetValue( value ).AsStringNoAddRef() );
						} else {
							CompileErrorCount++;
							ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
						}
					}
					LexicalAnalyzer.Unlex( token, value );
					tTJSVariant file(name);
					tTJSVariant propvalue(prop);
					PushAttributeFileProperty( symbol, file, prop, isparameter );
				} else {
					CompileErrorCount++;
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
				}
			} else {
				// 参照だった
				LexicalAnalyzer.Unlex( token, value );
				tTJSVariant ref(name);
				PushAttributeReference( symbol.AsStringNoAddRef(), ref, isparameter );
			}
		} else {
			CompileErrorCount++;
			ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定された参照の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
		}
	} else {
		// . も :: もない場合は、変数参照であるとして登録する。
		PushAttributeReference( symbol.AsStringNoAddRef(), valueSymbol );
		LexicalAnalyzer.Unlex( token, value, isparameter );
	}
}
void tTJSScriptBlock::ParseAttribute( const tTJSVariant& symbol, bool isparameter ) {
	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	if( token == Token::EQUAL ) {
		token = LexicalAnalyzer.GetInTagToken( value );
		switch( token ) {
		case Token::CONSTVAL:
		case Token::SINGLE_TEXT:
		case Token::DOUBLE_TEXT:
		case Token::NUMBER:
		case Token::OCTET: {
			const tTJSVariant& v = LexicalAnalyzer.GetValue(value);
			PushAttribute( *symbol.AsStringNoAddRef(), v, isparameter );
			}
			break;
		case Token::PLUS: {
			tjs_int v2;
			Token t = LexicalAnalyzer.GetInTagToken( v2 );
			if( t == Token::NUMBER ) {
				const tTJSVariant& v = LexicalAnalyzer.GetValue( v2 );
				PushAttribute( *symbol.AsStringNoAddRef(), v, isparameter );
			} else {
				CompileErrorCount++;
				ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に\"+\"数値以外が指定されました。数値として解釈できません。")).c_str() );
			}
			}
			break;
		case Token::MINUS: {
			tjs_int v2;
			Token t = LexicalAnalyzer.GetInTagToken( v2 );
			if( t == Token::NUMBER ) {
				tTJSVariant m(LexicalAnalyzer.GetValue( v2 ));
				m.changesign();
				PushAttribute( *symbol.AsStringNoAddRef(), m, isparameter );
			} else {
				CompileErrorCount++;
				ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に\"-\"数値以外が指定されました。数値として解釈できません。")).c_str() );
			}
			}
			break;
		case Token::SYMBOL:	// TJS2 value or file prop
			ParseAttributeValueSymbol( symbol, LexicalAnalyzer.GetValue(value), isparameter );
			break;
		}
	} else {
		if( !isparameter ) {
			PushTagCommand( ttstr(*symbol.AsStringNoAddRef()) );
		} else {
			tTJSVariant v(nullptr,nullptr);	// null
			PushAttribute( *symbol.AsStringNoAddRef(), v, isparameter );
		}
		LexicalAnalyzer.Unlex( token, value );
	}
}
bool tTJSScriptBlock::ParseSpecialAttribute( Token token, tjs_int value ) {
	switch( token ) {
	case Token::SINGLE_TEXT: {
		const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
		PushAttribute( __voice_name, v );
		return true;
	}
	case Token::DOUBLE_TEXT: {
		const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
		PushAttribute( __storage_name, v );
		return true;
	}
	case Token::LT:
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
			PushAttribute( __time_name, v );
		} else {
			CompileErrorCount++;
			ErrorLog( TJS_W("タグ内で'<'の後数値以外が指定されています。") );
		}
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token != Token::GT ) {
			CompileErrorCount++;
			ErrorLog( TJS_W("タグ内で'<'の後'>'で閉じられていません。") );
		}
		return true;

	case Token::LBRACE:
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
			PushAttribute( __wait_name, v );
		} else {
			CompileErrorCount++;
			ErrorLog( TJS_W("タグ内で'{'の後数値以外が指定されています。") );
		}
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token != Token::RBRACE ) {
			CompileErrorCount++;
			ErrorLog( TJS_W("タグ内で'{'の後'}'で閉じられていません。") );
		}
		return true;

	case Token::LPARENTHESIS:
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
			PushAttribute( __fade_name, v );
		} else {
			CompileErrorCount++;
			ErrorLog( TJS_W("タグ内で'('の後数値以外が指定されています。") );
		}
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token != Token::RPARENTHESIS ) {
			CompileErrorCount++;
			ErrorLog( TJS_W("タグ内で'('の後')'で閉じられていません。") );
		}
		return true;

	default:
		break;
	}
	return false;
}
void tTJSScriptBlock::ParseTag() {
	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	bool findtagname = false;
	do {
		switch( token ) {
		case Token::SYMBOL: {
			const tTJSVariant& val = LexicalAnalyzer.GetValue(value);
			ttstr name( val.AsStringNoAddRef() );
			SetCurrentTagName( name );
			findtagname = true;
			break;
		}

		case Token::EOL:
			if( !LineAttribute ) {
				MultiLineTag = true;
			} else {
				LineAttribute = false;
			}
			return;

		case Token::RBRACKET:
			MultiLineTag = false;
			return;	// exit tag

		default: {
			if( ParseSpecialAttribute( token, value ) ) {
				findtagname = true;
			} else {
				ttstr* word = GetTagSignWord( token );
				if( word != null ) {
					PushTagCommand( word );
				} else {
					// unknown symbol
					findtagname = true;
				}
			}
			break;
		}
		}
	} while(!findtagname);

	ParseAttributes();
}
void tTJSScriptBlock::ParseAttributes() {
	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	bool intag = true;
	do {
		switch( token ) {
		case Token::SYMBOL:
			ParseAttribute( LexicalAnalyzer.GetValue(value) );
			break;

		case Token::DOLLAR:	// パラメータとして解釈
			token = LexicalAnalyzer.GetInTagToken( value );
			if( token == Token::SYMBOL ) {
				ParseAttribute( LexicalAnalyzer.GetValue(value), true );
			} else {
				CompileErrorCount++;
				ErrorLog( TJS_W("タグ内で$の後にパラメータ名として解釈できない文字が用いられました。") );
			}
			break;

		case Token::RBRACKET:	// ] タグ終了
			MultiLineTag = false;
			intag = false;
			break;

		case Token::EOL:
			if( !LineAttribute ) {
				MultiLineTag = true;
			} else {
				LineAttribute = false;
			}
			intag = false;
			break;

		default:
			if( !ParseSpecialAttribute( token, value ) ) {
				CompileErrorCount++;
				ErrorLog( TJS_W("タグ内で解釈できない記号が用いられました。") );
			}
			break;
		}
	} while( intag );
}
/**
 * <<< transname attributes
 */
void tTJSScriptBlock::ParseTransition() {
	CreateTagDic();
	SetCurrentTagName( __endtrans_name );

	LineAttribute = true;
	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	if( token == Token::SYMBOL ) {
		const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
		tjs_int v2;
		Token t = LexicalAnalyzer.GetInTagToken( v2 );
		if( t == Token::EQUAL ) {	// <<< symbol=
			LexicalAnalyzer.Unlex( t, v2 );
		} else {
			PushAttribute( __trans_name, v );
			token = t;
			value = v;
		}
	} else {
		LexicalAnalyzer.Unlex( token, value );
	}
	ParseAttributes();
}
/**
 * @名前指定の時、charnameタグとして処理する
 * 指定された名前は name = 属性へ
 * 代替表示名がある時は alias = 属性へ
 */
void tTJSScriptBlock::ParseCharacter() {
	CreateTagDic();
	SetCurrentTagName( __charname_name );

	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	if( token == Token::SYMBOL ) {
		const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
		PushAttribute( __name_name, v );
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token == Token::SLASH ) {
			// 代替表示名指定
			token = LexicalAnalyzer.GetInTagToken( value );
			if( token == Token::SYMBOL || token == Token::SINGLE_TEXT || token == Token::DOUBLE_TEXT ) {
				v = LexicalAnalyzer.GetValue( value );
				PushAttribute( __alias_name, v );
			} else {
				tTJSVariant voidvar;
				PushAttribute( __alias_name, voidvar );	// alias に空文字指定
				LexicalAnalyzer.Unlex( token, value );
			}
		} else {
			LexicalAnalyzer.Unlex( token, value );
		}
		LineAttribute = true;
		ParseAttributes();
	} else {
		CompileErrorCount++;
		ErrorLog( TJS_W("@の後に名前として解釈できない文字が指定されています。") );
	}
}
/**
 * #labelname|description
 */
void tTJSScriptBlock::ParseLabel() {
	CreateCurrentLabelDic();

	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	if( token == Token::Symbol || token == Token::VERTLINE ) {
		if( token == Token::Symbol ) {
			const tTJSVariant& v = LexicalAnalyzer.GetValue( value );
			token = LexicalAnalyzer.GetInTagToken( value )
		}
		if( token == Token::VERTLINE ) {
			ttstr desc = LexicalAnalyzer.GetRemainString()
			SetCurrentLabelDescription( desc );
		}
		
	} else {
		CompileErrorCount++;
		ErrorLog( TJS_W("#の後にラベル名として解釈できない文字が指定されています。") );
	}

	AddCurrentDicToLine();
}
/**
%[
	type : "select",
	text : "user reading text",	// null の時targetラベル参照
	image : "imagefile"
	target : "target"
]
 
 */
void tTJSScriptBlock::ParseSelect( tjs_int number ) {
	CreateCurrentDic(__select_name.AsVariantStringNoAddRef);

	tjs_int value;
	Token token = LexicalAnalyzer.GetInTagToken( value );
	if( token == Token::ASTERISK ) {
		// * の時は、nullを入れてtargetのラベル参照
		tTJSVariant v(nullptr,nullptr);
		SetValueToCurrentDic( __text_name, v );
		token = LexicalAnalyzer.GetInTagToken( value );
		if( token != Token::VERTLINE ) {
			CompileErrorCount++;
			ErrorLog( TJS_W("選択肢で*の後に|がありません。") );
		}
	} else if( token == Token::VERTLINE ) {
		// | の時は、次の|までを画像ファイル名として読み込む
		tjs_int text = LexicalAnalyzer.ReadToVerline();
		if( text >= 0 ) {
			const tTJSVariant& v = LexicalAnalyzer.GetValue( text );
			SetValueToCurrentDic( __image_name, v );
		}
	} else {
		// それ以外の時は、|までを表示するテキストとして解釈する
		LexicalAnalyzer.Unlex();
		tjs_int text = LexicalAnalyzer.ReadToVerline();
		if( text >= 0 ) {
			const tTJSVariant& v = LexicalAnalyzer.GetValue( text );
			SetValueToCurrentDic( __text_name, v );
		}
	}
	tjs_int text = LexicalAnalyzer.ReadToVerline();
	if( text >= 0 ) {
		const tTJSVariant& v = LexicalAnalyzer.GetValue( text );
		SetValueToCurrentDic( __target_name, v );
	}
	// それ以降は属性として読み込む
	LineAttribute = true;
	ParseAttributes();

	AddCurrentDicToLine();
}
/**
%[
	type : "next",
	target : "filename",
	cond : "flag"
]
 */
void tTJSScriptBlock::ParseNextScenario() {
	CreateCurrentDic(__next_name.AsVariantStringNoAddRef);

	tjs_int text = LexicalAnalyzer.ReadToSpace();
	if( text >= 0 ) {
		const tTJSVariant& v = LexicalAnalyzer.GetValue( text );
		SetValueToCurrentDic( __target_name, v );
	}
	tjs_int text = LexicalAnalyzer.ReadToSpace();
	if( text >= 0 ) {
		const tTJSVariant& v = LexicalAnalyzer.GetValue( text );
		tTJSVariantString* vs = v.AsStringNoAddRef();
		if( __if_name == ttstr(vs) ) {
			text = LexicalAnalyzer.ReadToSpace();
			if( text >= 0 ) {
				const tTJSVariant& v = LexicalAnalyzer.GetValue( text );
				SetValueToCurrentDic( __cond_name, v );
			} else {
				CompileErrorCount++;
				ErrorLog( TJS_W(">の後のifに続く条件式がありません。") );
			}
		}
	}

	AddCurrentDicToLine();
}
/**
 タグかテキストを解析する
 */
bool tTJSScriptBlock::ParseTag( tjs_int token, tjs_int value ) {
	if( token == Token::EOL ) return false;

	switch( token ) {
	case Token::TEXT:
		PushValueCurrentLine( LexicalAnalyzer.GetValue(value) );
		return true;

	case Token::BEGIN_TAG:
		ParseTag();
		return true;

	// ルビがまだ未対応

	default:
		CompileErrorCount++;
		ErrorLog( TJS_W("不明な文法です。") );
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
		LineAttribute = false;

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
				ParseSelect(value);
				HasSelectLine = true;
				break;
			case Token::NEXT_SCENARIO:	// >
				ParseNextScenario();
				break;
			case Token::LINE_COMMENTS:
				CreateCurrentDic( __linecomment_name.AsVariantStringNoAddRef );
				AddCurrentDicToLine();
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
