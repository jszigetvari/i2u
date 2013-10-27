#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <gd.h>

// This program was written by Janos SZIGETVARI (c) 2012-2013.
// This and other files related to the i2u project may be downloaded from:
// https://github.com/jszigetvari/i2u or by running the command
// git clone https://github.com/jszigetvari/i2u
// The files related to this project may be copied and (re)distibuted in accordance with the GNU GPL v3 license.

#ifdef _GNU_SOURCE
#include <getopt.h> /* getopt_long */
#endif

/*=========== DEFINES ==========*/

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#define EXEC_NAME	"i2u"

#define ALL_OK						0
#define ERR_OUT_OF_MEMORY			255
#define ERR_INVALID_ARG				254
#define ERR_STAT_OTHER				253
#define ERR_CLEANUP					252
#define ERR_OPEN_FAIL				251
#define ERR_WRITE_FAIL				250
#define ERR_FOPEN_FAIL				249
#define ERR_POPEN_FAIL				248
#define ERR_UNSUPPORTED_FORMAT		247
#define ERR_INFILE_ACCESS			246
#define ERR_GD_XPMOPEN_FAIL	245

#define JPEG_STRING					"image/jpeg"
#define PNG_STRING					"image/png"
#define GIF_STRING					"image/gif"
#define BMP_STRING					"image/x-ms-bmp"
#define XPM_STRING					"image/x-xpixmap"

#define MIME_TYPE_COMMAND			"/usr/bin/file --mime-type \'%s\' | /usr/bin/awk '{print $2;}'"
#define MIME_TYPE_COMMAND_FMT_LEN	2			

/*=========== TYPEDEFS ==========*/

typedef enum { false=0, true=1 } bool;
typedef enum { ansi=8, unicode=32 } outfmt_t;
typedef enum { low=8, basic=16, extended=256 } colors_t;
typedef enum { UNKNOWN=0, jpeg=1, png=2, gif=3, bmp=4, xpm=5 } image_type;

typedef gdImagePtr (*gdImageCreateFunction) (void *);

typedef struct conf {
		char *m_infile;
		char *m_outfile;
		colors_t m_colors;
		outfmt_t m_outformat;
		image_type m_ftype;
		int m_targ_width;
		int m_toleranceradius;
		long int m_bgcolor;
		long int m_newcolor;
} conf_t;

typedef conf_t * conf_ptr_t;
typedef conf_t const * conf_cptr_t;

/*=========== GLOBALS ==========*/

conf_t i2u_conf = {
		NULL,
		NULL,
		256,
		unicode,
		UNKNOWN,
		80,
		0,
		-1,
		-1
	};
	
