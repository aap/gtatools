#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	FILE *dirf, *imgf;
	FILE *f;
	int len;
	unsigned int dirPos, imgPos, size, written;
	char buf[24];
	int i;
	int ver2f;

	argc--;
	argv++;

	if ((imgf = fopen("gta.img", "wb")) == NULL) {
		fprintf(stderr, "error: can't open gta.img\n");
		return 1;
	}

	ver2f = 0;
	if (strcmp(*argv, "-v2") == 0) {
		ver2f = 1;
		argc--;
		argv++;
		dirf = imgf;
	} else {
		if ((dirf = fopen("gta.dir", "wb")) == NULL) {
			fprintf(stderr, "error: can't open gta.dir\n");
			return 1;
		}
	}

	dirPos = 0;
	imgPos = ver2f ?  imgPos = (((argc*32)+8)+2047)/2048 : 0;
	if (ver2f) {
		fwrite("VER2", 1, 4, dirf);
		fwrite(&argc, 4, 1, dirf);
		dirPos += 8;
	}
	for (i = 0; i < argc; i++) {
		if ((f = fopen(argv[i], "rb")) == NULL) {
			fprintf(stderr, "error: can't open %s\n", argv[i]);
			return 1;
		}

		fseek(imgf, imgPos*2048, SEEK_SET);
		do {
			char b[2048];
			size = fread(b, 1, 2048, f);
			fwrite(b, 1, size, imgf);
			written += size;
		} while (size == 2048);
		written = (written + 2047) >> 11;
		fclose(f);

		fseek(dirf, dirPos, SEEK_SET);
		fwrite(&imgPos, 4, 1, dirf);
		fwrite(&written, 4, 1, dirf);

		len = strlen(argv[i]);
		if (len > 24)
			printf("warning: %s has more than 24 characters\n",
			       argv[i]);
		memset(buf, 0, 24);
		memcpy(buf, argv[i], len);
		fwrite(buf, 1, 24, dirf);

		imgPos += written;
		dirPos += 32;
		fseek(imgf, imgPos*2048-1, SEEK_SET);
		fputc(0, imgf);
	}

	if (!ver2f)
		fclose(dirf);
	fclose(imgf);

	return 0;
}
