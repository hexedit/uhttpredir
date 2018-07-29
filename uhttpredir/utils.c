#include <utils.h>

#include <stdio.h>
#include <stdlib.h>

char * load_file(const char *file)
{
    size_t size;
    char *data;
    FILE *fp;

    fp = fopen(file, "r");
    if (0 == fp) return 0;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    data = malloc(size);
    if (0 != data)
    {
        fseek(fp, 0, SEEK_SET);
        if (fread(data, 1, size, fp) != size)
        {
            free(data);
            data = 0;
        }
    }

    fclose(fp);
    return data;
}

#ifdef _WIN32

char *win32_strerror(int err)
{
	static char res[1024];
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) res,
		0,
		NULL
	);
	return res;
}

#endif /* _WIN32 */