int colors256[256][3] = {
		{ 0, 0, 0 }, { 128, 0, 0 }, { 0, 128, 0 }, { 128, 128, 0 }, { 0, 0, 128 }, { 128, 0, 128 },
		{ 0, 128, 128 }, { 192, 192, 192 }, { 128, 128, 128 }, { 255, 0, 0 }, { 0, 255, 0 }, { 255, 255, 0 },
		{ 0, 0, 255 }, { 255, 0, 255 }, { 0, 255, 255 }, { 255, 255, 255 }, { 0, 0, 0 }, { 0, 0, 95 },
		{ 0, 0, 135 }, { 0, 0, 175 }, { 0, 0, 215 }, { 0, 0, 255 }, { 0, 95, 0 }, { 0, 95, 95 },
		{ 0, 95, 135 }, { 0, 95, 175 }, { 0, 95, 215 }, { 0, 95, 255 }, { 0, 135, 0 }, { 0, 135, 95 }, 
		{ 0, 135, 135 }, { 0, 135, 175 }, { 0, 135, 215 }, { 0, 135, 255 }, { 0, 175, 0 }, { 0, 175, 95 },
		{ 0, 175, 135 }, { 0, 175, 175 }, { 0, 175, 215 }, { 0, 175, 255 }, { 0, 215, 0 }, { 0, 215, 95 },
		{ 0, 215, 135 }, { 0, 215, 175 }, { 0, 215, 215 }, { 0, 215, 255 }, { 0, 255, 0 }, { 0, 255, 95 },
		{ 0, 255, 135 }, { 0, 255, 175 }, { 0, 255, 215 }, { 0, 255, 255 }, { 95, 0, 0 }, { 95, 0, 95 },
		{ 95, 0, 135 }, { 95, 0, 175 }, { 95, 0, 215 }, { 95, 0, 255 }, { 95, 95, 0 }, { 95, 95, 95 },
		{ 95, 95, 135 }, { 95, 95, 175 }, { 95, 95, 215 }, { 95, 95, 255 }, { 95, 135, 0 }, { 95, 135, 95 },
		{ 95, 135, 135 }, { 95, 135, 175 }, { 95, 135, 215 }, { 95, 135, 255 }, { 95, 175, 0 }, { 95, 175, 95 }, 
		{ 95, 175, 135 }, { 95, 175, 175 }, { 95, 175, 215 }, { 95, 175, 255 }, { 95, 215, 0 }, { 95, 215, 95 },
		{ 95, 215, 135 }, { 95, 215, 175 }, { 95, 215, 215 }, { 95, 215, 255 }, { 95, 255, 0 }, { 95, 255, 95 },
		{ 95, 255, 135 }, { 95, 255, 175 }, { 95, 255, 215 }, { 95, 255, 255 }, { 135, 0, 0 }, { 135, 0, 95 },
		{ 135, 0, 135 }, { 135, 0, 175 }, { 135, 0, 215 }, { 135, 0, 255 }, { 135, 95, 0 }, { 135, 95, 95 },
		{ 135, 95, 135 }, { 135, 95, 175 }, { 135, 95, 215 }, { 135, 95, 255 }, { 135, 135, 0 }, { 135, 135, 95 },
		{ 135, 135, 135 }, { 135, 135, 175 }, { 135, 135, 215 }, { 135, 135, 255 }, { 135, 175, 0 }, { 135, 175, 95 },
		{ 135, 175, 135 }, { 135, 175, 175 }, { 135, 175, 215 }, { 135, 175, 255 }, { 135, 215, 0 }, { 135, 215, 95 }, 
		{ 135, 215, 135 }, { 135, 215, 175 }, { 135, 215, 215 }, { 135, 215, 255 }, { 135, 255, 0 }, { 135, 255, 95 },
		{ 135, 255, 135 }, { 135, 255, 175 }, { 135, 255, 215 }, { 135, 255, 255 }, { 175, 0, 0 }, 	{ 175, 0, 95 },
		{ 175, 0, 135 }, { 175, 0, 175 }, { 175, 0, 215 }, { 175, 0, 255 }, { 175, 95, 0 }, { 175, 95, 95 }, 
		{ 175, 95, 135 }, { 175, 95, 175 }, { 175, 95, 215 }, { 175, 95, 255 }, { 175, 135, 0 }, { 175, 135, 95 }, 
		{ 175, 135, 135 }, { 175, 135, 175 }, { 175, 135, 215 }, { 175, 135, 255 }, { 175, 175, 0 }, { 175, 175, 95 }, 
		{ 175, 175, 135 }, { 175, 175, 175 }, { 175, 175, 215 }, { 175, 175, 255 }, { 175, 215, 0 }, { 175, 215, 95 }, 
		{ 175, 215, 135 }, { 175, 215, 175 }, { 175, 215, 215 }, { 175, 215, 255 }, { 175, 255, 0 }, { 175, 255, 95 }, 
		{ 175, 255, 135 }, { 175, 255, 175 }, { 175, 255, 215 }, { 175, 255, 255 }, { 215, 0, 0 }, { 215, 0, 95 }, 
		{ 215, 0, 135 }, { 215, 0, 175 }, { 215, 0, 215 }, { 215, 0, 255 }, { 215, 95, 0 }, { 215, 95, 95 },
		{ 215, 95, 135 }, { 215, 95, 175 }, { 215, 95, 215 }, { 215, 95, 255 }, { 215, 135, 0 }, { 215, 135, 95 },
		{ 215, 135, 135 }, { 215, 135, 175 }, { 215, 135, 215 }, { 215, 135, 255 }, { 215, 175, 0 }, { 215, 175, 95 },
		{ 215, 175, 135 }, { 215, 175, 175 }, { 215, 175, 215 }, { 215, 175, 255 }, { 215, 215, 0 }, { 215, 215, 95 }, 
		{ 215, 215, 135 }, { 215, 215, 175 }, { 215, 215, 215 }, { 215, 215, 255 },	{ 215, 255, 0 }, { 215, 255, 95 }, 
		{ 215, 255, 135 }, { 215, 255, 175 }, { 215, 255, 215 }, { 215, 255, 255 }, { 255, 0, 0 }, { 255, 0, 95 }, 
		{ 255, 0, 135 }, { 255, 0, 175 }, { 255, 0, 215 }, { 255, 0, 255 }, { 255, 95, 0 }, { 255, 95, 95 }, 
		{ 255, 95, 135 }, { 255, 95, 175 }, { 255, 95, 215 }, { 255, 95, 255 }, { 255, 135, 0 }, { 255, 135, 95 }, 
		{ 255, 135, 135 }, { 255, 135, 175 }, { 255, 135, 215 }, { 255, 135, 255 },	{ 255, 175, 0 }, { 255, 175, 95 }, 
		{ 255, 175, 135 }, { 255, 175, 175 }, { 255, 175, 215 }, { 255, 175, 255 },	{ 255, 215, 0 }, { 255, 215, 95 },
		{ 255, 215, 135 }, { 255, 215, 175 }, { 255, 215, 215 }, { 255, 215, 255 }, { 255, 255, 0 }, { 255, 255, 95 },
		{ 255, 255, 135 }, { 255, 255, 175 }, { 255, 255, 215 }, { 255, 255, 255 },	{ 0, 0, 0 }, { 18, 18, 18 },
		{ 28, 28, 28 }, { 38, 38, 38 }, { 48, 48, 48 }, { 58, 58, 58 }, { 68, 68, 68 }, { 78, 78, 78 },
		{ 88, 88, 88 }, { 98, 98, 98 }, { 108, 108, 108 }, { 118, 118, 118 }, { 128, 128, 128 }, { 138, 138, 138 }, 
		{ 148, 148, 148 }, { 158, 158, 158 }, { 168, 168, 168 }, { 178, 178, 178 }, { 188, 188, 188 }, { 198, 198, 198 }, 
		{ 208, 208, 208 }, { 218, 218, 218 }, { 228, 228, 228 }, { 238, 238, 238 } 
	};

