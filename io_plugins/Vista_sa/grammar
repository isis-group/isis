%scanner                VistaScanner.h
%scanner-token-function d_scanner.lex()
%debug
%class-name VistaParser
%scanner-class-name VistaScanner
%namespace vista_internal
%filenames VistaParser

%token MAGIC WORD QUOTED_STRING HISTORY_KEY IMAGE_KEY

%polymorphic STRING: std::string; SPAIR: std::pair<std::string,std::string>; MAP: isis::util::PropertyMap
%type <STRING> word
%type <SPAIR> entry
%type <MAP> block

%%

startrule:
	magic block
|
	magic history block
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
	MAGIC word
	{
		std::cout << "Magic " << $2 << std::endl;
	}
;

entry:
	word ':' word
	{
		std::cout << "Entry " << $1 << "=" << $3 << std::endl;
		$$ = std::make_pair($1,$3);
	}
|
	block
;

block_entries:
	// empty
|
	block_entries entry
;

block:
	'{' block_entries '}'
;

history:
	HISTORY_KEY ':' block
;

image:
	IMAGE_KEY block
;

chunk:
	IMAGE_KEY ':' image
;
