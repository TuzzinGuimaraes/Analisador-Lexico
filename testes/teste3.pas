{ Teste com laco while e variaveis com nomes maiusculos/minusculos }
program Contador;
var
   N   : integer;
   Acc : real;
begin
   N   := 1;
   Acc := 0.0;
   while N <= 10 do
   begin
      Acc := Acc + N;
      N   := N + 1
   end
end.