int colors16[16][3] = {
		{0, 0, 0}, {128, 0, 0}, {0, 128, 0}, {128, 128, 0}, {0, 0, 128}, {128, 0, 128},
		{0, 128, 128}, {192, 192, 192}, {128, 128, 128}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0},
		{0, 0, 255}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}
	};
	
int colors8[8][3] = {
		{0, 0, 0}, {250, 75, 75}, {24, 178, 24}, {178, 104, 24}, {192, 0, 0}, {225, 30, 225}, {24, 178, 178}, {178, 178, 178}     
  };

/*=========== FUNCTION DECLARATIONS ==========*/

void help(FILE *, char const * const);
bool check_readability(struct stat const *);
void cleanup_main();
image_type get_image_type(char const * const);
double get_color_distance(int, int, int, int, int, int);
int get_closest_color8(int, int, int);
int get_closest_color16(int, int, int);
int get_closest_color256(int, int, int);

/*=========== FUNCTIONS ==========*/

bool check_readability(struct stat const *filestat) {
	uid_t u = getuid();
	gid_t g = getgid();
		
	if (u == filestat->st_uid) {
		if ((filestat->st_mode & S_IRUSR) == S_IRUSR) return true;
	}
	else {
		if (g == filestat->st_gid) {
			if ((filestat->st_mode & S_IRGRP) == S_IRGRP) return true;
		}
		else {
			if ((filestat->st_mode & S_IROTH) == S_IROTH) return true;
		}
	}
	
	return false;
}

