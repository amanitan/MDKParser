
#include "ReservedWord.h"

ReservedWord::ReservedWord() {
	endtrans_ = TJSMapGlobalStringMap(TJS_W("endtrans"));
	begintrans_ = TJSMapGlobalStringMap(TJS_W("begintrans"));

	storage_ = TJSMapGlobalStringMap(TJS_W("storage"));
	type_ = TJSMapGlobalStringMap(TJS_W("type"));
	name_ = TJSMapGlobalStringMap(TJS_W("name"));
	value_ = TJSMapGlobalStringMap(TJS_W("value"));

	tag_ = TJSMapGlobalStringMap(TJS_W("tag"));
	label_ = TJSMapGlobalStringMap(TJS_W("label"));
	select_ = TJSMapGlobalStringMap(TJS_W("select"));
	next_ = TJSMapGlobalStringMap(TJS_W("next"));
	selopt_ = TJSMapGlobalStringMap( TJS_W( "selopt" ) );

	attribute_ = TJSMapGlobalStringMap(TJS_W("attribute"));
	parameter_ = TJSMapGlobalStringMap(TJS_W("parameter"));
	command_ = TJSMapGlobalStringMap(TJS_W("command"));
	ref_ = TJSMapGlobalStringMap(TJS_W("ref"));
	file_ = TJSMapGlobalStringMap(TJS_W("file"));
	prop_ = TJSMapGlobalStringMap(TJS_W("prop"));

	trans_ = TJSMapGlobalStringMap(TJS_W("trans"));
	charname_ = TJSMapGlobalStringMap(TJS_W("charname"));
	alias_ = TJSMapGlobalStringMap(TJS_W("alias"));
	description_ = TJSMapGlobalStringMap(TJS_W("description"));
	text_ = TJSMapGlobalStringMap(TJS_W("text"));
	image_ = TJSMapGlobalStringMap(TJS_W("image"));
	target_ = TJSMapGlobalStringMap(TJS_W("target"));
	if_ = TJSMapGlobalStringMap(TJS_W("if"));
	cond_ = TJSMapGlobalStringMap(TJS_W("cond"));
	comment_ = TJSMapGlobalStringMap(TJS_W("comment"));

	voice_ = TJSMapGlobalStringMap(TJS_W("voice"));
	time_ = TJSMapGlobalStringMap(TJS_W("time"));
	wait_ = TJSMapGlobalStringMap(TJS_W("wait"));
	fade_ = TJSMapGlobalStringMap(TJS_W("fade"));

	lines_ = TJSMapGlobalStringMap( TJS_W( "lines" ) );

	ruby_ = TJSMapGlobalStringMap( TJS_W( "ruby" ) );
	endruby_ = TJSMapGlobalStringMap( TJS_W( "endruby" ) );
	l_ = TJSMapGlobalStringMap( TJS_W( "l" ) );
	textstyle_ = TJSMapGlobalStringMap( TJS_W( "textstyle" ) );
	inlineimage_ = TJSMapGlobalStringMap( TJS_W( "inlineimage" ) );
	emoji_ = TJSMapGlobalStringMap( TJS_W( "emoji" ) );
}

static ReservedWord* gReservedWord = nullptr;
void InitializeReservedWord() {
	if( !gReservedWord ) {
		gReservedWord = new ReservedWord();
	}
}
void FinalizeReservedWord() {
	if( gReservedWord ) {
		delete gReservedWord;
		gReservedWord = nullptr;
	}
}
const ReservedWord* GetRWord() {
	return gReservedWord;
}
