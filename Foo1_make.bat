@echo off
echo 'compile Foo1 to class'
javac Foo1.java
echo 'transfer Foo1.class to dex'
dx --dex --output=Foo1.dex Foo1.class
