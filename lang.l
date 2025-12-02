%option noyywrap yylineno
%option outfile="lexer.c" header-file="lexer.h"
%{
#include "lang.h"
#include "parser.h"
%}

%%

"0|[1-9][0-9]*" {
    return TM_NAT_RE;
}

"[_A-Za-z][_A-Za-z0-9]*" {
    return TM_IDENT_RE;
}

"[ \\t\\n\\r]" {
    return TM_BLANK_RE;
}

\"[_A-Za-z][_A-Za-z0-9]*\" {
    yylval.i = new_str(yytext, yyleng);
    return TM_KEYWORD;
}

\"[^A-Za-z0-9_]{1,2}\" {
    yylval.i = new_str(yytext, yyleng);
    return TM_SYMBOL;
}

0|[1-9][0-9]* {
    yylval.n = build_nat(yytext, yyleng);
    return TM_NAT;
}

"var" {
    return TM_VAR;
}

"if" {
    return TM_IF;
}

"then" {
    return TM_THEN;
}

"else" {
    return TM_ELSE;
}

"while" {
    return TM_WHILE;
}

"do" {
    return TM_DO;
}

"for" {
    return TM_for;
}

"continue" {
    return TM_continue;
}

"break" {
    return TM_break;
}

"return" {
    return TM_return;
}

"func" {
    return TM_func;
}

"proc" {
    return TM_proc;
}

[_A-Za-z][_A-Za-z0-9]* {
    yylval.i = new_str(yytext, yyleng);
    return TM_IDENT;
}

";" {
    return TM_SEMICOL;
    }

"(" {
    return TM_LEFT_PAREN;
    }

")" {
    return TM_RIGHT_PAREN;
    }

"{" {
    return TM_LEFT_BRACE;
    }

"}" {
    return TM_RIGHT_BRACE;
    }

"+" {
    return TM_PLUS;
    }

"-" {
    return TM_MINUS;
    }

"*" {
    return TM_MUL;
    }

"/" {
    return TM_DIV;
    }

"%" {
    return TM_MOD;
    }

"<" {
    return TM_LT;
    }

">" {
    return TM_GT;
    }

"<=" {
    return TM_LE;
    }

">=" {
    return TM_GE;
    }

"==" {
    return TM_EQ;
    }

"!=" {
    return TM_NE;
    }

"=" {
    return TM_ASGNOP;
    }

"&&" {
    return TM_AND;
    }

"||" {
    return TM_OR;
    }

"!" {
    return TM_NOT;
    }

"&" {
    return TM_and;
    }

"," {
    return TM_comma;
    }

[ \t\n\r]    { };

.   {printf("%s",yytext);
     return -1; }
%%
