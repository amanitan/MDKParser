//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script Block Management
//---------------------------------------------------------------------------
#include "Parser.h"
#include "Tag.h"
#include "ScenarioDictionary.h"
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
void Parser::Initialize() {
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
}
//---------------------------------------------------------------------------
void Parser::AddSignWord( tjs_char sign, const ttstr& word ) {
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
ttstr* Parser::GetTagSignWord( Token token ) {
	auto ret = TagCommandPair.find( token );
	if( ret != TagCommandPair.end() ) {
		return &ret->second;
	}
	return nullptr;
}
//---------------------------------------------------------------------------
Parser::Parser() {
	Lex.reset( new LexicalAnalyzer(this) );
}
//---------------------------------------------------------------------------
Parser::~Parser() {
	Lex.reset();
}
//---------------------------------------------------------------------------
const tjs_char * Parser::GetLine(tjs_int line, tjs_int *linelength) const
{
	// note that this function DOES matter LineOffset
	if(linelength) *linelength = LineLengthVector[line];
	return Script.get() + LineVector[line];
}
//---------------------------------------------------------------------------
tjs_int Parser::SrcPosToLine(tjs_int pos) const
{
	tjs_uint s = 0;
	tjs_uint e = (tjs_uint)LineVector.size();
	while(true)
	{
		if(e-s <= 1) return s; // LineOffset is added
		tjs_uint m = s + (e-s)/2;
		if(LineVector[m] > pos)
			e = m;
		else
			s = m;
	}
}
//---------------------------------------------------------------------------
tjs_int Parser::LineToSrcPos(tjs_int line) const
{
	// assumes line is added by LineOffset
	return LineVector[line];
}
//---------------------------------------------------------------------------
void Parser::ConsoleOutput(const tjs_char *msg, void *data)
{
	TVPAddLog( msg );
}
//---------------------------------------------------------------------------
void Parser::WarningLog( const tjs_char* message ) {
	Log( LogType::Warning, message );
}
//---------------------------------------------------------------------------
void Parser::ErrorLog( const tjs_char* message ) {
	if( CompileErrorCount == 0 ) {
		FirstError = ttstr(message);
	}
	CompileErrorCount++;
	Log( LogType::Error, message );
}
//---------------------------------------------------------------------------
void Parser::Log( LogType type, const tjs_char* message ) {
	ttstr typemes;
	if( type == LogType::Warning ) {
		typemes = ttstr( TJS_W("warning : ") );
	} else {
		typemes = ttstr( TJS_W("error : ") );
	}
	tjs_int line = CurrentLine + 1;
	TVPAddLog( typemes + TJS_W("(") + ttstr(line) + TJS_W(") " + message ) );
}
//---------------------------------------------------------------------------
/** ルビ/文字装飾用スタックをクリアする。 */
void Parser::ClearRubyDecorationStack() {
	while( !RubyDecorationStack.empty() ) {
		RubyDecorationStack.top()->Release();
		RubyDecorationStack.pop();
	}
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に値を設定する。 */
void Parser::PushAttribute( const tTJSVariantString* name, iTJSDispatch2* dic, bool isparameter ) {
	tTJSVariant tmp( dic, dic );
	dic->Release();
	PushAttribute( name, tmp, isparameter );
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に値を設定する。 */
void Parser::PushAttribute( const tTJSVariantString* name, const tTJSVariant& value, bool isparameter ) {
	if( isparameter ) {
		if( CurrentTag->setParameter( name, value ) ) {
			WarningLog( ( ttstr( const_cast<tTJSVariantString*>(name) ) + ttstr( TJS_W( " パラメータが二重に追加されています。" ) ) ).c_str() );
		}
	} else {
		if( CurrentTag->setAttribute( name, value ) ) {
			WarningLog( ( ttstr( const_cast<tTJSVariantString*>(name) ) + ttstr( TJS_W( " 属性が二重に追加されています。" ) ) ).c_str() );
		}
	}
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)に参照を設定する。 */
void Parser::PushAttributeReference( const tTJSVariantString& name, const tTJSVariant& value, bool isparameter ) {
	iTJSDispatch2* ref = TJSCreateDictionaryObject();
	ref->PropSetByVS( TJS_MEMBERENSURE, GetRWord()->ref(), &value, ref );
	tTJSVariant tmp(ref,ref);
	ref->Release();
	PushAttribute( &name, tmp, isparameter );
}
//---------------------------------------------------------------------------
/** 指定された名前で現在の辞書の属性(もしくはパラメータ)にファイルプロパティを設定する。 */
void Parser::PushAttributeFileProperty( const tTJSVariantString& name, const tTJSVariant& file, const tTJSVariant& prop, bool isparameter ) {
	iTJSDispatch2* ref = TJSCreateDictionaryObject();
	ref->PropSetByVS( TJS_MEMBERENSURE, GetRWord()->file(), &file, ref );
	ref->PropSetByVS( TJS_MEMBERENSURE, GetRWord()->prop(), &prop, ref );
	tTJSVariant tmp(ref,ref);
	ref->Release();
	PushAttribute( &name, tmp, isparameter );
}
//---------------------------------------------------------------------------
/**
 タグ属性に書かれた参照とファイル属性をパースする
 name : 参照
 name.value : 参照
 name::value : ファイル属性
 name.exp::value.value : ファイル属性
 */
void Parser::ParseAttributeValueSymbol( const tTJSVariant& symbol, const tTJSVariant& valueSymbol, bool isparameter ) {
	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	if( token == Token::DOUBLE_COLON ) {
		//ファイル属性として解釈する
		const tTJSVariantString* filename = valueSymbol.AsStringNoAddRef();
		token = Lex->GetInTagToken( value );
		if( token == Token::SYMBOL ) {
			ttstr prop( Lex->GetValue( value ).AsStringNoAddRef() );
			token = Lex->GetInTagToken( value );
			while( token == Token::DOT ) {
				prop += ttstr(TJS_W("."));
				token = Lex->GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					prop += ttstr( Lex->GetValue( value ).AsStringNoAddRef() );
				} else {
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
				}
			}
			Lex->Unlex( token, value );
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
		token = Lex->GetInTagToken( value );
		if( token == Token::SYMBOL ) {
			// まずはどちらかわからないので両方のケースを考慮する
			name += ttstr( Lex->GetValue( value ).AsStringNoAddRef() );
			token = Lex->GetInTagToken( value );
			while( token == Token::DOT ) {
				name += ttstr(TJS_W("."));
				token = Lex->GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					name += ttstr( Lex->GetValue( value ).AsStringNoAddRef() );
				} else {
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定された参照の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
				}
			}
			if( token == Token::DOUBLE_COLON ) {
				// ファイル属性だった
				token = Lex->GetInTagToken( value );
				if( token == Token::SYMBOL ) {
					ttstr prop( Lex->GetValue( value ).AsStringNoAddRef() );
					token = Lex->GetInTagToken( value );
					while( token == Token::DOT ) {
						prop += ttstr(TJS_W("."));
						token = Lex->GetInTagToken( value );
						if( token == Token::SYMBOL ) {
							prop += ttstr( Lex->GetValue( value ).AsStringNoAddRef() );
						} else {
							ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\".\"の後に文字列以外が来ています。")).c_str() );
						}
					}
					Lex->Unlex( token, value );
					tTJSVariant file(name);
					tTJSVariant propvalue(prop);
					PushAttributeFileProperty( *symbol.AsStringNoAddRef(), file, prop, isparameter );
				} else {
					ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定されたファイル属性の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
				}
			} else {
				// 参照だった
				Lex->Unlex( token, value );
				tTJSVariant ref(name);
				PushAttributeReference( *symbol.AsStringNoAddRef(), ref, isparameter );
			}
		} else {
			ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に指定された参照の記述が間違っています。\"::\"の後に文字列以外が来ています。")).c_str() );
		}
	} else {
		// . も :: もない場合は、変数参照であるとして登録する。
		PushAttributeReference( *symbol.AsStringNoAddRef(), valueSymbol );
		Lex->Unlex( token, value );
	}
}
//---------------------------------------------------------------------------
void Parser::ParseAttribute( const tTJSVariant& symbol, bool isparameter ) {
	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	if( token == Token::EQUAL ) {
		token = Lex->GetInTagToken( value );
		switch( token ) {
		case Token::CONSTVAL:
		case Token::SINGLE_TEXT:
		case Token::DOUBLE_TEXT:
		case Token::NUMBER:
		case Token::OCTET: {
			const tTJSVariant& v = Lex->GetValue(value);
			PushAttribute( symbol.AsStringNoAddRef(), v, isparameter );
			}
			break;
		case Token::PLUS: {
			tjs_int v2;
			Token t = Lex->GetInTagToken( v2 );
			if( t == Token::NUMBER ) {
				const tTJSVariant& v = Lex->GetValue( v2 );
				PushAttribute( symbol.AsStringNoAddRef(), v, isparameter );
			} else {
				ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に\"+\"数値以外が指定されました。数値として解釈できません。")).c_str() );
			}
			}
			break;
		case Token::MINUS: {
			tjs_int v2;
			Token t = Lex->GetInTagToken( v2 );
			if( t == Token::NUMBER ) {
				tTJSVariant m(Lex->GetValue( v2 ));
				m.changesign();
				PushAttribute( symbol.AsStringNoAddRef(), m, isparameter );
			} else {
				ErrorLog( (ttstr(symbol.AsStringNoAddRef()) + TJS_W("の属性値に\"-\"数値以外が指定されました。数値として解釈できません。")).c_str() );
			}
			}
			break;
		case Token::SYMBOL:	// TJS2 value or file prop
			ParseAttributeValueSymbol( symbol, Lex->GetValue(value), isparameter );
			break;
		}
	} else {
		if( !isparameter ) {
			CurrentTag->addCommand( symbol.AsStringNoAddRef() );
		} else {
			tTJSVariant v(nullptr,nullptr);	// null
			PushAttribute( symbol.AsStringNoAddRef(), v, isparameter );
		}
		Lex->Unlex( token, value );
	}
}
//---------------------------------------------------------------------------
bool Parser::ParseSpecialAttribute( Token token, tjs_int value ) {
	switch( token ) {
	case Token::SINGLE_TEXT: {
		const tTJSVariant& v = Lex->GetValue( value );
		PushAttribute( GetRWord()->voice(), v );
		return true;
	}
	case Token::DOUBLE_TEXT: {
		const tTJSVariant& v = Lex->GetValue( value );
		PushAttribute( GetRWord()->storage(), v );
		return true;
	}
	case Token::LT:
		token = Lex->GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = Lex->GetValue( value );
			PushAttribute( GetRWord()->time(), v );
		} else {
			ErrorLog( TJS_W("タグ内で'<'の後数値以外が指定されています。") );
		}
		token = Lex->GetInTagToken( value );
		if( token != Token::GT ) {
			ErrorLog( TJS_W("タグ内で'<'の後'>'で閉じられていません。") );
		}
		return true;

	case Token::LBRACE:
		token = Lex->GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = Lex->GetValue( value );
			PushAttribute( GetRWord()->wait(), v );
		} else {
			ErrorLog( TJS_W("タグ内で'{'の後数値以外が指定されています。") );
		}
		token = Lex->GetInTagToken( value );
		if( token != Token::RBRACE ) {
			ErrorLog( TJS_W("タグ内で'{'の後'}'で閉じられていません。") );
		}
		return true;

	case Token::LPARENTHESIS:
		token = Lex->GetInTagToken( value );
		if( token == Token::NUMBER ) {
			const tTJSVariant& v = Lex->GetValue( value );
			PushAttribute( GetRWord()->fade(), v );
		} else {
			ErrorLog( TJS_W("タグ内で'('の後数値以外が指定されています。") );
		}
		token = Lex->GetInTagToken( value );
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
void Parser::ParseTag() {
	CurrentTag->release();

	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	bool findtagname = false;
	if( !FixTagName.IsEmpty() ) {
		findtagname = true;
		CurrentTag->setTagName( FixTagName.AsVariantStringNoAddRef() );
	}
	do {
		switch( token ) {
		case Token::SYMBOL: {
			if( FixTagName.IsEmpty() ) {
				const tTJSVariant& val = Lex->GetValue( value );
				ttstr name( val.AsStringNoAddRef() );
				CurrentTag->setTagName( name.AsVariantStringNoAddRef() );
				findtagname = true;
			} else {
				Lex->Unlex( token, value );
			}
			break;
		}

		case Token::EOL:
			if( !LineAttribute ) {
				MultiLineTag = true;
			} else {
				LineAttribute = false;
			}
			Scenario->addTagToCurrentLine( *CurrentTag.get() );
			CurrentTag->release();
			return;

		case Token::RBRACKET:
			MultiLineTag = false;
			Scenario->addTagToCurrentLine( *CurrentTag.get() );
			CurrentTag->release();
			return;	// exit tag

		default: {
			if( ParseSpecialAttribute( token, value ) ) {
				findtagname = true;
			} else {
				ttstr* word = GetTagSignWord( token );
				if( word != nullptr ) {
					CurrentTag->addCommand( word->AsVariantStringNoAddRef() );
					token = Lex->GetInTagToken( value );
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
	Scenario->addTagToCurrentLine( *CurrentTag.get() );
	CurrentTag->release();
}
//---------------------------------------------------------------------------
void Parser::ParseAttributes() {
	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	bool intag = true;
	do {
		switch( token ) {
		case Token::SYMBOL:
			ParseAttribute( Lex->GetValue(value) );
			break;

		case Token::DOLLAR:	// パラメータとして解釈
			token = Lex->GetInTagToken( value );
			if( token == Token::SYMBOL ) {
				ParseAttribute( Lex->GetValue(value), true );
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
		if( intag ) {
			token = Lex->GetInTagToken( value );
		}
	} while( intag );
}
//---------------------------------------------------------------------------
/**
 * <<< transname attributes
 */
void Parser::ParseTransition() {
	CurrentTag->release();
	CurrentTag->setTagName( GetRWord()->endtrans() );

	LineAttribute = true;
	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	if( token == Token::SYMBOL ) {
		tjs_int v2;
		Token t = Lex->GetInTagToken( v2 );
		if( t != Token::EQUAL ) {	// <<< symbol=
			PushAttribute( GetRWord()->trans(), Lex->GetValue( value ) );
		}
		Lex->Unlex( t, v2 );
	} else {
		Lex->Unlex( token, value );
	}
	ParseAttributes();

	Scenario->setTag( *CurrentTag.get() );
	CurrentTag->release();
}
//---------------------------------------------------------------------------
/**
 * @名前指定の時、charnameタグとして処理する
 * 指定された名前は name = 属性へ
 * 代替表示名がある時は alias = 属性へ
 */
void Parser::ParseCharacter() {
	CurrentTag->release();
	CurrentTag->setTagName( GetRWord()->charname() );

	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	if( token == Token::SYMBOL ) {
		PushAttribute( GetRWord()->name(), Lex->GetValue( value ) );
		token = Lex->GetInTagToken( value );
		if( token == Token::SLASH ) {
			// 代替表示名指定
			token = Lex->GetInTagToken( value );
			if( token == Token::SYMBOL || token == Token::SINGLE_TEXT || token == Token::DOUBLE_TEXT ) {
				PushAttribute( GetRWord()->alias(), Lex->GetValue( value ) );
			} else {
				tTJSVariant voidvar;
				PushAttribute( GetRWord()->alias(), voidvar );	// alias に空文字指定
				Lex->Unlex( token, value );
			}
		} else {
			Lex->Unlex( token, value );
		}
		LineAttribute = true;
		ParseAttributes();
	} else {
		ErrorLog( TJS_W("@の後に名前として解釈できない文字が指定されています。") );
	}

	Scenario->setTag( *CurrentTag.get() );
	CurrentTag->release();
}
//---------------------------------------------------------------------------
/**
 * #labelname|description
 */
void Parser::ParseLabel() {
	CurrentTag->release();
	CurrentTag->setTagName( GetRWord()->label() );

	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	if( token == Token::SYMBOL || token == Token::VERTLINE ) {
		if( token == Token::SYMBOL ) {
			CurrentTag->setValue( GetRWord()->name(), Lex->GetValue( value ) );
			token = Lex->GetInTagToken( value );
		}
		if( token == Token::VERTLINE ) {
			ttstr desc = Lex->GetRemainString();
			if( desc.GetLen() > 0 ) {
				CurrentTag->setText( GetRWord()->description(), desc );
			}
		}
		
	} else {
		ErrorLog( TJS_W("#の後にラベル名として解釈できない文字が指定されています。") );
	}

	Scenario->setTag( *CurrentTag.get() );
	CurrentTag->release();
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
void Parser::ParseSelect( tjs_int number ) {
	CurrentTag->release();
	CurrentTag->setTagName( GetRWord()->select() );

	tjs_int value;
	Token token = Lex->GetInTagToken( value );
	if( token == Token::ASTERISK ) {
		// * の時は、nullを入れてtargetのラベル参照
		tTJSVariant v(nullptr,nullptr);
		CurrentTag->setValue( GetRWord()->text(), v );
		token = Lex->GetInTagToken( value );
		if( token != Token::VERTLINE ) {
			ErrorLog( TJS_W("選択肢で*の後に|がありません。") );
		}
	} else if( token == Token::VERTLINE ) {
		// | の時は、次の|までを画像ファイル名として読み込む
		tjs_int text = Lex->ReadToVerline();
		if( text >= 0 ) {
			CurrentTag->setValue( GetRWord()->image(), Lex->GetValue( text ) );
		}
	} else {
		// それ以外の時は、|までを表示するテキストとして解釈する
		Lex->Unlex();
		tjs_int text = Lex->ReadToVerline();
		if( text >= 0 ) {
			CurrentTag->setValue( GetRWord()->text(), Lex->GetValue( text ) );
		}
	}
	tjs_int text = Lex->ReadToVerline();
	if( text >= 0 ) {
		CurrentTag->setValue( GetRWord()->target(), Lex->GetValue( text ) );
	}
	// それ以降は属性として読み込む
	LineAttribute = true;
	ParseAttributes();

	Scenario->setTag( *CurrentTag.get() );
	CurrentTag->release();
}
//---------------------------------------------------------------------------
/**
%[
	type : "next",
	target : "filename",
	cond : "flag"
]
 */
void Parser::ParseNextScenario() {
	CurrentTag->release();
	CurrentTag->setTagName( GetRWord()->next() );

	tjs_int text = Lex->ReadToSpace();
	if( text >= 0 ) {
		CurrentTag->setValue( GetRWord()->target(), Lex->GetValue( text ) );
	}
	text = Lex->ReadToSpace();
	if( text >= 0 ) {
		const tTJSVariant& v = Lex->GetValue( text );
		tTJSVariantString* vs = v.AsStringNoAddRef();
		if( GetRWord()->if_ == ttstr(vs) ) {
			text = Lex->ReadToSpace();
			if( text >= 0 ) {
				CurrentTag->setValue( GetRWord()->cond(), Lex->GetValue( text ) );
			} else {
				ErrorLog( TJS_W(">の後のifに続く条件式がありません。") );
			}
		}
	}

	Scenario->setTag( *CurrentTag.get() );
	CurrentTag->release();
}
//---------------------------------------------------------------------------
/**
 タグかテキストを解析する
 */
bool Parser::ParseTag( Token token, tjs_int value ) {
	if( token == Token::EOL ) return false;
	TextAttribute = false;

	switch( token ) {
	case Token::TEXT:
		Scenario->addValueToCurrentLine( Lex->GetValue( value ) );
		return true;

	case Token::BEGIN_TAG:
		ParseTag();
		return true;

	case Token::VERTLINE: {	// ルビ or 文字装飾
		iTJSDispatch2* dic = TJSCreateDictionaryObject();
		RubyDecorationStack.push( dic );
		tTJSVariant v( dic, dic );
		Scenario->addValueToCurrentLine( dic );	// 空の辞書を追加しておく
		return true;
	}
	case Token::BEGIN_RUBY: {	// 《が来たので、ルビ文字であるとみなす
		int text = Lex->ReadToCharStrict( TJS_W( '》') );
		if( text >= 0 ) {
			if( !RubyDecorationStack.empty() ) {
				{	// rubyタグの内容を埋める
					Tag tag( RubyDecorationStack.top() );
					RubyDecorationStack.pop();
					tag.setAttribute( GetRWord()->text(), Lex->GetValue( text ) );
					tag.setTagName( GetRWord()->ruby() );
				}
				// [endruby]タグ追加
				Tag tag( GetRWord()->endruby() );
				Scenario->addTagToCurrentLine( tag );
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
				Tag tag( RubyDecorationStack.top() );
				RubyDecorationStack.pop();
				tag.setTagName( GetRWord()->ruby() );
			}
			// [endruby]タグ追加
			Tag tag( GetRWord()->endruby() );
			Scenario->addTagToCurrentLine( tag );
		} else {
			ErrorLog( TJS_W( "《の前に|がないため、ルビとして解釈できません。" ) );
		}
		return true;
	}
	case Token::BEGIN_TXT_DECORATION: {	// { が来たので、テキスト装飾であるとみなす
		int text = Lex->ReadToCharStrict( TJS_W( '}' ) );
		if( text >= 0 ) {
			if( !RubyDecorationStack.empty() ) {
				iTJSDispatch2* dic = RubyDecorationStack.top();
				RubyDecorationStack.pop();
				Tag* oldTag = CurrentTag.release();
				CurrentTag.reset( new Tag( dic ) );
				CurrentTag->setTagName( GetRWord()->textstyle() );
				TextAttribute = true;
				ParseAttributes();
				TextAttribute = false;
				Scenario->addTagToCurrentLine( *CurrentTag.get() );
				CurrentTag->release();
				CurrentTag.reset( oldTag );
			} else {
				ErrorLog( TJS_W( "'{'の前に'|'がないため、文字装飾として解釈できません。" ) );
			}
		} else {
			ErrorLog( TJS_W( "'{'の後に'}'がないため、文字装飾として解釈できません。" ) );
		}
		return true;
	}

	case Token::WAIT_RETURN: { // l タグ追加
		Tag tag( GetRWord()->l() );
		Scenario->addTagToCurrentLine( tag );
		return true;
	}

	case Token::INNER_IMAGE: {	// inlineimageタグ追加
		int text = Lex->ReadToCharStrict( TJS_W( ')' ) );
		if( text >= 0 ) {
			Tag tag( GetRWord()->inlineimage() );
			tag.setAttribute( GetRWord()->storage(), Lex->GetValue( text ) );
			Scenario->addTagToCurrentLine( tag );
		} else {
			ErrorLog( TJS_W( "':('の後に画像名がないか、')'がありません。" ) );
		}
		return true;
	}
	case Token::COLON: {	// emojiタグ追加
		int text = Lex->ReadToCharStrict( TJS_W( ':' ) );
		if( text >= 0 ) {
			Tag tag( GetRWord()->emoji() );
			tag.setAttribute( GetRWord()->storage(), Lex->GetValue( text ) );
			Scenario->addTagToCurrentLine( tag );
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
void Parser::ParseLine( tjs_int line ) {
	if( static_cast<tjs_uint>(line) < LineVector.size() ) {
		CurrentTag->release();
		ClearRubyDecorationStack();

		LineAttribute = false;
		TextAttribute = false;

		tjs_int length;
		const tjs_char *str = GetLine( line, &length );
		if( length == 0 ) {
			// 改行のみ
			Scenario->setValue( 0 );
		} else {
			Lex->reset( str, length );
			tjs_int value;
			Token token = Lex->GetFirstToken( value );
			if( HasSelectLine ) {
				if( token == Token::SELECT ) {
					ParseSelect(value);
				} else {
					HasSelectLine = false;
					// 選択肢オプション
					CurrentTag->setTagName( GetRWord()->selopt() );
					LineAttribute = true;
					ParseAttributes();

					Scenario->setTag( *CurrentTag.get() );
					CurrentTag->release();
				}
				return;
			}

			// first token
			switch( token ) {
			case Token::EOL:
				// タブのみの行も空行と同じ
				Scenario->setValue( 0 );
				break;

			case Token::BEGIN_TRANS: {	// >>> 
				// トランジション開始以降の文字列は無視し、begintransタグを格納するのみ
				Tag tag( GetRWord()->begintrans() );
				Scenario->setTag( tag );
				break;
			}
			case Token::END_TRANS:	// <<<
				ParseTransition();
				break;

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
				Scenario->setVoid();
				break;

			case Token::BEGIN_FIX_NAME: {
				FixTagName.Clear();
				tjs_int text = Lex->ReadToSpace();
				if( text >= 0 ) {
					FixTagName = ttstr( Lex->GetString( text ) );
				}
				Scenario->setVoid();
				break;
			}
			case Token::END_FIX_NAME:
				FixTagName.Clear();
				Scenario->setVoid();
				break;

			default:
				while( ParseTag( token, value ) ) {
					token = Lex->GetTextToken( value );
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
iTJSDispatch2* Parser::ParseText( const tjs_char* text ) {
	TJS_F_TRACE( "tTJSScriptBlock::ParseText" );

	// compiles text and executes its global level scripts.
	// the script will be compiled as an expression if isexpressn is true.
	if( !text ) return nullptr;
	if( !text[0] ) return nullptr;

	TJS_D( ( TJS_W( "Counting lines ...\n" ) ) )

	Script.reset( new tjs_char[TJS_strlen( text ) + 1] );
	TJS_strcpy( Script.get(), text );

	Lex->Free();
	CurrentTag.reset( new Tag() );
	Scenario.reset( new ScenarioDictionary() );
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

	HasSelectLine = false;
	LineAttribute = false;
	MultiLineTag = false;
	TextAttribute = false;
	FirstError.Clear();
	CompileErrorCount = 0;
	for( CurrentLine = 0; static_cast<tjs_uint>(CurrentLine) < LineVector.size(); CurrentLine++ ) {
		Scenario->setCurrentLine( CurrentLine );
		ParseLine( CurrentLine );
	}
	if( CompileErrorCount ) {
		TJS_eTJSCompileError( FirstError );
	}
	iTJSDispatch2* retDic = TJSCreateDictionaryObject();
	if( retDic ) {
		tTJSVariant tmp( Scenario->getArray(), Scenario->getArray() );
		retDic->PropSetByVS( TJS_MEMBERENSURE, GetRWord()->lines(), &tmp, retDic );
		Scenario->release();
	}
	return retDic;
}
//---------------------------------------------------------------------------
