{ Erro: multiplos caracteres invalidos }
program MultiErro;
var
   x : integer;
begin
   x := 5;
   x := x % 2;
   x := x $ 1
end.
