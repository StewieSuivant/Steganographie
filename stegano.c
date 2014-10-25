#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stegano.h"


/***********************************************************
                       print_header function
***********************************************************/
void print_header(BMP_HEADER *header)
{
  	fprintf(stdout, "identifier = '%c%c'\n"
					"fileSize = '%d'\n"
					"reserved = '%d'\n"
					"dataOffset = '%d'\n"
					"headerSize = '%d'\n"
					"width = '%d'\n"
					"height = '%d'\n"
					"planes = '%d'\n"
					"bitsPerPixels = '%d'\n"
					"compression = '%d'\n"
					"bitmapDataSize = '%d'\n"
					"hResolution = '%d'\n"
					"vResolution = '%d'\n"
					"colors = '%d'\n"
					"importantColors = '%d'\n",
					header->identifier[0], header->identifier[1],
					header->fileSize, header->reserved, header->dataOffset,
					header->headerSize, header->width, header->height,
					header->planes, header->bitsPerPixels, header->compression,
					header->bitmapDataSize, header->hResolution,
					header->vResolution, header->colors, header->importantColors
					);
}


/***********************************************************
                  tab2octet function
***********************************************************/
int tab2octet(int tab[], int size)
{
	int res = 0, bit = 0;

	for (int i=0; i<size ; i++)
	{
		for (int j = 7; j >= 0; j--)
		{
			bit = tab[i] & ((int)1<<j);
			res = res | bit;
		}
		if (i != (size-1))
			res = res << 8;
	}

	return res;
}

/***********************************************************
                  loadBitmapHeader function
***********************************************************/
int loadBitmapHeader(FILE *file, BMP_HEADER *header)
{
	int tab4[4] = {0}, tab2[2] = {0};

	/* identifier */
	for (int i=0; i<2; i++)
	{
		header->identifier[i] = fgetc (file);
	}

	/* fileSize */
	champs(4,tab4,file);
	header->fileSize = tab2octet (tab4, 4);

	/* reserved */
	champs(4,tab4,file);
	header->reserved = tab2octet (tab4, 4);
	
	/* dataOffset */
	champs(4,tab4,file);
	header->dataOffset = tab2octet (tab4, 4);

	/* headerSize */
	champs(4,tab4,file);
	header->headerSize = tab2octet (tab4, 4);

	/* width */
	champs(4,tab4,file);
	header->width = tab2octet (tab4, 4);

	/* height */
	champs(4,tab4,file);
	header->height = tab2octet (tab4, 4);

	/* planes */
	champs(2,tab2,file);
	header->planes = tab2octet (tab2, 2);

	/* bitsPerPixels */
	champs(2,tab2,file);
	header->bitsPerPixels = tab2octet (tab2, 2);

	/* compression */
	champs(4,tab4,file);
	header->compression = tab2octet (tab4, 4);

	/* bitmapDataSize */
	champs(4,tab4,file);
	header->bitmapDataSize = tab2octet (tab4, 4);

	/* hResolution */
	champs(4,tab4,file);
	header->hResolution = tab2octet (tab4, 4);

	/* vResolution */
	champs(4,tab4,file);
	header->vResolution = tab2octet (tab4, 4);

	/* colors */
	champs(4,tab4,file);
	header->colors = tab2octet (tab4, 4);

	/* importantColors */
	champs(4,tab4,file);
	header->importantColors = tab2octet (tab4, 4);

	return 1;
}


/***********************************************************
                  isBitmapHeaderCorrect function
***********************************************************/
int isBitmapHeaderCorrect(BMP_HEADER *header)
{
	if (header->identifier[0] != 'B' || header->identifier[1] != 'M')
		return 0;
	else if (header->compression != 0)
		return 0;
	else if (header->bitsPerPixels != 24)
		return 0;
	else if (header->bitmapDataSize != (header->width * header->height * 3))
		return 0;
	else
		return 1;
}


/***********************************************************
                  getBit function
***********************************************************/
// WARNING : VERIFIER QUE LE NOMBRE DE BIT DU MESSAGE SOIENT INFERIEURE AU NOMBRE D'OCTETS COMPOSANT L'IMAGE
int getBit(char *m, int n) // A = 01000001
{
	int octetOffset = n / 8;
	int bitOffset = 7 - (n % 8);
	int bit, octet;

	octet = m[octetOffset];

	bit = octet & (1<<bitOffset);
	bit >>= bitOffset;

	return bit;
}