image_type get_image_type(const char * const file_name) {
	image_type retval = UNKNOWN;
	FILE *p_buf = NULL;
	char *s_buf = NULL;
	char *fgets_retval;
	size_t s_len = 0;
	
	/* STAT the input file to see if it's there... */
	struct stat stat_buf;
		
	if (stat(file_name, &stat_buf) == -1) {
		perror("stat(): ");
		cleanup_main();
		exit(ERR_STAT_OTHER);
	}
	else {
		if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "\'%s\' is not a regular file!\n", file_name);
			cleanup_main();
			exit(ERR_INFILE_ACCESS);
		}
				
		if (check_readability(&stat_buf) != true) {
			fprintf(stderr, "\'%s\' is not a readable for the user/group on whose behalf %s is running on!\n", file_name, EXEC_NAME);
			cleanup_main();
			exit(ERR_INFILE_ACCESS);
		}
	}
	
	
	/* run command */
	s_len = (strlen(MIME_TYPE_COMMAND)+strlen(file_name)-1);
	
	if ((s_buf = (char *) calloc(s_len, sizeof(char))) == NULL) {
		fprintf(stderr, "calloc() failed!\n");
		cleanup_main();
		exit(ERR_OUT_OF_MEMORY);
	}
		
	snprintf(s_buf, s_len, MIME_TYPE_COMMAND, file_name);
	
	if ((p_buf = popen(s_buf, "r")) == NULL) {
		perror("popen(): ");
		cleanup_main();
		exit(ERR_POPEN_FAIL);
	}
	
	free(s_buf);
	
	/* read output */
	if ((s_buf = (char *) calloc(257, sizeof(char))) == NULL) {
		fprintf(stderr, "calloc() failed!\n");
		cleanup_main();
		exit(ERR_OUT_OF_MEMORY);
	}
	
	fgets(s_buf+128, 128, p_buf);
	do {
		memcpy(s_buf, s_buf+128, 128);
		memset(s_buf+128, '\0', 128);
		fgets_retval=fgets(s_buf+128, 128, p_buf);
				
		if (strstr(s_buf, JPEG_STRING) != NULL) retval = jpeg;
		if (strstr(s_buf, PNG_STRING) != NULL) retval = png;
		if (strstr(s_buf, GIF_STRING) != NULL) retval = gif;
		if (strstr(s_buf, BMP_STRING) != NULL) retval = bmp;
		if (strstr(s_buf, XPM_STRING) != NULL) retval = xpm;
	} while ((fgets_retval != NULL) && (retval == UNKNOWN));
	
	free(s_buf);
	pclose(p_buf);
	
	return retval;

}

double get_color_distance(int r_1, int r_2, int g_1, int g_2, int b_1, int b_2) {
		return (sqrt(pow((double) (r_1-r_2), 2.)+pow((double) (g_1-g_2), 2.)+pow((double) (b_1-b_2), 2.)));
}

int get_closest_color8(int r, int g, int b) {
	double dist, min_dist = 445.0;/* max distance in a 256^3 cube is 443.405 */
	int min=-1, i;
	
	for (i=0; i<8; i++) {
		dist = get_color_distance(colors8[i][0], r, colors8[i][1], g, colors8[i][2], b);
		if (dist < min_dist) {
			min_dist = dist;
			min = i;
		}
	}
	return min;
}

