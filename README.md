# postpdb
Postprocess drmemory and dynamorio stacks using PDB symbol files.

If you collected a log of memory access error stacks or other stacks 
using drmemory or another dynamorio tool, but didn't set up `_NT_SYMBOL_PATH`
to have proper symbols, you can use this tool to postprocess the output
and convert the raw entries like:

`<mydll.dll!DllMain+0x34593>`

into

`mydll.dll!Internal::Member+0x12 [c:\work\internal.cpp:456]`

Compile using Visual Studio 2010 and put dbghelp.dll, dbgeng.dll and symsrv.dll from a recent Windows Debugging Tools release in the same directory as the executable. Or grab a binary from [Releases](releases). But you wouldn't run a random binary from the internet. Would you ?
