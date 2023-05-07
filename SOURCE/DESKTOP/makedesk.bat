for %%a in (*.A86) do rasm86 %%a $szpz
rasm86 rtlasm.a86 $szpznc
for %%a in (*.c) do hc %%a
linkit
