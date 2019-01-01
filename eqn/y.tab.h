/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     YYLBRACE = 258,
     YYRBRACE = 259,
     YYLSQUARE = 260,
     YYRSQUARE = 261,
     YYCOLON = 262,
     YYNUMBER = 263,
     YYNAME = 264,
     YYFUNNAME = 265,
     YYCONSTANT = 266,
     YYEQUAL = 267,
     YYDOTDOT = 268,
     YYCOMMA = 269,
     YYMINUS = 270,
     YYPLUS = 271,
     UMINUS = 272,
     YYDOT = 273,
     YYDEVIDE = 274,
     YYTIMES = 275,
     ASSOCIATE = 276,
     YYPOWER = 277
   };
#endif
#define YYLBRACE 258
#define YYRBRACE 259
#define YYLSQUARE 260
#define YYRSQUARE 261
#define YYCOLON 262
#define YYNUMBER 263
#define YYNAME 264
#define YYFUNNAME 265
#define YYCONSTANT 266
#define YYEQUAL 267
#define YYDOTDOT 268
#define YYCOMMA 269
#define YYMINUS 270
#define YYPLUS 271
#define UMINUS 272
#define YYDOT 273
#define YYDEVIDE 274
#define YYTIMES 275
#define ASSOCIATE 276
#define YYPOWER 277




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
	eqnode *y_equ;
	double y_dbl;
	char   *y_str;
	eqn_funs *y_fun;
      } YYSTYPE;
/* Line 1248 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



