
%{

enum tokens {
	NL = 1,
	TAB,
	SPACE,
	TOP,
	TID,
	TCHAR,
	TSRING,
	TFLOAT,
	TDINT,
	TXINT,
	TRINT
};


%}

	/* http://www.kylheku.com/cgit/txr/tree/parser.l  [-!#$%&*+./<=>?@\^|~] */


ffrac		\.[0-9]+
fexp		[Ee][+-]?[0-9]+


%%

\#[^\n]*(\n)					
\n										{return NL;}
\r										{}
\t										{return TAB;}
" "										{return SPACE;}
[a-zA-Z_][a-zA-Z0-9_]*					{return TID;}
\$(.|\\\.)								{return TCHAR;}
\"([^\"]|\\\.)*\"						{return TSTRING;}
[0-9_]+({ffrac}|{fexp}|{ffrac}{fexp})	{return TFLOAT;}
[0-9_]+									{return TDINT;}
0x[0-9a-fA-F]+							{return TXINT;}
[0-9]+r[0-9A-Za-z_]+					{return TRINT;} 
	/*reemplazar lo anterior por muchas reglas, una para cada
	base para asegurar que sea correcto el numero*/
[-!#%&*+./<=>@\^|~]+					{return TOP;}



%%

int main() {
	int tok;
	
	while(tok = yylex()) {
		cout << tok << endl;
	}

}