int get_closest_color16(int r, int g, int b) {
	double dist, min_dist = 445.0;/* max distance in a 256^3 cube is 443.405 */
	int min=-1, i;
	
	for (i=0; i<16; i++) {
		dist = get_color_distance(colors16[i][0], r, colors16[i][1], g, colors16[i][2], b);
		if (dist < min_dist) {
			min_dist = dist;
			min = i;
		}
	}
	return min;
}

int get_closest_color256(int r, int g, int b) {
	double dist, min_dist = 445.0;/* max distance in a 256^3 cube is 443.405 */
	int min=-1, i;
	
	for (i=0; i<256; i++) {
		dist = get_color_distance(colors256[i][0], r, colors256[i][1], g, colors256[i][2], b);
		if (dist < min_dist) {
			min_dist = dist;
			min = i;
		}
	
	}
	return min;
}

void help(FILE *fd, char const * const en) {
    fprintf( fd,
	     "\n"
#ifdef _GNU_SOURCE
	     "Usage: %s [-i|--infile <file>] [-o|--outfile <file>] [-f|--format <8|32>] [-c|--color <8|16|256>] [-h|--help]>\n"
	     "Options:\t-i|--infile <file>:\t\tinput file\n"
		 "\t\t-o|--outfile <file>:\t\toutput file (stdout: -)\n"
		 "\t\t-c|--colors <8|16|256>:\t\tcolor depth (8, 16 or 256)\n"
		 "\t\t-f|--format <8|32>:\t\tbits per character (8 for ANSI or 32 for Unicode)\n"
		 "\t\t-w|--width <N>:\t\t\ttarget line width (the default is 80)\n"
		 "\t\t-t|--color-tolerance <N>:\ttolerance radius for color substitution\n"
		 "\t\t-b|--bgcolor <N>:\t\tpresent background color (as HEX-triplet)\n"
		 "\t\t-n|--newcolor <N>:\t\tcolor to use as background (as HEX-triplet)\n"
		 "\t\t-h|--help:\t\t\tshows you this screen\n"
#else /* not _GNU_SOURCE */ 
	     "Usage: %s [-i <file>] [-o <file>] [-f <8|32>] [-c <8|16|256>] [-h]>\n"
	     "Options:\t-i <file>:\t\tinput file\n"
		 "\t\t-o <file>:\t\toutput file (stdout: -)\n"
		 "\t\t-c <16|256>:\t\tcolor depth\n"
		 "\t\t-f <8|32>:\t\tbits per character (8 for ANSI or 32 for Unicode)\n"
		 "\t\t-w <N>:\t\t\ttarget line width (the default is 80)\n"
		 "\t\t-t <N>:\t\t\ttolerance radius for color substitution\n"
		 "\t\t-b <N>:\t\t\tpresent background color (as HEX-triplet)\n"
		 "\t\t-n <N>:\t\t\tcolor to use as background (as HEX-triplet)\n"
		 "\t\t-h:\t\t\tshows you this screen\n"
#endif /* _GNU_SOURCE */
	     "\n", EXEC_NAME);
}

void cleanup_main() {
	if (i2u_conf.m_infile != NULL) free(i2u_conf.m_infile);
	if (i2u_conf.m_outfile != NULL) free(i2u_conf.m_outfile);
}

/*=========== MAIN ==========*/

