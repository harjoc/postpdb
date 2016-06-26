@echo off

set _NT_SYMBOL_PATH=srv*c:\temp\pdb*\\symsrv\symbols

debug\postpdb %*
