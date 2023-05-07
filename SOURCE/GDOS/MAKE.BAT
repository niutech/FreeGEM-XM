if not exist GDOS.OBJ copy OBJWOSRC\GDOS.OBJ .
if not exist LOAD.OBJ copy OBJWOSRC\LOAD.OBJ .
for %%a in (*.a86) do rasm86 %%a $szpz
for %%a in (*.c) do call lc %%a
link
