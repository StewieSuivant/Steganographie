#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>
#include <getopt.h>

#include "stegano.h"

/* IDEES D'AMELIORATION
1) -g avec argument optionnel. Si argument, c'est le fichier dest, sinon, prendre fichier source.bmp et renvoyer source_ok.bmp.
2) Proposer une option pour écrire le message à cacher dans l'appel du programme.
3) Proposer via une option d'envoyer un fichier à cacher dans l'image.
4) free (message) impossible pour l'option -g, donc memoire non désalloué avant la fin du programme.
5) Tester le message entré pour vérifier qu'il ne contient pas de caractères interdit et vérifier qu'il n'est pas supérieur à bitmapDataSize
6) Docummenter beaucoup plus le code
*/

static void usage (int status);
static void version (void);
static void hideMessage(FILE *imgSrc, FILE *imgDest, char *message, unsigned int key);
static void retrieveMessage(FILE *imgSrc, unsigned int key);

bool generate = false;
bool retrieve = false;
unsigned int size = 0;
unsigned int permutationSize = 0;


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
				if ((message = malloc ((size + 1) * sizeof (*message))) == NULL)
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
      	fprintf (stdout, "Usage: ./%s [OPTION] FILE\n"
      					 "Usage: ./%s -g SOURCE DESTINATION CLE \"MESSAGE\"\n"
      					 "Usage: ./%s -r SOURCE CLE\n"
				         "Introduit un message caché dans une image au format BMP.\n"
				         "   -V\t--version\taffiche la version et quitte\n"
				         "   -h\t--help\t\taffiche l'aide\n", PROG_NAME, PROG_NAME, PROG_NAME);
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
  	BMP_HEADER *header = NULL;
  	unsigned int *permutation = NULL;
  	unsigned char *pixels = NULL;

	/* Initialisation of header */
	if ((header = malloc(sizeof (*header))) == NULL)
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

	permutationSize = header->bitmapDataSize;

    /* Initialisation of permutation */
	if ((permutation = malloc (permutationSize * sizeof (*permutation))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

	//print_header (header);

	pixels = loadBitmapDatas (imgSrc, header);

	createPermutationFunction (permutation, permutationSize, key);

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
  	unsigned int *permutation = NULL;
  	unsigned char *pixels = NULL;
  	char *message = NULL;

  	//int nbZero = 0, i = 0, bitOffset = 0;

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

	permutationSize = header->bitmapDataSize;

    /* Initialisation of permutation */ 
	if ((permutation = malloc (permutationSize * sizeof (*permutation))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

    /* Initialisation of message */
    // Nombre d'octets modifiable / 8 = Nombre d'octets max du message
	if ((message = malloc ((permutationSize / 8) * sizeof (*message))) == NULL)
    {
      	fprintf (stderr, "stegano: error: out of memory !");
      	exit (EXIT_FAILURE);
    }

	//print_header (header);

	pixels = loadBitmapDatas (imgSrc, header);

	createPermutationFunction (permutation, permutationSize, key);

	if (retrieveText(message, permutation, pixels) == 0)
	{
		fprintf (stderr, "stegano: error: hidetext failure\n");
		exit (EXIT_FAILURE);
	}

	fprintf(stdout, "message = '%s'\n", message);

	free (header);
	free (permutation);
	free (pixels);
	free (message);
}