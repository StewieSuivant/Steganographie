#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>
#include <getopt.h>

#include "stegano.h"

/* IDEES D'AMELIORATION
0) Limité à 499 caractères pour le moment pour le -r (due à la taille du tableau de permutation).
1) -g avec argument optionnel. Si argument, c'est le fichier dest, sinon, prendre fichier source.bmp et renvoyer source_ok.bmp.
2) Proposer une option pour écrire le message à cacher dans l'appel du programme.
3) Proposer via une option d'envoyer un fichier à cacher dans l'image.
*/

static void usage (int status);
static void version (void);
static void hideMessage(FILE *imgSrc, FILE *imgDest, char *message, unsigned int key);
static void retrieveMessage(FILE *imgSrc, unsigned int key);

bool generate = false;
bool retrieve = false;
int size = 0;


int main (int argc, char *argv[])
{
	FILE *imgSrc = NULL, *imgDest = NULL;
	char *message = NULL;
	int key;
	int optc;
	struct option long_opts[] = {
	    {"version", no_argument,       NULL, 'V'},
	    {"help",    no_argument,       NULL, 'h'},
	    {"generate",no_argument,       NULL, 'g'},
	    {"retrieve",no_argument,       NULL, 'r'},
	    {NULL,      0,                 NULL, 0}
  	};

  	// Parser of option accepting -V, -h, -g and -r
  	while ((optc = getopt_long (argc, argv, "Vhgr", long_opts, NULL)) != -1)
  	{
  		switch (optc)
      	{
      		case 'V':
	      		version ();
	      		break;

      		case 'h':
	      		usage (EXIT_SUCCESS);
	      		break;

	      	case 'g':
	      		if (retrieve)
	      		{
	      			fprintf (stderr, "error: incompatible arguments\n");
					usage (EXIT_FAILURE);
	      		}

				if (argc <= optind+3)
				{
					fprintf (stderr, "error: missing argument\n");
					usage (EXIT_FAILURE);
				}
				else if (argc > optind+4)
				{
					fprintf (stderr, "error: few argument\n");
					usage (EXIT_FAILURE);
				}

				if ((imgDest = fopen (argv[optind+1], "w")) == NULL)
				{
					if (errno == ENOENT)
					{
						fprintf (stderr, "error: file doesn't exist\n");
						usage (EXIT_FAILURE);
					}
					else
					{
						fprintf (stderr, "error: unknow error about inputfile\n");
						usage (EXIT_FAILURE);
					}
				}

				key = atoi (argv[optind+2]);

				size = strlen (argv[optind+3]);
				if ((message = malloc (4000 * sizeof (*message))) == NULL)
			    {
			      	fprintf (stderr, "stegano: error: out of memory !");
			      	exit (EXIT_FAILURE);
			    }
			    message = argv[optind+3];

	      		generate = true;
	      		break;

      		case 'r': // WARNING : PB SUR LE CONTROLE DE L'ECCES D'ARGUMENT
      			if (generate)
	   			{
	      			fprintf (stderr, "error: incompatible arguments\n");
					usage (EXIT_FAILURE);
	      		}

	      		if (argc <= optind+1)
				{
					fprintf (stderr, "error: missing argument\n");
					usage (EXIT_FAILURE);
				}

	      		if (argc > optind+2)
				{
					fprintf (stderr, "error: few argument\n");
					usage (EXIT_FAILURE);
				}

				key = atoi (argv[optind+1]);

	      		retrieve = true;
	      		break;

      		default:
	      		usage (EXIT_FAILURE);
      	}
  	}

	/* Si un nom de fichier n'est pas trouvé => EXIT_FAILURE */
    if (argc <= optind)
	{
		fprintf (stderr, "error: missing argument\n");
		usage (EXIT_FAILURE);
	}

    if ((imgSrc = fopen (argv[optind], "r")) == NULL)
	{
		if (errno == ENOENT)
		{
			fprintf (stderr, "error: file doesn't exist\n");
			usage (EXIT_FAILURE);
		}
		else
		{
			fprintf (stderr, "error: unknow error about inputfile\n");
			usage (EXIT_FAILURE);
		}
	}

	if (generate)
	{
		//char *message = "A"; // 01000001

		//size = strlen (message);
		hideMessage(imgSrc, imgDest, message, key);
	}
	
	if (retrieve)
	{
		retrieveMessage(imgSrc, key);
	}

	fclose (imgSrc);

	return EXIT_SUCCESS;
}


