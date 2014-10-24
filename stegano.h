#ifndef STEGANO_H
#define STEGANO_H

#define PROG_NAME "stegano"

#define PROG_VERSION	1
#define PROG_SUBVERSION	0
#define PROG_REVISION	0

#define champs(n,tab,file) for (int i=n-1; i>=0; i--){ tab[i] = fgetc (file); }


typedef struct BMP_HEADER
{
	char identifier[2]; /* Contient toujours l'octet 'B' suivit de l'octet 'M' */
	int fileSize; /* Taille totale du fichier en octet */
	int reserved; /* Champs réservé, doit être égal à 0 */
	int dataOffset; /* Nombre d'octets séparant le début du fichier des données de l'image */
	int headerSize; /* Taille en octet de l'header */
	int width; /* Largeur de l'image en pixels */
	int height; /* Hauteur de l'image en pixels */
	short planes; /* Nombre de plans */
	short bitsPerPixels; /* Nombre de bit nécessaires pour représenter un pixel */
	int compression; /* Type de compression */
	int bitmapDataSize; /* Taille en octets des données de l'image */
	int hResolution; /* Résolution horizontale de l'image en pixels par mètre */
	int vResolution; /* Résolution verticale de l'image en pixels par mètre */
	int colors; /* Nombre de couleurs dans l'image */
	int importantColors; /* Nombre de couleurs importantes*/
} BMP_HEADER;

void print_header (BMP_HEADER *header);
int tab2octet (int tab[], int size);
int loadBitmapHeader(FILE *fichier, BMP_HEADER *entete);
int isBitmapHeaderCorrect(BMP_HEADER *header);
int getBit(char *m, int n);
unsigned char *loadBitmapDatas(FILE *file, BMP_HEADER *header);
int saveBitmapDatas(FILE *file, BMP_HEADER *header, unsigned char *pixels);
void createPermutationFunction(int *tab, int size, unsigned int key);
int hideText(char *message, int *size, int *tab, unsigned char *pixels);

#endif /* STEGANO_H */
