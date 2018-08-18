## Grammer
<Program>   -> (<Func>|<StatList>)+
<Func>      -> <Type>'('<ParamList>')''{'<StatList>'}'

<ParamList> -> <Param>(, <Param>)*
<Param>     -> <Type><Ident>

<StatList>  -> <Stat>; (<Stat>)*
<Stat>      -> return <Expr>

<Expr>      -> <Term> ((+|-) <Term>)* | <>
<Term>      -> <Factor> ((/|*) <Factor>)*
<Factor>    -> <IntLit>
<IntLit>    -> (0-9)+
<Type> -> int
