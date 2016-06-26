# postpdb
Postprocess drmemory and dynamorio stacks using PDB symbol files.

If you collected a log of memory access error stacks or other stacks 
using drmemory or another dynamorio tool, but didn't set up `_NT_SYMBOL_PATH`
to have proper symbols, you can use this tool to postprocess the output
and convert the raw entries like:

<mydll.dll!DllMain+0x34593>

into

mydll.dll!Internal::Member+0x12 [c:\work\internal.cpp:456]

