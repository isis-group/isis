%scanner VistaScanner.h
%scanner-token-function d_scanner.lex()
%class-name VistaParser
%scanner-class-name VistaScanner
%namespace vista_internal
%filenames VistaParser

%token MAGIC_KEY WORD QUOTED_STRING HISTORY_KEY IMAGE_KEY

%polymorphic STRING: std::string; SPAIR: std::pair<std::string,std::string>; MAP: isis::util::PropertyMap; SLIST: std::list<std::string>
%type <STRING> word
%type <SPAIR> entry
%type <MAP> block block_entries
%type <SLIST> history history_list

%%

vista:
	magic block 
	{
		VistaParser::root.transfer($2);
		ACCEPT();
	}
;

block:
	'{' block_entries '}'{($$).transfer($2);}
;

block_entries:
	// empty
	{$$=isis::util::PropertyMap();}
|
	block_entries entry
	{
		($$).setValueAs(($2).first.c_str(),($2).second);
	}
|
	block_entries word ':' block
	{
/*   		($4).print(std::cout << "Got Sub-Block " << $2 << std::endl); */
		($$).touchBranch( ($2).c_str() ).transfer($4);
	}
|
	block_entries IMAGE_KEY ':' IMAGE_KEY block {
/*  		($5).print(std::cout << "Got Image" << std::endl);  */
		VistaParser::ch_list.push_back($5);
	}
|
	block_entries history
	{
/* 		 isis::util::listToOStream(($5).begin(),($5).end(),std::cout << "Got history ") << std::endl;  */
		 ($$).setValueAs("history",($2));
	}
;

entry:
	word ':' word
	{
/*  		std::cout << "Got " << std::make_pair($1,$3) << std::endl; */
		$$ = std::make_pair($1,$3);
	}
;

word:
	WORD{$$ = std::string(d_scanner.matched());}
|
	QUOTED_STRING
	{
		const std::string buff(d_scanner.matched());
		$$ = buff.substr(1,buff.length()-2);
	}
;

magic:
	MAGIC_KEY word
	{
		// @todo check version
	}
;

history:
	HISTORY_KEY ':' '{' history_list '}' {$$=$4;}
|
	HISTORY_KEY ':' word {($$).push_back($3);}
;

history_list:
	//empty
	{$$=std::list<std::string>();}
|
	history_list entry {($$).push_back(($2).first+":\t"+($2).second);}
;
	