int main(int argc, char **argv, char **env) {
/*=========== GENERIC VARIABLES ===========*/
	FILE *f_output = NULL, *f_input = NULL;
	int c, i;
	char *end;
	
/*=========== GETOPT VARIABLES ===========*/
	char const *const optstr = "hi:o:f:c:w:b:n:t:";
#ifdef _GNU_SOURCE
	static struct option long_opts[] = {
		{"help", 0, NULL, 'h'},
		{"infile", 1, NULL, 'i'},
		{"outfile", 1, NULL, 'o'},
		{"format", 1, NULL, 'f'},
		{"colors", 1, NULL, 'c'},
		{"width", 1, NULL, 'w'},
		{"bgcolor", 1, NULL, 'b'},
		{"newcolor", 1, NULL, 'n'},
		{"color-tolerance", 1, NULL, 't'},
		{NULL, 0, NULL, 0},
	};
#endif

/*=========== IMAGE-RELATED VARIABLES ===========*/
	int x, y;
	int color_idx = 0;
	int img_orig_h = 0, img_orig_w = 0;
	int img_targ_h = 0, img_targ_w = 0;
	int top = -1, bottom = -1, top_prev = -1, bottom_prev = -1;
	int color = -1, color_prev = -1;
	gdImagePtr image_data = NULL;
	gdImagePtr image_resized = NULL;
	gdImageCreateFunction image_openers[] = {
		NULL,
		(gdImageCreateFunction) gdImageCreateFromJpeg,
		(gdImageCreateFunction) gdImageCreateFromPng,
		(gdImageCreateFunction) gdImageCreateFromGif,
		(gdImageCreateFunction) gdImageCreateFromWBMP,
		(gdImageCreateFunction) gdImageCreateFromXpm	/* Accepts char * as first argument! */
	};

	
	while (true) {
#ifdef _GNU_SOURCE
		c = getopt_long(argc, argv, optstr, long_opts, NULL);
#else /* not _GNU_SOURCE */ 
		c = getopt(argc, argv, optstr);
#endif /* _GNU_SOURCE */
		if (c == -1) break;

		switch (c) {
			case 'h':
				help(stdout, EXEC_NAME);
				exit(ALL_OK);
				break;
			case 'i':
				i = strlen(optarg);
				if (i2u_conf.m_infile != NULL) free(i2u_conf.m_infile);
				if ((i2u_conf.m_infile = (char *) calloc(i+1, sizeof(char))) == NULL) {
					fprintf(stderr, "calloc() failed!\n");
					cleanup_main();
					exit(ERR_OUT_OF_MEMORY);
				}
				strncpy(i2u_conf.m_infile, optarg, i);
				break;
			case 'o':
				if (strncmp("-", optarg, 2) == 0) {
					if (i2u_conf.m_outfile != NULL) free(i2u_conf.m_outfile);
					i2u_conf.m_outfile = NULL;
					break;
				}
				else {
					i = strlen(optarg);
					if (i2u_conf.m_outfile != NULL) free(i2u_conf.m_outfile);
					if ((i2u_conf.m_outfile = (char *) calloc(i+1, sizeof(char))) == NULL) {
						fprintf(stderr, "calloc() failed!\n");
						cleanup_main();
						exit(ERR_OUT_OF_MEMORY);
					}
					strncpy(i2u_conf.m_outfile, optarg, i);
					break;
				}
			case 'f':
				i = (int) strtol(optarg, &end, 10);
				if (optarg != end) {
					i2u_conf.m_outformat = (i > 8) ? unicode : ansi;
				}
				else {	
					if (strncasecmp(optarg, "ansi", 5) == 0) i2u_conf.m_outformat = ansi;
					else {
						if (strncmp(optarg, "unicode", 8) == 0) i2u_conf.m_outformat = unicode;
						else fprintf(stderr, "Warning: invalid format specified. Defaulting to Unicode.\n"); 
					}
				}
				break;
			case 'c':
				i = (int) strtol(optarg, &end, 10);
				if (optarg != end) { 
					if (i < 9) i2u_conf.m_colors = low;
					else {
						if (i < 17) i2u_conf.m_colors = basic;
						else i2u_conf.m_colors = extended;
					}
				}
				else fprintf(stderr, "Warning: invalid color depth specified. Defaulting to 256.\n");
				break;
			case 'w':
				i = (int) strtol(optarg, &end, 10);
				if (optarg != end) i2u_conf.m_targ_width = i;
				else fprintf(stderr, "Warning: invalid width specified. Defaulting to 80.\n");
				break;
			case 't':
				i = (int) strtol(optarg, &end, 10);
				if (optarg != end) i2u_conf.m_toleranceradius = (i > 0) ? i : 0;
				else fprintf(stderr, "Warning: invalid color-tolerance radius specified. Defaulting to zero.\n");
				break;
			case 'b':
				i = strtol(optarg, &end, 16);
				if (optarg != end) i2u_conf.m_bgcolor = i;
				else fprintf(stderr, "Warning: invalid background color specified. No background-substitution will occur.\n");
				break;
			case 'n':
				i = strtol(optarg, &end, 16);
				if (optarg != end) i2u_conf.m_newcolor = i;
				else fprintf(stderr, "Warning: invalid substitution color specified. No background-substitution will occur.\n");
				break;
			case '?':
			default:
				help(stderr, EXEC_NAME);
				exit(ERR_INVALID_ARG);
		}
		if (c == 'x') break;
	}

		
		
	if (i2u_conf.m_infile == NULL) {
		fprintf(stderr, "Error: input file not set. Use option" 
#ifdef _GNU_SOURCE
			"s -i or --infile"
#else /* not _GNU_SOURCE */ 
			" -i"
#endif /* _GNU_SOURCE */
			" to specify one.\n");
		cleanup_main();
		exit(ERR_INVALID_ARG);
	}

/*=========== GD PART ==========*/
	
	i2u_conf.m_ftype = get_image_type(i2u_conf.m_infile);
	
	if (i2u_conf.m_ftype == UNKNOWN) {
		fprintf(stderr, "Error: unsupported input file type!\n" );
		cleanup_main();
		exit(ERR_UNSUPPORTED_FORMAT);
	}
	
	if (i2u_conf.m_ftype != xpm) {
			if ((f_input = fopen(i2u_conf.m_infile, "r")) == NULL) {
				perror("fopen(): ");
				cleanup_main();
				exit(ERR_FOPEN_FAIL);
			}
		
			image_data = image_openers[i2u_conf.m_ftype](f_input);
			
			fclose(f_input);
	}
	else {
			if ((image_data = image_openers[i2u_conf.m_ftype](i2u_conf.m_infile)) == NULL) {
				fprintf(stderr, "gdImageCreateFromXpm(): Error occured while opening file \'%s\'\n", i2u_conf.m_infile);
				cleanup_main();
				exit(ERR_GD_XPMOPEN_FAIL);
			}
	}
	
	img_orig_w = image_data->sx;
	img_orig_h = image_data->sy;
	img_targ_w = i2u_conf.m_targ_width;
	img_targ_h = (int) round(((double) img_targ_w)/((double) img_orig_w)*((double) img_orig_h));
	/* if (i2u_conf.m_outformat == ansi) img_targ_w /= 2; */
	
	/* change background, if requested */
	if ((i2u_conf.m_bgcolor > -1) && (i2u_conf.m_newcolor > -1)) {
			int bg_r, bg_g, bg_b;
			int nc_r, nc_g, nc_b;
			int bgcolor, newcolor;
			int radius = i2u_conf.m_toleranceradius;
			bg_r = (i2u_conf.m_bgcolor & 0x00ff0000) >> 16;
			bg_g = (i2u_conf.m_bgcolor & 0x0000ff00) >> 8;
			bg_b = (i2u_conf.m_bgcolor & 0x000000ff);
			nc_r = (i2u_conf.m_newcolor & 0x00ff0000) >> 16;
			nc_g = (i2u_conf.m_newcolor & 0x0000ff00) >> 8;
			nc_b = (i2u_conf.m_newcolor & 0x000000ff);
			
			if ((bgcolor = gdImageColorExact(image_data, bg_r, bg_g, bg_b)) != -1) {
				color_idx = gdImageGetPixel(image_data, 0, 0);
				newcolor = gdImageColorResolve(image_data, nc_r, nc_g, nc_b);
				for (y=0; y<img_orig_h; y++) {
					for (x=0; x<img_orig_w; x++) {
					  color_idx = gdImageGetPixel(image_data, x, y);
				    if (get_color_distance(gdImageRed(image_data, color_idx), gdImageRed(image_data, bgcolor), gdImageGreen(image_data, color_idx), gdImageGreen(image_data, bgcolor), gdImageBlue(image_data, color_idx), gdImageBlue(image_data, bgcolor)) <= radius) gdImageSetPixel(image_data, x, y, newcolor);
					}
				}
			}	
	}
		
	/* resize pic to fit console or the width specified */
	image_resized = gdImageCreateTrueColor(img_targ_w, img_targ_h);
	gdImageCopyResized(image_resized, image_data, 0, 0, 0, 0, img_targ_w, img_targ_h, img_orig_w, img_orig_h);
	gdImageDestroy(image_data);
	
	if (i2u_conf.m_outfile != NULL) {
		if ((f_output = fopen(i2u_conf.m_outfile, "w")) == NULL) {
			perror("fopen(): ");
			cleanup_main();
			exit(ERR_FOPEN_FAIL);
		}
	}
	else f_output = stdout;

	switch (i2u_conf.m_outformat) {
		case ansi: ;
			for (y=0; y<img_targ_h; y++) {
				for (x=0; x<img_targ_w; x++) {
					color_idx = gdImageGetPixel(image_resized, x, y);
					switch (i2u_conf.m_colors) {
						case low:
							color = get_closest_color8(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
							break;
						case basic:
							color = get_closest_color16(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
							break;
						case extended:
							color = get_closest_color256(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
							break;
					}
					if ((color != color_prev)) {
						switch (i2u_conf.m_colors) {
							case low:
								fprintf(f_output, "\x1b[01;30;%dm  ", color+40);
								break;
							case basic:
							case extended:
								fprintf(f_output, "\x1b[38;05;%dm\x1b[48;05;%dm  ", 0, color);
								break;
						}
						color_prev = color;
					}
					else {	
						fprintf(f_output, "  ");
					}
				}
				fprintf(f_output, "\x1b[0m\n");
				color_prev = -1;
			}
			break;
		case unicode: ; 
			for (y=0; y<img_targ_h; y+=2) {
				for (x=0; x<img_targ_w; x++) {
					color_idx = gdImageGetPixel(image_resized, x, y);
					switch (i2u_conf.m_colors) {
						case low:
							top = get_closest_color8(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
							break;
						case basic:
							top = get_closest_color16(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
							break;
						case extended:
							top = get_closest_color256(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
							break;
					}
					if (y+1 < img_targ_h) {
						color_idx = gdImageGetPixel(image_resized, x, y+1);
						switch (i2u_conf.m_colors) {
							case low:
								bottom = get_closest_color8(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
								break;
							case basic:
								bottom = get_closest_color16(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
								break;
							case extended:
								bottom = get_closest_color256(gdImageRed(image_resized, color_idx), gdImageGreen(image_resized, color_idx), gdImageBlue(image_resized, color_idx));
								break;
						}
					}
					else {
						bottom = 0;
					}
					if ((top != top_prev) || (bottom != bottom_prev)) {
						switch (i2u_conf.m_colors) {
							case low:
								fprintf(f_output, "\x1b[01;%d;%dm▄", bottom+40, top+30);
								break;
							case basic:
							case extended:
								fprintf(f_output, "\x1b[38;05;%dm\x1b[48;05;%dm▄", bottom, top);
								break;
						}
						bottom_prev = bottom;
						top_prev = top;
					}
					else {	
						fprintf(f_output, "▄");
					}
				}
				fprintf(f_output, "\x1b[0m\n");
				top_prev = bottom_prev = -1;
			}
			break;
	}
		
	gdImageDestroy(image_resized);
	fclose(f_output);
	
	return ALL_OK;
	
}
