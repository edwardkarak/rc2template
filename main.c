#include <windows.h>
#include <stdio.h>

void die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	
	getchar();
	va_end(ap);
	exit(EXIT_FAILURE);
}

void writetemplate(DWORD nres, const unsigned char *buf, int dlgid, FILE *fout)
{
	fprintf(fout, "static unsigned char dlg%d[] = {\n\t", dlgid);
	for (DWORD i = 0; i < nres; ++i) {
		fprintf(fout, "0x%x%s", 
				(unsigned char) (buf[i] & 0xff), i == nres - 1 ? "" : ", ");
		
		if (i && (i % 14 == 0))		/* wrapping */
			fprintf(fout, "\n\t");
	}
	fprintf(fout, "\n};\n");
}

enum {
	PROGNAME,
	MODFILE,
	DLGID,
	OUTFILE,
};

int main(int argc, char *argv[])
{
	HMODULE mod;
	HRSRC res;
	HGLOBAL globalres;
	DWORD nres;
	char *endp;
	unsigned char *buf;
	int dlgid;
	FILE *fout;

	if (argc < 4)
		die("Usage: %s MODULE_FILE DLGID OUTFILE", argv[PROGNAME]);

	dlgid = strtol(argv[DLGID], &endp, 0);
	if (*endp != '\0')
		die("Cannot parse dialog ID %s", argv[DLGID]);

	if ((mod = LoadLibraryEx(argv[MODFILE], 0, LOAD_LIBRARY_AS_DATAFILE)) == NULL)
		die("Cannot load module %s. Win32 error code %d", argv[MODFILE], GetLastError());

	if ((res = FindResource(mod, MAKEINTRESOURCE(dlgid), RT_DIALOG)) == NULL)
		die("Cannot find dialog resource with id %d. Win32 error code %d", dlgid, GetLastError());

	if ((globalres = LoadResource(mod, res)) == NULL)
		die("Cannot load resource. Win32 error code %d", GetLastError());

	if ((buf = (unsigned char *) LockResource(globalres)) == NULL)
		die("Cannot lock resource. Win32 error code %d", GetLastError());

	nres = SizeofResource(mod, res);
	
	if (fileExists(argv[OUTFILE])) {
		char line[80];
		printf("The file %s will be overwritten. Proceed (y/n)?");
		fgets(line, stdin, 80);
		if (!(line[0] == 'Y' || line[0] == 'y'))
			die("Permission denied to overwrite %s. Retry with a different name.", argv[OUTFILE]);	
	}

	if ((fout = fopen(argv[OUTFILE], "w")) == NULL)
		die("Cannot open %s for writing: %s", argv[OUTFILE], strerror(errno));

	writetemplate(nres, buf, dlgid, fout);

	fclose(fout);
	FreeLibrary(mod);
	fprintf(stderr, "Generated %s (%d bytes) successfully\n", argv[OUTFILE], nres);	/* use stderr so it can't be piped into a file by accident */
	getchar();
	return EXIT_SUCCESS;
}
int fileExists(const char *const filename)
{
	int ret = 0;
	FILE *fp;
	
	if ((fp = fopen("sample.txt","r")) != NULL) {
		ret = 1;
		fclose(file);
	}
	return ret;
}	
