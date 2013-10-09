%{
#include "y.tab.h"

%}

%x afterequal
%x incomment
%%

<incomment>[^\n]+ ;

<afterequal>float|binary {
  yylval = (YYSTYPE)strdup(yytext);
  return TYPE;
}

<INITIAL>[a-zA-Z][a-zA-Z0-9_]* {
  yylval = (YYSTYPE)strdup(yytext);
  return IDENTIFIER;
}

<afterequal,INITIAL>[0-9]+ {
  yylval = (YYSTYPE)strdup(yytext);
  return NUMBER;
}

<afterequal>[-+]?[0-9]*\.?[0-9]* {
  yylval = (YYSTYPE)strdup(yytext);
  return FLOAT;
}

<afterequal>[0-9a-fA-F]+ {
  yylval = (YYSTYPE)strdup(yytext);
  return HEXNUMBER;
}

<afterequal,INITIAL>#   { BEGIN(incomment); }

<afterequal>\"[^"]*\" {
  yylval = (YYSTYPE)strdup(yytext);
  return STRING;
}

<afterequal,INITIAL>[ \t]+ ; // Get rid of whitespace

<INITIAL>\. {
  return *yytext;
}

<afterequal>, {
  return *yytext;
}

= {
  BEGIN(afterequal);
  return *yytext;
}

<afterequal,incomment,INITIAL>\n {
  BEGIN(INITIAL);
  return *yytext;
}

.   yyerror("invalid character");
%%


int yywrap(void) {
  return 1;
}
