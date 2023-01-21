/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2022-07-26  Jason Bacon Begin
 ***************************************************************************/

#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define MAX_TAG_LEN 1024

void    usage(char *argv[]);
int     strip_file(const char *file);
char    *read_tag(FILE *infile);

int     main(int argc,char *argv[])

{
    switch(argc)
    {
	case 2:
	    return strip_file(argv[1]);
	    break;
	
	default:
	    usage(argv);
    }
    
    return EX_OK;
}


/***************************************************************************
 *  Use auto-c2man to generate a man page from this comment
 *
 *  Library:
 *      #include <>
 *      -l
 *
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  Examples:
 *
 *  Files:
 *
 *  Environment
 *
 *  See also:
 *
 *  History: 
 *  Date        Name        Modification
 *  2023-01-21  Jason Bacon Begin
 ***************************************************************************/

int     strip_file(const char *file)

{
    FILE    *infile;
    char    *tag;
    int     section_level, title_count, ch;
    
    if ( (infile = fopen(file, "r")) == NULL )
    {
	fprintf(stderr, "Cannot open %s: %s\n", file, strerror(errno));
	exit(EX_UNAVAILABLE);
    }
    section_level = 1;
    title_count = 0;
    if ( (tag = read_tag(infile)) != NULL )
    {
	if ( (memcmp(tag, "<chapter", 8) == 0) ||
	     (memcmp(tag, "<section", 8) == 0) )
	{
	    puts(tag);
	    while ( (tag = read_tag(infile)) != NULL )
	    {
		if ( memcmp(tag, "<section", 8) == 0 )
		{
		    if ( section_level++ == 1 )
		    {
			printf("\n    %s\n", tag);
			title_count = 1;
		    }
		}
		else if ( memcmp(tag, "</section", 9) == 0 )
		{
		    if ( --section_level == 1 )
			printf("    %s\n", tag);
		    else if ( section_level == 0 )
			puts(tag);
		}
		else if ( memcmp(tag, "<title", 6) == 0 )
		{
		    if ( ++title_count <= 2 )
		    {
			printf("%*s%s", title_count * 4, " ", "<title>");
			while ( (ch = getc(infile)) != '<' )
			    putchar(ch);
			ungetc(ch, infile);
			tag = read_tag(infile);
			if ( memcmp(tag, "</title", 7) == 0 )
			    puts(tag);
			else
			{
			    fprintf(stderr, "Expected </title>.\n");
			    return EX_DATAERR;
			}
		    }
		}
		else if ( memcmp(tag, "<xi:include", 11) == 0 )
		{
		    if ( strstr(tag, "HW/") != NULL )
		    {
			printf("\n        %s\n", tag);
		    }
		    else
		    {
			/*
			 *  FIXME: Only supports files in CWD
			 *  Add a DBK_PATH feature
			 */
			
			struct stat st;
			char *filename = strstr(tag, "href=") + 6;
			*strchr(filename, '"') = '\0';
			if ( stat(filename, &st) == 0 )
			    strip_file(filename);
		    }
		}
		else if ( memcmp(tag, "</chapter", 9) == 0 )
		{
		    puts(tag);
		}
	    }
	}
	else
	{
	    fprintf(stderr, "First tag should be <chapter> or <section>\n");
	    return EX_DATAERR;
	}
    }
    fclose(infile);
    return EX_OK;
}

char    *read_tag(FILE *infile)

{
    int     ch;
    static char    tag[MAX_TAG_LEN] = "<";
    size_t  c;
    
    while ( ((ch = getc(infile)) != '<') && (ch != EOF) )
	;
    if ( ch == '<' )
    {
	c = 1;
	while ( (c < MAX_TAG_LEN) && ((tag[c++] = getc(infile)) != '>') )
	    ;
	tag[c] = '\0';
	return tag;
    }
    return NULL;
}


void    usage(char *argv[])

{
    fprintf(stderr, "Usage: %s\n", argv[0]);
    exit(EX_USAGE);
}
