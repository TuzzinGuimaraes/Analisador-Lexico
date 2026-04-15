{ Teste com operadores relacionais e numero real com expoente }
program Calculo;
var
   a, b : integer;
   r    : real;
begin
   a := 5;
   b := 3;
   r := 1.5E2;
   if a <> b then
   begin
      if a >= b then
         a := a - b
      else
         b := b - a
   end
end.
