
#ifndef __MDK_TOKEN_H__
#define __MDK_TOKEN_H__

enum yytokentype {
     T_COMMA = 258,
     T_EQUAL = 259,
     T_AMPERSANDEQUAL = 260,
     T_VERTLINEEQUAL = 261,
     T_CHEVRONEQUAL = 262,
     T_MINUSEQUAL = 263,
     T_PLUSEQUAL = 264,
     T_PERCENTEQUAL = 265,
     T_SLASHEQUAL = 266,
     T_BACKSLASHEQUAL = 267,
     T_ASTERISKEQUAL = 268,
     T_LOGICALOREQUAL = 269,
     T_LOGICALANDEQUAL = 270,
     T_RBITSHIFTEQUAL = 271,
     T_LARITHSHIFTEQUAL = 272,
     T_RARITHSHIFTEQUAL = 273,
     T_QUESTION = 274,
     T_LOGICALOR = 275,
     T_LOGICALAND = 276,
     T_VERTLINE = 277,
     T_CHEVRON = 278,
     T_AMPERSAND = 279,
     T_NOTEQUAL = 280,
     T_EQUALEQUAL = 281,
     T_DISCNOTEQUAL = 282,
     T_DISCEQUAL = 283,
     T_SWAP = 284,
     T_LT = 285,
     T_GT = 286,
     T_LTOREQUAL = 287,
     T_GTOREQUAL = 288,
     T_RARITHSHIFT = 289,
     T_LARITHSHIFT = 290,
     T_RBITSHIFT = 291,
     T_PERCENT = 292,
     T_SLASH = 293,
     T_BACKSLASH = 294,
     T_ASTERISK = 295,
     T_EXCRAMATION = 296,
     T_TILDE = 297,
     T_DECREMENT = 298,
     T_INCREMENT = 299,
     T_NEW = 300,
     T_DELETE = 301,
     T_TYPEOF = 302,
     T_PLUS = 303,
     T_MINUS = 304,
     T_SHARP = 305,
     T_DOLLAR = 306,
     T_ISVALID = 307,
     T_INVALIDATE = 308,
     T_INSTANCEOF = 309,
     T_LPARENTHESIS = 310,
     T_DOT = 311,
     T_LBRACKET = 312,
     T_THIS = 313,
     T_SUPER = 314,
     T_GLOBAL = 315,
     T_RBRACKET = 316,
     T_CLASS = 317,
     T_RPARENTHESIS = 318,
     T_COLON = 319,
     T_SEMICOLON = 320,
     T_LBRACE = 321,
     T_RBRACE = 322,
     T_CONTINUE = 323,
     T_FUNCTION = 324,
     T_DEBUGGER = 325,
     T_DEFAULT = 326,
     T_CASE = 327,
     T_EXTENDS = 328,
     T_FINALLY = 329,
     T_PROPERTY = 330,
     T_PRIVATE = 331,
     T_PUBLIC = 332,
     T_PROTECTED = 333,
     T_STATIC = 334,
     T_RETURN = 335,
     T_BREAK = 336,
     T_EXPORT = 337,
     T_IMPORT = 338,
     T_SWITCH = 339,
     T_IN = 340,
     T_INCONTEXTOF = 341,
     T_FOR = 342,
     T_WHILE = 343,
     T_DO = 344,
     T_IF = 345,
     T_VAR = 346,
     T_CONST = 347,
     T_ENUM = 348,
     T_GOTO = 349,
     T_THROW = 350,
     T_TRY = 351,
     T_SETTER = 352,
     T_GETTER = 353,
     T_ELSE = 354,
     T_CATCH = 355,
     T_OMIT = 356,
     T_SYNCHRONIZED = 357,
     T_WITH = 358,
     T_INT = 359,
     T_REAL = 360,
     T_STRING = 361,
     T_OCTET = 362,
     T_FALSE = 363,
     T_NULL = 364,
     T_TRUE = 365,
     T_VOID = 366,
     T_NAN = 367,
     T_INFINITY = 368,
     T_UPLUS = 369,
     T_UMINUS = 370,
     T_EVAL = 371,
     T_POSTDECREMENT = 372,
     T_POSTINCREMENT = 373,
     T_IGNOREPROP = 374,
     T_PROPACCESS = 375,
     T_ARG = 376,
     T_EXPANDARG = 377,
     T_INLINEARRAY = 378,
     T_ARRAYARG = 379,
     T_INLINEDIC = 380,
     T_DICELM = 381,
     T_WITHDOT = 382,
     T_THIS_PROXY = 383,
     T_WITHDOT_PROXY = 384,
     T_CONSTVAL = 385,
     T_SYMBOL = 386,
     T_REGEXP = 387,
     T_VARIANT = 388,
   };

enum class Token : int {
	EMPTY = -1,
	EOL = 0,
	NEXT_SCENARIO,	// >
	BEGIN_TRANS,	// >>>
	END_TRANS,		// <<<
	AT,				// @
	LINE_COMMENTS,	// //
	SELECT,			// [0-9]+\.
	LABEL,			// #
	BEGIN_FIX_NAME,	// <=
	END_FIX_NAME,	// =>

	BEGIN_TAG,		// [
	TEXT,			// normal text
	VERTLINE,		// |
	WAIT_RETURN,	// >
	BEGIN_RUBY,		// 《
	END_RUBY,		// 》
	BEGIN_TXT_DECORATION,	// {
	END_TXT_DECORATION,	// }
	
	GT,		// >
	CONSTVAL,
	LT,		// <
	EQUAL,	// =
	EXCRAMATION,	// !
	AMPERSAND,		// &
	DOT,			// .
	PLUS,			// +
	MINUS,			// -
	ASTERISK,		// *
	SLASH,			// /
	BACKSLASH,		// \
	PERCENT,		// %
	CHEVRON,		// ^
	LBRACKET,		// [
	RBRACKET,		// ]
	LPARENTHESIS,	// (
	RPARENTHESIS,	// )
	TILDE,			// ~
	QUESTION,		// ?
	COLON,			// :
	DOUBLE_COLON,	// ::
	COMMA,			// ,
	SEMICOLON,		// ;
	LBRACE,			// {
	RBRACE,			// }
	SHARP,			// #
	DOLLAR,			// $
	SINGLE_TEXT,	// '...'
	DOUBLE_TEXT,	// "..."
	NUMBER,			// 0-9
	OCTET,			// <% ...  %>
	T_TRUE,			// true
	T_FALSE,		// false
	T_NULL,			// null
	SYMBOL,			// 
	CONSTVAL,		//

};



#endif