/***********************************************************
                  loadBitmapDatas function
***********************************************************/
//IDEE D'AMELIORATION : REMETTRE LE CURSEUR DE LECTURE DU FICHIER A 0 ET FAIRE UNE PRE BOUCLE DE DATAOFFSET AVEC FGETC A CHAQUE ITERATIONS
unsigned char *loadBitmapDatas(FILE *file, BMP_HEADER *header)
{
	unsigned char *pixels = NULL;

	if ((pixels = malloc (header->bitmapDataSize * sizeof (*pixels))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

	for (int i=0; i<header->bitmapDataSize; i++)
	{
		pixels[i] = fgetc (file);
	}

	if (fgetc (file) != EOF)
	{
		fprintf (stderr, "stegano: error: loadBitmapDatas failure\n");
		exit (EXIT_FAILURE);
	}

	return pixels;
}


/***********************************************************
                  saveBitmapDatas function
***********************************************************/
// WARNING : TROUVER UN MOYEN D'AMELIORER CETTE FONCTION
int saveBitmapDatas(FILE *file, BMP_HEADER *header, unsigned char *pixels)
{
	char octet = 0;
	int c0 = 0x000000ff;
	int c1 = 0x0000ff00;
	int c2 = 0x00ff0000;
	int c3 = 0xff000000;

	for (int i=0; i<2; i++)
	{
		fputc (header->identifier[i], file);
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->fileSize & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->fileSize & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->fileSize & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->fileSize & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->reserved & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->reserved & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->reserved & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->reserved & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->dataOffset & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->dataOffset & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->dataOffset & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->dataOffset & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->headerSize & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->headerSize & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->headerSize & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->headerSize & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->width & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->width & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->width & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->width & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->height & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->height & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->height & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->height & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<2; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->planes & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->planes & c1)>>8;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<2; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->bitsPerPixels & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->bitsPerPixels & c1)>>8;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->compression & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->compression & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->compression & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->compression & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->bitmapDataSize & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->bitmapDataSize & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->bitmapDataSize & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->bitmapDataSize & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->hResolution & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->hResolution & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->hResolution & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->hResolution & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->vResolution & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->vResolution & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->vResolution & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->vResolution & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->colors & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->colors & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->colors & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->colors & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int j=0; j<4; j++)
	{
		switch (j)
		{
			case 0:
				octet = header->importantColors & c0;
				fputc (octet, file);
				break;
			case 1:
				octet = (header->importantColors & c1)>>8;
				fputc (octet, file);
				break;
			case 2:
				octet = (header->importantColors & c2)>>16;
				fputc (octet, file);
				break;
			case 3:
				octet = (header->importantColors & c3)>>24;
				fputc (octet, file);
				break;
		}
	}

	for (int i=0; i<header->bitmapDataSize; i++)
	{
		octet = pixels[i];
		fputc (octet, file);
	}

	return 1;
}


/***********************************************************
            createPermutationFunction function
***********************************************************/
void createPermutationFunction(unsigned int *tab, int size, unsigned int key)
{
	srand (key);

	// On remplit la 1ere ligne de facon non aléatoire
	// Pb car i est un int. Tester avec double, long long double, etc...
  	for (unsigned int i = 0; i < size; i++)
    {
    	//printf("%d\n", i);
    	tab[i] = i;
    	//printf("tab[%d] = %d\n", i, tab[i]);
    }

  	// On crée l'aléatoire sur la 1ere ligne
  	for (unsigned int i = 0; i < size; i++)
    {
     	int tmp = 0;
      	int rand_x = rand () % (size);
      	int rand_y = rand () % (size);

      	// On échange les contenus des cases i et rand_number
      	tmp = tab[rand_x];
      	tab[rand_x] = tab[rand_y];
      	tab[rand_y] = tmp;
    }

    /*for (int i = 0; i < 80; i++)
    {
    	printf("tab[%d] = %d\n", i, tab[i]);
    }*/
}


/***********************************************************
            	hideText function
***********************************************************/
int hideText(char *message, int *size, unsigned int *tab, unsigned char *pixels)
{
	for (int i=0; i<((*size)*8); i++)
	{
		int octet = tab[i];

		int bit = getBit (message, i);

		if (bit == 1)
		{
			pixels[octet] |= (1 << 0);
		}
		else
		{
			pixels[octet] &= ~(1 << 0);
		}
	}

	for (int i=((*size)*8); i<(((*size)+1)*8); i++)
	{
		int octet = tab[i];

		pixels[octet] &= ~1;
	}

	return 1;
}