REM put c source disk in drive a: and 
REM misc source in drive b:
PAUSE
verify on
copy *.c a:
copy *.h b:
copy *.a86 b:
copy *.inp b:
copy *.mke b:
copy *.bat b:
copy gem30.exe b:
copy gem30.sym b:
copy gem.rsc b:
copy gem.dfn b:
copy gem.rsh b:
verify on
REM AES backup complete
