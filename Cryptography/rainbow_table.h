#include "spn.h"
#include "file_encrypt_system.h"
struct Line{
	char begin[8];
	char end[8];
};
void RainbowTable(int len, Bit8 *targetmd);
void getRainbowTable(char *filename, int len);
bool cmp(Line x, Line y);
void Reduction(Bit8 *md, char *pwd, int step);
int getPassword(char *filename, Bit8 *md, int len);