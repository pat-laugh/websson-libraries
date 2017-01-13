========================================================================
    STATIC LIBRARY : WebssonParser Project Overview
========================================================================

There are two constructors for Parser: Parser() and Parser(Language).
Parser(Language) sets the Parser's expression separator and decimal
separator depending on the language. '\n' is always a valid expression
separator.

Language | Expression Separator | Decimal Separator
-------- | -------------------- | -----------------
DEFAULT / EN | newline or comma | period
FR | newline or semi-colon | comma
INT | newline or semi-colon | period or comma

The parser has two sets of variables: "global" vars are all the vars
the parser owns, and "local" vars are strictly those obtained from
a directly specified document (excludes vars coming from imports).