/***********************************************************
                Usage function
***********************************************************/
static void usage (int status)
{
  	if (status == EXIT_SUCCESS)
    {
      	fprintf (stdout, "Usage: ./stegano [OPTION] FILE\n"
      					 "Usage: ./stegano -g SOURCE DESTINATION CLE\n"
      					 "Usage: ./stegano -r SOURCE CLE\n"
				         "Introduit un message caché dans une image au format BMP.\n"
				         "   -V\t--version\taffiche la version et quitte\n"
				         "   -h\t--help\t\taffiche l'aide\n");
    }
  	else
    {
      	fprintf (stderr, "Try 'stegano --help' for more information.\n");
    }

  	exit (status);
}


/***********************************************************
                Version function
***********************************************************/
static void version (void)
{
  fprintf (stdout, "stegano %d.%d.%d\n",
	         PROG_VERSION, PROG_SUBVERSION, PROG_REVISION);
  fprintf (stdout, "Ce logiciel introduit un message dans une image BMP.\n");

  exit (EXIT_SUCCESS);
}


/***********************************************************
            	hideMessage function
***********************************************************/
static void hideMessage(FILE *imgSrc, FILE *imgDest, char *message, unsigned int key)
{
	//FILE *imgDest = NULL;
  	BMP_HEADER *header = NULL;
  	int *permutation = NULL;
  	unsigned char *pixels = NULL;

	/* Initialisation of header */
	if ((header = malloc(sizeof (*header))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

    /* Initialisation of permutation */
	if ((permutation = malloc (4000 * sizeof (*permutation))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }


	if (loadBitmapHeader(imgSrc, header) == 0)
	{
		fprintf (stderr, "stegano: error: loadBitmapHeader failure\n");
		exit (EXIT_FAILURE);
	}

	if (isBitmapHeaderCorrect (header) == 0)
	{
		fprintf (stderr, "stegano: error: isBitmapHeaderCorrect failure\n");
		exit (EXIT_FAILURE);
	}

	//print_header (header);

	pixels = loadBitmapDatas (imgSrc, header);

	createPermutationFunction (permutation, 4000, key);

	if (hideText(message, &size, permutation, pixels) == 0)
	{
		fprintf (stderr, "stegano: error: hidetext failure\n");
		exit (EXIT_FAILURE);
	}

	if (saveBitmapDatas(imgDest, header, pixels) == 0)
	{
		fprintf (stderr, "stegano: error: isBitmapHeaderCorrect failure\n");
		exit (EXIT_FAILURE);
	}

	free (header);
	free (permutation);
	free (pixels);
	fclose (imgDest);
}


/***********************************************************
            	retrieveMessage function
***********************************************************/
// WARNING : VOIR A PEUT ETRE CHANGER LE TYPE DE RETOUR EN CAHR* ET PARTITIONNER EN PLUSIEURS FONCTION
static void retrieveMessage(FILE *imgSrc, unsigned int key)
{
	BMP_HEADER *header = NULL;
  	int *permutation = NULL;
  	unsigned char *pixels = NULL;
  	char *message = NULL;

  	int nbZero = 0, i = 0, bitOffset = 0;

  	/* Initialisation of header */
	if ((header = malloc (sizeof (*header))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

	if (loadBitmapHeader (imgSrc, header) == 0)
	{
		fprintf (stderr, "stegano: error: loadBitmapHeader failure\n");
		exit (EXIT_FAILURE);
	}

	if (isBitmapHeaderCorrect (header) == 0)
	{
		fprintf (stderr, "stegano: error: isBitmapHeaderCorrect failure\n");
		exit (EXIT_FAILURE);
	}

    /* Initialisation of permutation */ 
	if ((permutation = malloc (header->bitmapDataSize * sizeof (*permutation))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

    /* Initialisation of message */
    // Nombre d'octets modifiable / 8 = Nombre d'octets max du message
	if ((message = malloc ((header->bitmapDataSize/8) * sizeof (*message))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

	//print_header (header);

	pixels = loadBitmapDatas (imgSrc, header);

	createPermutationFunction (permutation, 4000, key);

	while (nbZero < 8)
	{
		int octet = permutation[i];
		int bit = pixels[octet] & 1;

		if (bit == 0)
		{
			nbZero++;
		}
		else
		{
			nbZero = 0;
		}

		i++;

		if (bit == 0)
		{
			message[bitOffset] &= ~1;
		}
		else
		{
			message[bitOffset] |= 1;
		}

		if ((i % 8) == 0)
		{
			bitOffset++;
		}
		else
		{
			message[bitOffset] <<= 1;
		}
	}

	fprintf(stdout, "message = '%s'\n", message);

	free (header);
	free (permutation);
	free (pixels);
	free (message);
}