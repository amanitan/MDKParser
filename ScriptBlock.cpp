//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Block Management
//---------------------------------------------------------------------------
#include "ScriptBlock.h"
#include <assert.h>

static const tjs_char* TVPInternalError = TJS_W( "内部エラーが発生しました: at %1 line %2" );
#define TVPThrowInternalError \
	TVPThrowExceptionMessage(TVPInternalError, __FILE__,  __LINE__)

template <typename TContainer>
void split( const tjs_string& val, const tjs_char& delim, TContainer& result ) {
	result.clear();
	const tjs_string::size_type n = 1;
	tjs_string::size_type pos = 0;
	while( pos != tjs_string::npos ) {
		tjs_string::size_type p = val.find( delim, pos );
		if( p == tjs_string::npos ) {
			if( pos >= ( val.size() - 1 ) ) {
				if( val[pos] != delim ) {
					result.push_back( val.substr( pos ) );
				}
			} else {
				result.push_back( val.substr( pos ) );
			}
			break;
		} else {
			if( ( p - pos ) == 1 ) {
				if( val[pos] != delim ) {
					result.push_back( val.substr( pos ) );
				}
			} else {
				result.push_back( val.substr( pos, p - pos ) );
			}
		}
		pos = p + n;
	}
}

static void TJSReportExceptionSource( const ttstr &msg ) {
	//if( TJSEnableDebugMode )
	{
		TVPAddLog( msg );
	}
}
//---------------------------------------------------------------------------
static void TJS_eTJSCompileError( const ttstr & msg ) {
	TJSReportExceptionSource( msg );
	TVPThrowExceptionMessage( msg.c_str() );
}
//---------------------------------------------------------------------------
static void TJS_eTJSCompileError( const tjs_char *msg ) {
	TJSReportExceptionSource( msg );
	TVPThrowExceptionMessage( msg );
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
	SignToToken.insert(std::make_pair(TJS_W('|'),Token::VERTLINE));

	__endtrans_name = TJSMapGlobalStringMap(TJS_W("endtrans"));
	__begintrans_name = TJSMapGlobalStringMap(TJS_W("begintrans"));

	__storage_name = TJSMapGlobalStringMap(TJS_W("storage"));
	__type_name = TJSMapGlobalStringMap(TJS_W("type"));
	__name_name = TJSMapGlobalStringMap(TJS_W("name"));
	__value_name = TJSMapGlobalStringMap(TJS_W("value"));

	__tag_name = TJSMapGlobalStringMap(TJS_W("tag"));
	__label_name = TJSMapGlobalStringMap(TJS_W("label"));
	__select_name = TJSMapGlobalStringMap(TJS_W("select"));
	__next_name = TJSMapGlobalStringMap(TJS_W("next"));
	__selopt_name = TJSMapGlobalStringMap( TJS_W( "selopt" ) );

	__attribute_name = TJSMapGlobalStringMap(TJS_W("attribute"));
	__parameter_name = TJSMapGlobalStringMap(TJS_W("parameter"));
	__command_name = TJSMapGlobalStringMap(TJS_W("command"));
	__ref_name = TJSMapGlobalStringMap(TJS_W("ref"));
	__file_name = TJSMapGlobalStringMap(TJS_W("file"));
	__prop_name = TJSMapGlobalStringMap(TJS_W("prop"));

	__trans_name = TJSMapGlobalStringMap(TJS_W("trans"));
	__charname_name = TJSMapGlobalStringMap(TJS_W("charname"));
	__alias_name = TJSMapGlobalStringMap(TJS_W("alias"));
	__description_name = TJSMapGlobalStringMap(TJS_W("description"));
	__text_name = TJSMapGlobalStringMap(TJS_W("text"));
	__image_name = TJSMapGlobalStringMap(TJS_W("image"));
	__target_name = TJSMapGlobalStringMap(TJS_W("target"));
	__if_name = TJSMapGlobalStringMap(TJS_W("if"));
	__cond_name = TJSMapGlobalStringMap(TJS_W("cond"));
	__comment_name = TJSMapGlobalStringMap(TJS_W("comment"));

	__voice_name = TJSMapGlobalStringMap(TJS_W("voice"));
	__time_name = TJSMapGlobalStringMap(TJS_W("time"));
	__wait_name = TJSMapGlobalStringMap(TJS_W("wait"));
	__fade_name = TJSMapGlobalStringMap(TJS_W("fade"));

	__lines_name = TJSMapGlobalStringMap( TJS_W( "lines" ) );

	__ruby_name = TJSMapGlobalStringMap( TJS_W( "ruby" ) );
	__endruby_name = TJSMapGlobalStringMap( TJS_W( "endruby" ) );
	__l_name = TJSMapGlobalStringMap( TJS_W( "l" ) );
	__textstyle_name = TJSMapGlobalStringMap( TJS_W( "textstyle" ) );
	__inlineimage_name = TJSMapGlobalStringMap( TJS_W( "inlineimage" ) );
	__emoji_name = TJSMapGlobalStringMap( TJS_W( "emoji" ) );
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
		return &ret->second;
	}
	return nullptr;
}
//---------------------------------------------------------------------------
tTJSScriptBlock::tTJSScriptBlock() {
	iTJSDispatch2* arrayClass = nullptr;
	iTJSDispatch2* a = TJSCreateArrayObject( &arrayClass );
	try {
		tTJSVariant val;
		tjs_error er = arrayClass->PropGet( 0, TJS_W("add"), nullptr, &val, arrayClass );
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
	LexicalAnalyzer.reset();
	if(ArrayAddFunc) ArrayAddFunc->Release(), ArrayAddFunc = nullptr;
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
void tTJSScriptBlock::WarningLog( const tjs_char* message ) {
	Log( LogType::Warning, message );
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::ErrorLog( const tjs_char* message ) {
	if( CompileErrorCount == 0 ) {
		FirstError = ttstr(message);
	}
	CompileErrorCount++;
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
		TVPAddLog( typemes + Name.get() + TJS_W("(") + ttstr(line) + TJS_W(")") );
	} else {
		tjs_char ptr[128];
		TJS_snprintf(ptr, sizeof(ptr)/sizeof(tjs_char), TJS_W("0x%p"), this);
		ttstr name = ttstr(TJS_W("anonymous@")) + ptr;
		TVPAddLog( typemes + name + TJS_W("(") + ttstr(line) + TJS_W(")") );
	}
}
//---------------------------------------------------------------------------
/** 指定されたタイプ名の辞書を生成する。 */
void tTJSScriptBlock::CreateCurrentDic( const tTJSVariantString& name ) {
	if( CurrentDic ) {
		CurrentDic->Release();
	}
	CurrentDic = TJSCreateDictionaryObject();
	tTJSVariant tmp(name);
	CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __type_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
}
//---------------------------------------------------------------------------
/** 新たに辞書を生成する。 */
void tTJSScriptBlock::CreateCurrentTagDic() {
	if( CurrentDic ) {
		CurrentDic->Release();
	}
	CurrentDic = TJSCreateDictionaryObject();
	//tTJSVariant tmp(__tag_name);
	//CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __type_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
}
//---------------------------------------------------------------------------
/** ラベルとして新たに辞書を生成する。 */
void tTJSScriptBlock::CreateCurrentLabelDic() {
	CreateCurrentDic( *__label_name.AsVariantStringNoAddRef() );
}
//---------------------------------------------------------------------------
/** 現在の辞書やタグに関連する要素をクリアする。 */
void tTJSScriptBlock::ClearCurrentTag() {
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
//---------------------------------------------------------------------------
/** ルビ/文字装飾用スタックをクリアする。 */
void tTJSScriptBlock::ClearRubyDecorationStack() {
	while( !RubyDecorationStack.empty() ) {
		RubyDecorationStack.top()->Release();
		RubyDecorationStack.pop();
	}
}
//---------------------------------------------------------------------------
/** 現在の行に直接値を格納する。 */
void tTJSScriptBlock::AddValueToLine( const tTJSVariant& val ) {
	assert( ScenarioLines );
	ScenarioLines->PropSetByNum( TJS_MEMBERENSURE, CurrentLine, &val, ScenarioLines );
}
//---------------------------------------------------------------------------
/** 現在の行に配列の要素として指定された値を追加する。 */
void tTJSScriptBlock::PushValueCurrentLine( const tTJSVariant& val ) {
	if( !CurrentLineArray ) {
		CurrentLineArray = TJSCreateArrayObject();
		tTJSVariant val( CurrentLineArray, CurrentLineArray );
		AddValueToLine( val );
	}
	tTJSVariant* param[] = { const_cast<tTJSVariant*>(&val) };
	ArrayAddFunc->FuncCall( 0, nullptr, nullptr, nullptr, 1, param, CurrentLineArray );
}
//---------------------------------------------------------------------------
/** 現在の行に配列の要素としてタグを直接追加する。 */
void tTJSScriptBlock::PushDirectTagCurrentLine( const tTJSVariantString* name, const tTJSVariantString* attr , const tTJSVariant* value ) {
	iTJSDispatch2* dic = TJSCreateDictionaryObject();
	tTJSVariant val( dic, dic );
	tTJSVariant tag( name );
	dic->PropSetByVS( TJS_MEMBERENSURE, __name_name.AsVariantStringNoAddRef(), &tag, dic );
	if( value && attr ) {
		// 値と属性名がある時は属性として追加する
		iTJSDispatch2* attribute = TJSCreateDictionaryObject();
		tTJSVariant tmp( attribute, attribute );
		dic->PropSetByVS( TJS_MEMBERENSURE, __attribute_name.AsVariantStringNoAddRef(), &tmp, dic );
		attribute->PropSetByVS( TJS_MEMBERENSURE, const_cast<tTJSVariantString*>(attr), value, attribute );
		attribute->Release();
	} else if( attr ) {
		// 属性名のみがある時はコマンドとして追加する
		iTJSDispatch2* command = TJSCreateArrayObject();
		tTJSVariant tmp( command, command );
		dic->PropSetByVS( TJS_MEMBERENSURE, __command_name.AsVariantStringNoAddRef(), &tmp, dic );

		assert( ArrayAddFunc );
		tTJSVariant value( attr );
		tTJSVariant *pval = &value;
		ArrayAddFunc->FuncCall( 0, nullptr, nullptr, nullptr, 1, &pval, command );
		command->Release();
	} /* else 属性名も値もない時は、タグとしてのみ追加する */
	dic->Release();
	PushValueCurrentLine( val );
}
//---------------------------------------------------------------------------
/** 現在の辞書に名前を設定する。 */
void tTJSScriptBlock::SetCurrentTagName( const ttstr& name ) {
	if( !CurrentDic ) {
		CurrentDic = TJSCreateDictionaryObject();
	}
	tTJSVariant tmp(name);
	CurrentDic->PropSetByVS( TJS_MEMBERENSURE, __name_name.AsVariantStringNoAddRef(), &tmp, CurrentDic );
}
//---------------------------------------------------------------------------
/** 現在の辞書をタグとして現在の行に追加する */
void tTJSScriptBlock::PushCurrentTag() {
	if( CurrentDic ) {
		tTJSVariant tmp(CurrentDic, CurrentDic);
		CurrentDic->Release();
		CurrentDic = nullptr;
		PushValueCurrentLine( tmp );
		ClearCurrentTag();
	}
}
//---------------------------------------------------------------------------
/** 現在の辞書を現在の行に直接格納する。(ラベルに使用) */
void tTJSScriptBlock::PushCurrentLabel() {
	if( CurrentDic ) {
		tTJSVariant tmp(CurrentDic, CurrentDic);
		CurrentDic->Release();
		CurrentDic = nullptr;
		AddValueToLine( tmp );
	}
}
//---------------------------------------------------------------------------
/** 指定された名前のタグを現在の行に追加する。 */
void tTJSScriptBlock::PushNameTag( const ttstr& name ) {
	SetCurrentTagName( name );
	PushCurrentTag();
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に値を設定する。 */
void tTJSScriptBlock::PushAttribute( const ttstr& name, const tTJSVariant& value, bool isparameter ) {
	PushAttribute( *name.AsVariantStringNoAddRef(), value, isparameter );
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に値を設定する。 */
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
		tjs_error hr = dest->PropGet( 0, name, nullptr, &v, dest );
		if( hr != TJS_E_MEMBERNOTFOUND ) {
			if( isparameter ) {
				WarningLog( (ttstr(name) + ttstr(TJS_W(" パラメータが二重に追加されています。"))).c_str() );
			} else {
				WarningLog( (ttstr(name) + ttstr(TJS_W(" 属性が二重に追加されています。"))).c_str() );
			}
		}
	}
	dest->PropSetByVS( TJS_MEMBERENSURE, const_cast<tTJSVariantString*>(&name), &value, dest );
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に参照を設定する。 */
void tTJSScriptBlock::PushAttributeReference( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter ) {
	iTJSDispatch2* ref = TJSCreateDictionaryObject();
	ref->PropSetByVS( TJS_MEMBERENSURE, __ref_name.AsVariantStringNoAddRef(), &value, ref );
	tTJSVariant tmp(ref,ref);
	ref->Release();
	PushAttribute( name, tmp, isparameter );
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)にファイルプロパティを設定する。 */
void tTJSScriptBlock::PushAttributeFileProperty( const tTJSVariantString& name, const tTJSVariant& file, const tTJSVariant& prop, bool isparameter ) {
	iTJSDispatch2* ref = TJSCreateDictionaryObject();
	ref->PropSetByVS( TJS_MEMBERENSURE, __file_name.AsVariantStringNoAddRef(), &file, ref );
	ref->PropSetByVS( TJS_MEMBERENSURE, __prop_name.AsVariantStringNoAddRef(), &prop, ref );
	tTJSVariant tmp(ref,ref);
	ref->Release();
	PushAttribute( name, tmp, isparameter );
}
//---------------------------------------------------------------------------
/** 現在のタグにコマンドを追加する。 */
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
//---------------------------------------------------------------------------
/*
void tTJSScriptBlock::SetCurrentLabelName( const tTJSVariant& val ) {
	SetValueToCurrentDic( __name_name, val );
}
*/
//---------------------------------------------------------------------------
/** 現在の辞書にラベル詳細として文字列を設定する */
void tTJSScriptBlock::SetCurrentLabelDescription( const ttstr& desc ) {
	tTJSVariant tmp( desc );
	SetValueToCurrentDic( __description_name, tmp );
}
//---------------------------------------------------------------------------
/** 現在の辞書に指定された名前で値を設定する */
void tTJSScriptBlock::SetValueToCurrentDic( const ttstr& name, const tTJSVariant& val ) {
	if( !CurrentDic ) {
		CurrentDic = TJSCreateDictionaryObject();
	}
	CurrentDic->PropSetByVS( TJS_MEMBERENSURE, name.AsVariantStringNoAddRef(), &val, CurrentDic );
}
//---------------------------------------------------------------------------
/** 現在の辞書を現在の行に直接格納する。 */
void tTJSScriptBlock::AddCurrentDicToLine() {
	tTJSVariant tmp(CurrentDic,CurrentDic);
	CurrentDic->Release();
	CurrentDic = nullptr;
	AddValueToLine( tmp );
	ClearCurrentTag();
}
//---------------------------------------------------------------------------
/**
 タグ属性に書かれた参照とファイル属性をパースする
 name : 参照
 name.value : 参照
 name::value : ファイル属性
 name.exp::value.value : ファイル属性
 */
void tTJSScriptBlock::ParseAttributeValueSymbol( const tTJSVariant& symbol, const tTJSVariant& valueSymbol, bool isparameter ) {
	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	if( token == Token::DOUBLE_COLON ) {
		//ファイル属性として解釈する
		const tTJSVariantString* filename = valueSymbol.AsStringNoAddRef();
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token == Token::SYMBOL ) {
			ttstr prop( LexicalAnalyzer->GetValue( value ).AsStringNoAddRef() );
			token = LexicalAnalyzer->GetInTagToken( value );
			while( token == Token::DOT ) {
				prop += ttstr(TJS_W("."));
				token = LexicalAnalyzer->GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					prop += ttstr( LexicalAnalyzer->GetValue( value ).AsStringNoAddRef() );
				} else {
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
				}
			}
			LexicalAnalyzer->Unlex( token, value );
			tTJSVariant file(filename);
			tTJSVariant propvalue(prop);
			PushAttributeFileProperty( *symbol.AsStringNoAddRef(), file, prop, isparameter );
		} else {
			ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
		}
	} else if( token == Token::DOT ) {
		// 参照かファイル属性のファイル名に拡張子が付いているかのどちらか
		ttstr name( valueSymbol.AsStringNoAddRef() );
		name += ttstr( TJS_W(".") );
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token == Token::SYMBOL ) {
			// まずはどちらかわからないので両方のケースを考慮する
			name += ttstr( LexicalAnalyzer->GetValue( value ).AsStringNoAddRef() );
			token = LexicalAnalyzer->GetInTagToken( value );
			while( token == Token::DOT ) {
				name += ttstr(TJS_W("."));
				token = LexicalAnalyzer->GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					name += ttstr( LexicalAnalyzer->GetValue( value ).AsStringNoAddRef() );
				} else {
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定された参照の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
				}
			}
			if( token == Token::DOUBLE_COLON ) {
				// ファイル属性だった
				token = LexicalAnalyzer->GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					ttstr prop( LexicalAnalyzer->GetValue( value ).AsStringNoAddRef() );
					token = LexicalAnalyzer->GetInTagToken( value );
					while( token == Token::DOT ) {
						prop += ttstr(TJS_W("."));
						token = LexicalAnalyzer->GetInTagToken( value );
						if( token == Token::SYMBOL ) {
							prop += ttstr( LexicalAnalyzer->GetValue( value ).AsStringNoAddRef() );
						} else {
							ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
						}
					}
					LexicalAnalyzer->Unlex( token, value );
					tTJSVariant file(name);
					tTJSVariant propvalue(prop);
					PushAttributeFileProperty( *symbol.AsStringNoAddRef(), file, prop, isparameter );
				} else {
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
				}
			} else {
				// 参照だった
				LexicalAnalyzer->Unlex( token, value );
				tTJSVariant ref(name);
				PushAttributeReference( *symbol.AsStringNoAddRef(), ref, isparameter );
			}
		} else {
			ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定された参照の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
		}
	} else {
		// . も :: もない場合は、変数参照であるとして登録する。
		PushAttributeReference( *symbol.AsStringNoAddRef(), valueSymbol );
		LexicalAnalyzer->Unlex( token, value );
	}
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::ParseAttribute( const tTJSVariant& symbol, bool isparameter ) {
	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	if( token == Token::EQUAL ) {
		token = LexicalAnalyzer->GetInTagToken( value );
		switch( token ) {
		case Token::CONSTVAL:
		case Token::SINGLE_TEXT:
		case Token::DOUBLE_TEXT:
		case Token::NUMBER:
		case Token::OCTET: {
			const tTJSVariant& v = LexicalAnalyzer->GetValue(value);
			PushAttribute( *symbol.AsStringNoAddRef(), v, isparameter );
			}
			break;
		case Token::PLUS: {
			tjs_int v2;
			Token t = LexicalAnalyzer->GetInTagToken( v2 );
			if( t == Token::NUMBER ) {
				const tTJSVariant& v = LexicalAnalyzer->GetValue( v2 );
				PushAttribute( *symbol.AsStringNoAddRef(), v, isparameter );
			} else {
				ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に\"+\"数値以外が指定されました。数値として解釈できません。")).c_str() );
			}
			}
			break;
		case Token::MINUS: {
			tjs_int v2;
			Token t = LexicalAnalyzer->GetInTagToken( v2 );
			if( t == Token::NUMBER ) {
				tTJSVariant m(LexicalAnalyzer->GetValue( v2 ));
				m.changesign();
				PushAttribute( *symbol.AsStringNoAddRef(), m, isparameter );
			} else {
				ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に\"-\"数値以外が指定されました。数値として解釈できません。")).c_str() );
			}
			}
			break;
		case Token::SYMBOL:	// TJS2 value or file prop
			ParseAttributeValueSymbol( symbol, LexicalAnalyzer->GetValue(value), isparameter );
			break;
		}
	} else {
		if( !isparameter ) {
			PushTagCommand( ttstr(*symbol.AsStringNoAddRef()) );
		} else {
			tTJSVariant v(nullptr,nullptr);	// null
			PushAttribute( *symbol.AsStringNoAddRef(), v, isparameter );
		}
		LexicalAnalyzer->Unlex( token, value );
	}
}
//---------------------------------------------------------------------------
bool tTJSScriptBlock::ParseSpecialAttribute( Token token, tjs_int value ) {
	switch( token ) {
	case Token::SINGLE_TEXT: {
		const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
		PushAttribute( __voice_name, v );
		return true;
	}
	case Token::DOUBLE_TEXT: {
		const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
		PushAttribute( __storage_name, v );
		return true;
	}
	case Token::LT:
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
			PushAttribute( __time_name, v );
		} else {
			ErrorLog( TJS_W("タグ内で'<'の後数値以外が指定されています。") );
		}
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token != Token::GT ) {
			ErrorLog( TJS_W("タグ内で'<'の後'>'で閉じられていません。") );
		}
		return true;

	case Token::LBRACE:
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
			PushAttribute( __wait_name, v );
		} else {
			ErrorLog( TJS_W("タグ内で'{'の後数値以外が指定されています。") );
		}
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token != Token::RBRACE ) {
			ErrorLog( TJS_W("タグ内で'{'の後'}'で閉じられていません。") );
		}
		return true;

	case Token::LPARENTHESIS:
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
			PushAttribute( __fade_name, v );
		} else {
			ErrorLog( TJS_W("タグ内で'('の後数値以外が指定されています。") );
		}
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token != Token::RPARENTHESIS ) {
			ErrorLog( TJS_W("タグ内で'('の後')'で閉じられていません。") );
		}
		return true;

	default:
		break;
	}
	return false;
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::ParseTag() {
	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	bool findtagname = false;
	if( !FixTagName.IsEmpty() ) {
		findtagname = true;
		SetCurrentTagName( FixTagName );
	}
	do {
		switch( token ) {
		case Token::SYMBOL: {
			if( FixTagName.IsEmpty() ) {
				const tTJSVariant& val = LexicalAnalyzer->GetValue( value );
				ttstr name( val.AsStringNoAddRef() );
				SetCurrentTagName( name );
				findtagname = true;
			} else {
				LexicalAnalyzer->Unlex( token, value );
			}
			break;
		}

		case Token::EOL:
			if( !LineAttribute ) {
				MultiLineTag = true;
			} else {
				LineAttribute = false;
			}
			PushCurrentTag();
			return;

		case Token::RBRACKET:
			MultiLineTag = false;
			PushCurrentTag();
			return;	// exit tag

		default: {
			if( ParseSpecialAttribute( token, value ) ) {
				findtagname = true;
			} else {
				ttstr* word = GetTagSignWord( token );
				if( word != nullptr ) {
					PushTagCommand( *word );
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
	PushCurrentTag();
}
//---------------------------------------------------------------------------
void tTJSScriptBlock::ParseAttributes() {
	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	bool intag = true;
	do {
		switch( token ) {
		case Token::SYMBOL:
			ParseAttribute( LexicalAnalyzer->GetValue(value) );
			break;

		case Token::DOLLAR:	// パラメータとして解釈
			token = LexicalAnalyzer->GetInTagToken( value );
			if( token == Token::SYMBOL ) {
				ParseAttribute( LexicalAnalyzer->GetValue(value), true );
			} else {
				ErrorLog( TJS_W("タグ内で$の後にパラメータ名として解釈できない文字が用いられました。") );
			}
			break;

		case Token::RBRACKET:	// ] タグ終了
			if( TextAttribute ) {
				ErrorLog( TJS_W( "文字装飾が']'で閉じられています。" ) );
			}
			MultiLineTag = false;
			intag = false;
			break;

		case Token::RBRACE:	// }
			if( TextAttribute ) {
				MultiLineTag = false;
				intag = false;
			} else {
				ErrorLog( TJS_W( "タグ内で解釈できない記号が用いられました。" ) );
			}
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
				ErrorLog( TJS_W("タグ内で解釈できない記号が用いられました。") );
			}
			break;
		}
	} while( intag );
}
//---------------------------------------------------------------------------
/**
 * <<< transname attributes
 */
void tTJSScriptBlock::ParseTransition() {
	CreateCurrentTagDic();
	SetCurrentTagName( __endtrans_name );

	LineAttribute = true;
	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	if( token == Token::SYMBOL ) {
		const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
		tjs_int v2;
		Token t = LexicalAnalyzer->GetInTagToken( v2 );
		if( t == Token::EQUAL ) {	// <<< symbol=
			LexicalAnalyzer->Unlex( t, v2 );
		} else {
			PushAttribute( __trans_name, v );
			token = t;
			value = v;
		}
	} else {
		LexicalAnalyzer->Unlex( token, value );
	}
	ParseAttributes();
}
//---------------------------------------------------------------------------
/**
 * @名前指定の時、charnameタグとして処理する
 * 指定された名前は name = 属性へ
 * 代替表示名がある時は alias = 属性へ
 */
void tTJSScriptBlock::ParseCharacter() {
	CreateCurrentTagDic();
	SetCurrentTagName( __charname_name );

	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	if( token == Token::SYMBOL ) {
		const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
		PushAttribute( __name_name, v );
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token == Token::SLASH ) {
			// 代替表示名指定
			token = LexicalAnalyzer->GetInTagToken( value );
			if( token == Token::SYMBOL || token == Token::SINGLE_TEXT || token == Token::DOUBLE_TEXT ) {
				const tTJSVariant& v2 = LexicalAnalyzer->GetValue( value );
				PushAttribute( __alias_name, v2 );
			} else {
				tTJSVariant voidvar;
				PushAttribute( __alias_name, voidvar );	// alias に空文字指定
				LexicalAnalyzer->Unlex( token, value );
			}
		} else {
			LexicalAnalyzer->Unlex( token, value );
		}
		LineAttribute = true;
		ParseAttributes();
	} else {
		ErrorLog( TJS_W("@の後に名前として解釈できない文字が指定されています。") );
	}
}
//---------------------------------------------------------------------------
/**
 * #labelname|description
 */
void tTJSScriptBlock::ParseLabel() {
	CreateCurrentLabelDic();

	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	if( token == Token::SYMBOL || token == Token::VERTLINE ) {
		if( token == Token::SYMBOL ) {
			const tTJSVariant& v = LexicalAnalyzer->GetValue( value );
			token = LexicalAnalyzer->GetInTagToken( value );
		}
		if( token == Token::VERTLINE ) {
			ttstr desc = LexicalAnalyzer->GetRemainString();
			SetCurrentLabelDescription( desc );
		}
		
	} else {
		ErrorLog( TJS_W("#の後にラベル名として解釈できない文字が指定されています。") );
	}

	AddCurrentDicToLine();
}
//---------------------------------------------------------------------------
/**
%[
	type : "select",
	text : "user reading text",	// null の時targetラベル参照
	image : "imagefile"
	target : "target"
]
 
 */
void tTJSScriptBlock::ParseSelect( tjs_int number ) {
	CreateCurrentDic(*__select_name.AsVariantStringNoAddRef());

	tjs_int value;
	Token token = LexicalAnalyzer->GetInTagToken( value );
	if( token == Token::ASTERISK ) {
		// * の時は、nullを入れてtargetのラベル参照
		tTJSVariant v(nullptr,nullptr);
		SetValueToCurrentDic( __text_name, v );
		token = LexicalAnalyzer->GetInTagToken( value );
		if( token != Token::VERTLINE ) {
			ErrorLog( TJS_W("選択肢で*の後に|がありません。") );
		}
	} else if( token == Token::VERTLINE ) {
		// | の時は、次の|までを画像ファイル名として読み込む
		tjs_int text = LexicalAnalyzer->ReadToVerline();
		if( text >= 0 ) {
			const tTJSVariant& v = LexicalAnalyzer->GetValue( text );
			SetValueToCurrentDic( __image_name, v );
		}
	} else {
		// それ以外の時は、|までを表示するテキストとして解釈する
		LexicalAnalyzer->Unlex();
		tjs_int text = LexicalAnalyzer->ReadToVerline();
		if( text >= 0 ) {
			const tTJSVariant& v = LexicalAnalyzer->GetValue( text );
			SetValueToCurrentDic( __text_name, v );
		}
	}
	tjs_int text = LexicalAnalyzer->ReadToVerline();
	if( text >= 0 ) {
		const tTJSVariant& v = LexicalAnalyzer->GetValue( text );
		SetValueToCurrentDic( __target_name, v );
	}
	// それ以降は属性として読み込む
	LineAttribute = true;
	ParseAttributes();

	AddCurrentDicToLine();
}
//---------------------------------------------------------------------------
/**
%[
	type : "next",
	target : "filename",
	cond : "flag"
]
 */
void tTJSScriptBlock::ParseNextScenario() {
	CreateCurrentDic(*__next_name.AsVariantStringNoAddRef());

	tjs_int text = LexicalAnalyzer->ReadToSpace();
	if( text >= 0 ) {
		const tTJSVariant& v = LexicalAnalyzer->GetValue( text );
		SetValueToCurrentDic( __target_name, v );
	}
	text = LexicalAnalyzer->ReadToSpace();
	if( text >= 0 ) {
		const tTJSVariant& v = LexicalAnalyzer->GetValue( text );
		tTJSVariantString* vs = v.AsStringNoAddRef();
		if( __if_name == ttstr(vs) ) {
			text = LexicalAnalyzer->ReadToSpace();
			if( text >= 0 ) {
				const tTJSVariant& v = LexicalAnalyzer->GetValue( text );
				SetValueToCurrentDic( __cond_name, v );
			} else {
				ErrorLog( TJS_W(">の後のifに続く条件式がありません。") );
			}
		}
	}

	AddCurrentDicToLine();
}
//---------------------------------------------------------------------------
/**
 タグかテキストを解析する
 */
bool tTJSScriptBlock::ParseTag( Token token, tjs_int value ) {
	if( token == Token::EOL ) return false;
	TextAttribute = false;

	switch( token ) {
	case Token::TEXT:
		PushValueCurrentLine( LexicalAnalyzer->GetValue(value) );
		return true;

	case Token::BEGIN_TAG:
		ParseTag();
		return true;

	case Token::VERTLINE: {	// ルビ or 文字装飾
		iTJSDispatch2* dic = TJSCreateDictionaryObject();
		RubyDecorationStack.push( dic );
		tTJSVariant v( dic, dic );
		PushValueCurrentLine( v );	// 空の辞書を追加しておく
		return true;
	}
	case Token::BEGIN_RUBY: {	// 《が来たので、ルビ文字であるとみなす
		int text = LexicalAnalyzer->ReadToCharStrict( TJS_W( '》') );
		if( text >= 0 ) {
			if( !RubyDecorationStack.empty() ) {
				{	// rubyタグの内容を埋める
					const tTJSVariant &v = LexicalAnalyzer->GetValue( text );
					iTJSDispatch2* dic = RubyDecorationStack.top();
					RubyDecorationStack.pop();
					dic->PropSetByVS( TJS_MEMBERENSURE, __text_name.AsVariantStringNoAddRef(), &v, dic );
					tTJSVariant tag( __ruby_name );
					dic->PropSetByVS( TJS_MEMBERENSURE, __name_name.AsVariantStringNoAddRef(), &tag, dic );
				}
				// [endruby]タグ追加
				PushDirectTagCurrentLine( __endruby_name.AsVariantStringNoAddRef() );
			} else {
				ErrorLog( TJS_W( "《の前に|がないため、ルビとして解釈できません。" ) );
			}
		} else {
			ErrorLog( TJS_W( "'《'の後に'》'がないため、ルビとして解釈できません。" ) );
		}
		return true;
	}
	case Token::END_RUBY: {	// ルビ辞書の可能性
		if( !RubyDecorationStack.empty() ) {
			{	// rubyタグの内容を埋める/ text属性がないrubyとして登録する、text属性がない場合は辞書から検索してもらう
				iTJSDispatch2* dic = RubyDecorationStack.top();
				RubyDecorationStack.pop();
				tTJSVariant tag( __ruby_name );
				dic->PropSetByVS( TJS_MEMBERENSURE, __name_name.AsVariantStringNoAddRef(), &tag, dic );
			}
			// [endruby]タグ追加
			PushDirectTagCurrentLine( __endruby_name.AsVariantStringNoAddRef() );
		} else {
			ErrorLog( TJS_W( "《の前に|がないため、ルビとして解釈できません。" ) );
		}
		return true;
	}
	case Token::BEGIN_TXT_DECORATION: {	// { が来たので、テキスト装飾であるとみなす
		int text = LexicalAnalyzer->ReadToCharStrict( TJS_W( '}' ) );
		if( text >= 0 ) {
			if( !RubyDecorationStack.empty() ) {
				iTJSDispatch2* dic = RubyDecorationStack.top();
				RubyDecorationStack.pop();
				assert( CurrentDic ); // null のはず
				iTJSDispatch2* oldDic = CurrentDic;
				CurrentDic = dic;
				SetCurrentTagName( __textstyle_name );
				TextAttribute = true;
				ParseAttributes();
				TextAttribute = false;
				PushCurrentTag();
				CurrentDic = oldDic;
			} else {
				ErrorLog( TJS_W( "'{'の前に'|'がないため、文字装飾として解釈できません。" ) );
			}
		} else {
			ErrorLog( TJS_W( "'{'の後に'}'がないため、文字装飾として解釈できません。" ) );
		}
		return true;
	}

	case Token::WAIT_RETURN: // l タグ追加
		PushDirectTagCurrentLine( __l_name.AsVariantStringNoAddRef() );
		return true;

	case Token::INNER_IMAGE: {	// inlineimageタグ追加
		int text = LexicalAnalyzer->ReadToCharStrict( TJS_W( ')' ) );
		if( text >= 0 ) {
			const tTJSVariant &v = LexicalAnalyzer->GetValue( text );
			PushDirectTagCurrentLine( __inlineimage_name.AsVariantStringNoAddRef(), __storage_name.AsVariantStringNoAddRef(), &v );
		} else {
			ErrorLog( TJS_W( "':('の後に画像名がないか、')'がありません。" ) );
		}
		return true;
	}
	case Token::COLON: {	// emojiタグ追加
		int text = LexicalAnalyzer->ReadToCharStrict( TJS_W( ':' ) );
		if( text >= 0 ) {
			const tTJSVariant &v = LexicalAnalyzer->GetValue( text );
			PushDirectTagCurrentLine( __emoji_name.AsVariantStringNoAddRef(), __storage_name.AsVariantStringNoAddRef(), &v );
		} else {
			ErrorLog( TJS_W( "':'の後に絵文字名がないか、':'がありません。" ) );
		}
		return true;
	}

	default:
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
	if( static_cast<tjs_uint>(line) < LineVector.size() ) {
		ClearCurrentTag();
		ClearRubyDecorationStack();

		LineAttribute = false;
		TextAttribute = false;

		tjs_int length;
		const tjs_char *str = GetLine( line, &length );
		if( length == 0 ) {
			// 改行のみ
			tTJSVariant val( 0 );
			AddValueToLine( val );
		} else {
			LexicalAnalyzer->reset( str, length );
			tjs_int value;
			Token token = LexicalAnalyzer->GetFirstToken( value );
			if( HasSelectLine ) {
				if( token == Token::SELECT ) {
					ParseSelect(value);
				} else {
					HasSelectLine = false;
					// 選択肢オプション
					SetCurrentTagName( __selopt_name );
					LineAttribute = true;
					ParseAttributes();
					AddCurrentDicToLine();
				}
				return;
			}

			// first token
			switch( token ) {
			case Token::EOL: {
				// タブのみの行も空行と同じ
				tTJSVariant val( 0 );
				AddValueToLine( val );
				break;
			}
			case Token::BEGIN_TRANS:	// >>> 
				// トランジション開始以降の文字列は無視し、begintransタグを格納するのみ
				PushNameTag( __begintrans_name );
				break;
			case Token::END_TRANS: {	// <<<
				ParseTransition();
				AddCurrentDicToLine();
				break;
				}
			case Token::AT:	// @
				ParseCharacter();
				AddCurrentDicToLine();
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
			case Token::LINE_COMMENTS: {
				//CreateCurrentDic( *__comment_name.AsVariantStringNoAddRef() );
				//AddCurrentDicToLine();
				tTJSVariant val;
				AddValueToLine( val );	// void
				break;
			}
			case Token::BEGIN_FIX_NAME: {
				FixTagName.Clear();
				tjs_int text = LexicalAnalyzer->ReadToSpace();
				if( text >= 0 ) {
					FixTagName = ttstr( LexicalAnalyzer->GetString( text ) );
				}
				tTJSVariant val;
				AddValueToLine( val );	// void
				break;
			}
			case Token::END_FIX_NAME: {
				FixTagName.Clear();
				tTJSVariant val;
				AddValueToLine( val );	// void
				break;
			}
			default:
				while( ParseTag( token, value ) ) {
					token = LexicalAnalyzer->GetTextToken( value );
				}
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
/**
 * 引数で渡された文字列を解析して、文字列として返す。
 */
iTJSDispatch2* tTJSScriptBlock::ParseText( const tjs_char* text ) {
	TJS_F_TRACE( "tTJSScriptBlock::ParseText" );

	// compiles text and executes its global level scripts.
	// the script will be compiled as an expression if isexpressn is true.
	if( !text ) return nullptr;
	if( !text[0] ) return nullptr;

	TJS_D( ( TJS_W( "Counting lines ...\n" ) ) )

	Script.reset( new tjs_char[TJS_strlen( text ) + 1] );
	TJS_strcpy( Script.get(), text );

	LexicalAnalyzer->Free();
	ClearCurrentTag();
	ClearRubyDecorationStack();
	FixTagName.Clear();
	LineVector.clear();
	LineLengthVector.clear();

	// calculation of line-count
	tjs_char *script = Script.get();
	tjs_char *ls = script;
	tjs_char *p = script;
	while( *p ) {
		if( *p == TJS_W( '\r' ) || *p == TJS_W( '\n' ) ) {
			LineVector.push_back( int( ls - script ) );
			LineLengthVector.push_back( int( p - ls ) );
			if( *p == TJS_W( '\r' ) && p[1] == TJS_W( '\n' ) ) p++;
			p++;
			ls = p;
		} else {
			p++;
		}
	}

	if( p != ls ) {
		LineVector.push_back( int( ls - script ) );
		LineLengthVector.push_back( int( p - ls ) );
	}

	LineOffset = 0;
	HasSelectLine = false;
	LineAttribute = false;
	MultiLineTag = false;
	TextAttribute = false;
	FirstError.Clear();
	FirstErrorPos = 0;

	if( ScenarioLines ) {
		ScenarioLines->Release();
		ScenarioLines = nullptr;
	}
	ScenarioLines = TJSCreateArrayObject();
	CompileErrorCount = 0;
	for( CurrentLine = 0; static_cast<tjs_uint>(CurrentLine) < LineVector.size(); CurrentLine++ ) {
		ParseLine( CurrentLine );
	}
	if( CompileErrorCount ) {
		TJS_eTJSCompileError( FirstError );
	}
	iTJSDispatch2* retDic = TJSCreateDictionaryObject();
	if( retDic ) {
		tTJSVariant tmp( ScenarioLines, ScenarioLines );
		retDic->PropSetByVS( TJS_MEMBERENSURE, __lines_name.AsVariantStringNoAddRef(), &tmp, retDic );
		ScenarioLines->Release();
		ScenarioLines = nullptr;
	}
	return retDic;
}
//---------------------------------------------------------------------------
