@echo off
echo 'disassemble Foo1.class'
javap -c Foo1 > Foo1_java_dasm.txt
echo 'disassemble Foo1.dex'
dexdump -d -f -h Foo1.dex > Foo1_dex_dasm.txt
