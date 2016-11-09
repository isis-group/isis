%class-name VistaScanner
%namespace vista_internal
%filenames VistaScanner

%%

[ \t\n\r]+                          // skip white space chars.
V-data			return VistaParser::MAGIC;
history			return VistaParser::HISTORY_KEY;
image			return VistaParser::IMAGE_KEY;
[[:alnum:]_-]+	return VistaParser::WORD;
\"[^\"]+\"		return VistaParser::QUOTED_STRING;
