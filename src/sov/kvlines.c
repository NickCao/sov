#ifndef kvlines_h
#define kvlines_h

#include "mt_map.c"

int kvlines_read(char* libpath, mt_map_t* db);
int kvlines_write(char* libpath, mt_map_t* db);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "mt_string.c"
#include "mt_string_ext.c"
#include <limits.h>
#ifdef __linux__
    #include <linux/limits.h>
#endif
#include <stdio.h>

int kvlines_read(char* libpath, mt_map_t* db)
{
    int   retv  = -1;
    char* dbstr = mt_string_new_file(libpath); // REL 0

    if (dbstr)
    {
	retv = 0;

	char* key = NULL;
	char* val = NULL;

	int in_word = 0;
	int in_comm = 0;
	int newline = 1;

	int word_index = 0;
	int store_word = 0;

	for (int index = 0; index <= strlen(dbstr); index++)
	{
	    if (dbstr[index] == '\n' || dbstr[index] == '\0')
	    {
		// newline or end of file, close word, set new line
		if (in_word == 1) store_word = 1;
		in_word = 0;
		in_comm = 0;
		newline = 1;
	    }
	    else if (dbstr[index] == ' ' || dbstr[index] == '\t')
	    {
		if (key == NULL)
		{
		    // space, close word
		    if (in_word == 1) store_word = 1;
		    in_word = 0;
		}
	    }
	    else if (dbstr[index] == '#' && newline == 1)
	    {
		// comment after newline, toggle comment mode
		in_comm = 1;
		newline = 0;
	    }
	    else if (in_comm == 0 && in_word == 0)
	    {
		// if not in comment and not in word, set word mode, store index
		in_word    = 1;
		newline    = 0;
		word_index = index;
	    }

	    if (store_word == 1)
	    {
		store_word = 0;
		if (key == NULL)
		{
		    key = mt_string_new_cstring("");
		    key = mt_string_append_sub(key, dbstr, word_index, index - word_index);
		}
		else if (val == NULL)
		{
		    val = mt_string_new_cstring("");
		    val = mt_string_append_sub(val, dbstr, word_index, index - word_index);

		    MPUT(db, key, val);

		    REL(key);
		    REL(val);

		    key = NULL;
		    val = NULL;
		}
	    }
	}

	REL(dbstr);
    }

    return retv;
}

int kvlines_write(char* libpath, mt_map_t* db)
{
    int   retv = -1;
    char* path = mt_string_new_format(PATH_MAX + NAME_MAX, "%snew", libpath); // REL 0
    FILE* file = fopen(path, "w");                                            // CLOSE 0

    if (file)
    {
	retv              = 0;
	mt_vector_t* vals = VNEW(); // REL 1
	mt_map_values(db, vals);

	for (int vali = 0; vali < vals->length; vali++)
	{
	    mt_map_t*    entry = vals->data[vali];
	    mt_vector_t* keys  = VNEW(); // REL 2

	    mt_map_keys(entry, keys);

	    for (int keyi = 0; keyi < keys->length; keyi++)
	    {
		char* key = keys->data[keyi];
		char* val = MGET(entry, key);

		if (fprintf(file, "%s %s\n", key, val) < 0) retv = -1;
	    }

	    REL(keys); // REL 2

	    if (retv < 0) break;
	}

	if (fclose(file) == EOF) retv = -1; // CLOSE 0

	REL(vals); // REL 1

	if (retv == 0)
	{
	    if (rename(path, libpath) != 0) retv = -1;
	}
	else
	    printf("ERROR kvlines_write cannot write file\n");
    }
    else
	printf("ERROR kvlines_write cannot open file %s\n", path);

    REL(path); // REL 0

    return retv;
}

#endif
