
# メッセージ類のヘッダーファイルなどを生成します。
use utf8;
use Spreadsheet::ParseXLSX;
use Cwd;
use Encode qw/encode decode/;
use File::Spec;
use warnings;
use strict;

{
	my $jp_res_filename = 'string_table_jp.rc';
	my $en_res_filename = 'string_table_en.rc';
	my $res_header_filename = 'string_table_resource.h';
	my $src_header_filename = 'MDKMessages.h';
	my $src_get_mes = 'MDKMessages.cpp';
	my $excelfile = 'messages.xlsx';
	my $excel;
	my @mes_id;
	my @res_id;
	my @mes_jp;
	my @mes_en;
	my @mes_opt;
	my @tarminate;

	# Excel
	my $parser   = Spreadsheet::ParseXLSX->new();
	my $workbook = $parser->parse($excelfile);
	if ( !defined $workbook ) {
	    die $parser->error(), ".\n";
	}
	# シートを読み込む
	my @worksheets = $workbook->worksheets();
	my $worksheet = $worksheets[0];
	my ( $row_min, $row_max ) = $worksheet->row_range();
	my ( $col_min, $col_max ) = $worksheet->col_range();

	# 2行目から読む(1行目はタイトル)
	my $MesId;
	for my $row ( 1 .. $row_max ) {
		$MesId = $worksheet->get_cell( $row, 0 )->value();
		my $MesJp = $worksheet->get_cell( $row, 1 )->value();
		my $MesEn = $worksheet->get_cell( $row, 2 )->value();
		# リソースファイル用にエスケープする
		if( defined $MesJp ) {
			#$MesJp = decode( 'cp932', $MesJp );
			#$MesJp = encode( 'UTF-8', $MesJp );
			$MesJp =~ s/(\\")/""/g;
			$MesJp =~ s/(\\')/'/g;
		}
		if( defined $MesEn ) {
			$MesEn =~ s/(\\")/""/g;
			$MesEn =~ s/(\\')/'/g;
		}
		if( defined $MesId ) {
			my $ResId = $MesId;
			$ResId =~ s/^(MDK)/$1_/;
			$ResId =~ s/([a-z])([A-Z])/$1_$2/g;
			$ResId = uc $ResId;
			$ResId = "IDS_".$ResId;
			push @mes_id, $MesId;
			push @res_id, $ResId;
			push @mes_jp, $MesJp;
			push @mes_en, $MesEn;
		}
	}
	my $tarlength = @mes_id;
	push @tarminate, $tarlength;


	# String Table のリソースを出力
	open( FHJP, ">:raw:encoding(UTF-16LE):crlf:utf8", "$jp_res_filename" ) or die;
	open( FHEN, ">:raw:encoding(UTF-16LE):crlf:utf8", "$en_res_filename" ) or die;
	open FHH, ">$res_header_filename" or die;

	open FHMH, ">$src_header_filename" or die;
	open FHCPP, ">$src_get_mes" or die;

	print FHJP "\x{FEFF}"; #BOMを出力
	print FHJP "STRINGTABLE\n";
	print FHJP "BEGIN\n";
	print FHEN "\x{FEFF}"; #BOMを出力
	print FHEN "STRINGTABLE\n";
	print FHEN "BEGIN\n";

	print FHH  <<'HEADER';
// generated from gentext.pl Messages.xlsx
#ifndef __STRING_TABLE_RESOURCE_H__
#define __STRING_TABLE_RESOURCE_H__
HEADER

	print FHMH  <<'HEADER';
// generated from gentext.pl Messages.xlsx
#ifndef __MDK_MESSAGES_H__
#define __MDK_MESSAGES_H__
HEADER

	print FHCPP <<'CPPSRC';
// generated from gentext.pl Messages.xlsx
#ifdef _WIN32
#include <windows.h>
#endif
#include "tp_stub.h"
#include <memory>

#include "MDKMessages.h"
#include "string_table_resource.h"

static tjs_char MESSAGE_WORK_AREA[MAX_MESSAGE_LENGTH];
const int RESOURCE_IDS[NUM_MDK_MESSAGE_MAX] = {
CPPSRC

	my $length = @mes_id;
	my $maxlen = 24;
	my $mesmaxlen = 1024;
	for( my $i = 0; $i < $length; $i++ ) {
		my $len = length $res_id[$i];
		if( ($len+1) > $maxlen ) { $maxlen = ($len+1); }
		$len = length $mes_jp[$i];
		if( ($len+1) > $mesmaxlen ) { $mesmaxlen = ($len+1); }
		$len = length $mes_en[$i];
		if( ($len+1) > $mesmaxlen ) { $mesmaxlen = ($len+1); }
	}
	print FHMH "static const int MAX_MESSAGE_LENGTH = $mesmaxlen;\n";
	print FHMH "enum {\n";

	for( my $i = 0; $i < $length; $i++ ) {
		my $len = length $res_id[$i];
		my $line = "    ".$res_id[$i];
		my $header_res = "#define ".$res_id[$i];
		for( my $j = $len; $j < $maxlen; $j++ ) {
			$line .= " ";
			$header_res .= " ";
		}
		my $jpline = $line . "\"".$mes_jp[$i]."\"\n";
		my $enline = $line . "\"".$mes_en[$i]."\"\n";
		print FHJP $jpline;
		print FHEN $enline;
		my $id = $i + 10000; # 10000以降の番号に割り当てておく
		print FHH $header_res.$id."\n";

		my $enumid = $res_id[$i];
		$enumid =~ s/^(IDS_)//;
		$enumid = "NUM_".$enumid;
		print FHMH "\t".$enumid.",\n";
		print FHCPP "\t".$res_id[$i].",\n";
	}
	print FHMH "\tNUM_MDK_MESSAGE_MAX\n";
	print FHMH "};\n";

	print FHCPP <<'CPPSRC';
};

extern HINSTANCE TVPMDKParserInst;

ttstr TVPMdkGetText( int num ) {
	int len = ::LoadString( TVPMDKParserInst, RESOURCE_IDS[num], MESSAGE_WORK_AREA, MAX_MESSAGE_LENGTH );
	if( len <= 0 ) {
		return ttstr(TJS_W("Internal Error.") );
	} else {
		MESSAGE_WORK_AREA[len] = TJS_W('\0');
		return ttstr( MESSAGE_WORK_AREA );
	}
}
CPPSRC
	print FHJP "END\n";
	close FHJP;

	print FHEN "END\n";
	close FHEN;
	
	print FHH "#endif\n";
	close FHH;

	print FHMH "extern ttstr TVPMdkGetText( int num );\n";
	print FHMH "#endif\n";
	close FHMH;

	exit;
}
