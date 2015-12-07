%{
  #include <stdio.h>
  #define TOKEN(TYPE) printf("%s\n", #TYPE)
%}

%option noyywrap

%%

[0-9]+ { TOKEN(DIGIT); }
