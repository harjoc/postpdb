#include <Windows.h>
#include <stdio.h>
#include <DbgHelp.h>

#include <string>

using namespace std;

#pragma warning(disable:4996)

#define errx(code, ...)	do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); exit(code); } while(0)
#define err(code, ...)	do { fprintf(stderr, __VA_ARGS__); perror(" "); exit(code); } while(0)
#define warnx(...)		do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define warn(...)		do { fprintf(stderr, __VA_ARGS__); perror(" "); } while(0)
#define trace warnx

#define PROC ((HANDLE)8)

#define INSTALLDIR "installdir"

int main(int argc, char *argv[])
{
	if (argc < 3)
		errx(1, "syntax: %s results.txt results-resolved.txt", argv[0]);

	if (GetFileAttributes(L"installdir") == INVALID_FILE_ATTRIBUTES)
		errx(1, "installdir/ should contain a copy of your installed app, at least the executables");

	trace("initializing dbghelp");

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_DEBUG);	

	if (!SymInitialize(PROC, NULL, FALSE))
		errx(1, "SymInitialize() error %d", GetLastError());

	FILE *fin = fopen(argv[1], "r");
	if (!fin) err(1, "fopen(%s)", argv[1]);

	FILE *fout = fopen(argv[2], "w");
	if (!fout) err(1, "fopen(%s)", argv[2]);

	for (;;)
	{
		char buf[1024];
		if (!fgets(buf, 1024, fin))
			break;

		bool resolved = false;
		DWORD64 base = 0;
		//--

		do {
			// <KERNEL32.dll+0x90baf>

			//const char *paren = strchr(buf, '(');
			//if (!paren) break;

			const char *bracket0 = strchr(buf, '<');
			if (!bracket0) break;
		
			const char *bracket1 = strchr(bracket0, '>');
			if (!bracket1) break;

			const char *plus = strchr(bracket0, '+');
			if (!plus || plus > bracket1) break;

			char addrstr[32];
			char module[128];
			char ofsstr[32];

			//_snprintf(addrstr, 32, "%.*s", bracket0-paren-1, paren+1);
			_snprintf(module, 128, "%.*s", plus-bracket0-1, bracket0+1);
			_snprintf(ofsstr, 32, "%.*s", bracket1-plus-1, plus+1);

			DWORD64 addr = stoull(addrstr, NULL, 16);
			DWORD64 ofs = stoull(ofsstr, NULL, 16);

			//trace("%llx [%s] [%s] %llx\n", addr, module, ofsstr, ofs);

			char path[128];
			_snprintf(path, 128, "%s\\%s", INSTALLDIR, module);
		
			base = SymLoadModuleEx(PROC, NULL, path, NULL, 0, 0, NULL, 0);
			if (base==0)
			{
				warnx("SymLoadModuleEx(%s)error %d", module, GetLastError());
				break;
			}

			warnx("loaded pdb for %s", module);

			struct {
				SYMBOL_INFO sym;
				char data[128];
			} sym_buf;

			sym_buf.sym.SizeOfStruct = sizeof(SYMBOL_INFO);
			sym_buf.sym.MaxNameLen = sizeof(sym_buf.data);

			DWORD64 sym_displacement;

			if (!SymFromAddr(PROC, base + ofs, &sym_displacement, &sym_buf.sym))
			{
				warnx("SymFromAddr(%s+0x%llx) error %d", module, ofs, GetLastError());
				break;
			}
			
			//sym_buf.sym.
			IMAGEHLP_LINE64 line = {};
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			DWORD line_displacement;

			BOOL have_line = SymGetLineFromAddr64(PROC, base + ofs, &line_displacement, &line);
			if (!have_line)
			{
				warnx("SymGetLineFromAddr64(%s+0x%llx) error %d", module, ofs, GetLastError());
				break;
			}

			char symbol[1024];
			_snprintf(symbol, 1024, "# %s!%s+0x%x [", module, sym_buf.sym.Name, sym_displacement);
			fprintf(fout, symbol);			
			fprintf(fout, "%s:%d", line.FileName, line.LineNumber);
			fprintf(fout, "]\n");
			
			resolved = true;
		} while (0);
		
		if (!resolved) fputs(buf, fout);

		// dbghelp can only handle one DLL loaded at a time without a process

		base = SymUnloadModule64(PROC, base);
	}

	fclose(fin);
	fclose(fout);

	return 0;
}
