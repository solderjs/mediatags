#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "apar_glob.h"
#include "m4a_json.h"
#include "mhash.h"

#define TABSPACE "    "

int m4a_display_json_tree(
    FILE *in,
    FILE *out)
{
    char     *line = NULL;
    size_t   len = 0;
    ssize_t  lnsz;
    int      prvlvl = 0;

    char pfx[512];
    int i, j;

    fprintf(out, "{\n");
    while ((lnsz = getline(&line, &len, in)) != -1) 
    {
        size_t span;
        int curlvl;
        char *tok;
        char *ptree[10];
        char lvlspc[256];

        line[lnsz-1] = '\0';
        //printf("Retrieved line of length %zu :\n", lnsz);
        //printf("%s", line);

        tok = strtok(line, " ");
        if ((tok != NULL) && (strcmp(tok, "Atom") != 0)) continue;

        i = 0;
        while (tok != NULL)
        {
            //printf("===> %s\n", tok);
            ptree[i] = tok;
            tok = strtok(NULL, " ");
            i++;
        }
        //for (j = 0; j < i; j++) printf ("===> %s\n", ptree[j]);

        pfx[0] = '\0';
        span = strspn(line, " ");
        curlvl = (span >> 2) + 1;
        curlvl = (curlvl << 1) - 1;
        //printf ("Initial number of spaces: %d [%d]\n", span, curlvl);
        if (prvlvl != 0) 
        { 
            if (curlvl > prvlvl)
            {
                strcat(pfx, ",\n");
                for (j = 0; j < prvlvl + 1; j++) strcat(pfx, TABSPACE);
                strcat(pfx, "\"value\": {\n");
            }
            else if (curlvl == prvlvl)
            {
                pfx[0] = '\n'; pfx[1] = '\0'; 
                for (j = 0; j < prvlvl; j++) strcat(pfx, TABSPACE);
                strcat(pfx, "},\n");
            }
            else
            {
                strcat(pfx, "\n");
                for (i = prvlvl + 1; i > curlvl+1; i--) 
                {
                    for (j = 1; j < i; j++) strcat(pfx, TABSPACE);
                    strcat(pfx, "}\n");
                }
                for (j = 1; j < i; j++) strcat(pfx, TABSPACE);
                strcat(pfx, "},\n");
            }
        }
        lvlspc[0] = '\0';
        for (j = 0; j < curlvl; j++) strcat(lvlspc, TABSPACE);
        strcat(pfx, lvlspc);
        strcat(lvlspc, TABSPACE);

        ptree[6][strlen(ptree[6])-1] = '\0'; // Delete the trailing comma
        fprintf(
            out, 
           "%s\"%s\": {\n%s\"start\": \"%s\",\n%s\"length\": \"%s\",\n%s\"end\": \"%s\"",
            pfx, ptree[1], 
            lvlspc, ptree[3], 
            lvlspc, ptree[6], 
            lvlspc, ptree[9]);

        prvlvl = curlvl;
    }

    /*
    **  Update brackets on completion
    */
    pfx[0] =  '\n'; pfx[1] = '\0';
    for (i = prvlvl + 1; i > 0; i--) 
    {
        for (j = 1; j < i; j++) strcat(pfx, TABSPACE);
        strcat(pfx, "}\n");
    }
    fprintf(out, "%s\n", pfx);

    if (line != NULL) free(line);

    return 0;
}

void m4a_stuff_backslash(char *inp, char *out)
{
    int i = 0;
    int j = 0;

    char prev = 'a';

    while (i <= (int)strlen(inp))
    {
        if (inp[i] == ' ')
        {
            if (prev != ' ') out[j++] = ' ';
            prev = ' ';
            i++;
        }
        else if (inp[i] == '"')
        {
            out[j++] = '\\';
            out[j++] = '"';
            i++;
            prev = 'a';
        }
        else if (inp[i] == '\t')
        {
            out[j++] = '\\';
            out[j++] = 't';
            i++;
            prev = 'a';
        }
        else
        {
            out[j++] = inp[i++];
            prev = 'a';
        }
    }
}

int m4a_display_json_tags(
    FILE *in,
    FILE *out,
    unsigned char *md5sum,
    unsigned char *sha1sum,
    char          *covr)
{
    char     *line = NULL;
    size_t   len = 0;
    ssize_t  lnsz;
    int      fst = 0;

    
    while ((lnsz = getline(&line, &len, in)) != -1) 
    {
        char *tok;
        char *ptree[64];
        char pfx[512];
        int i, j;
        char atom[128];
        char value[128];
        char sanitised[256];

        line[lnsz-1] = '\0';
        //printf("Retrieved line of length %zu :\n", lnsz);
        //printf("%s", line);

        tok = strtok(line, " ");

        // Empty Line
        if (tok == NULL) continue;

        // Overflowing Value has \n in it
        if ((tok != NULL) && (strcmp(tok, "Atom") != 0)) 
        {
            line[strlen(line)] = ' ';
            m4a_stuff_backslash(line, sanitised);
            fprintf(out, "\\n %s", sanitised);
            continue;
        }

        // Gather all tokens
        i = 0;
        while (tok != NULL)
        {
            //printf("===> %s\n", tok);
            ptree[i] = tok;
            tok = strtok(NULL, " ");
            i++;
        }
        //for (j = 0; j < i; j++) printf ("===> %s\n", ptree[j]);


        // Aggregate Token for Atom name
        atom[0] = '\0';
        j = 1;
        ptree[j][strlen(ptree[j]) - 1] = '\0';
        while (strcmp(ptree[j], "contains:") != 0)
        {
            strcat(atom, ptree[j]);
            j++;
        }
        strcat(atom, "\"");

        // For Covr it is enough to display value ass "true"
        if (strcmp(atom, "\"covr\"") != 0)
        {
            j++;
            value[0] = '\0';
            strcpy(value, ptree[j++]);
            for (; j < i; j++) sprintf(value, "%s %s", value, ptree[j]);
        }
        else
            strcpy(value, "true");


        // Generate appropriate Prefix
        pfx[0] = '\0';
        if (fst) strcat(pfx, "\",\n");
        strcat(pfx, TABSPACE);

        m4a_stuff_backslash(value, sanitised);
        fprintf(
            out, 
           "%s%s: \"%s",
            pfx, atom, sanitised);

        fst = 1;
    }

    if (md5sum != NULL)
    {
        const char *tab = TABSPACE;
        fprintf(
            out, "\",\n%s\"md5sum\": \"%s",
            tab, md5sum);
    }
    if (sha1sum != NULL)
    {
        const char *tab = TABSPACE;
        fprintf(
            out, "\",\n%s\"sha1sum\": \"%s",
            tab, sha1sum);
    }

    fprintf(out, "\"\n}\n");
    if (line != NULL) free(line);

    return 0;
}



int m4a_stream_chksum(
    char *fname, 
    unsigned char *md5sum,
    unsigned char *sha1sum)
{
    int             i;
    int             len = 0;
    AtomicInfo*     atom = NULL;
    MHASH           mdd  = NULL;
    MHASH           shd  = NULL;
    unsigned char   *md5res;
    unsigned char   *sha1res;
    unsigned char   bfr[M4A_CHKSUM_BFR_SZ];
    FILE            *fp;
    int             cnt;

    for (i=0; i < atom_number; i++) 
    { 
        atom = &parsedAtoms[i]; 
        if (memcmp(atom->AtomicName, "mdat", 4) == 0) break;
    }

    if (i == atom_number) return 3;

    if (atom->AtomicLength > 100)
    {
        len = atom->AtomicLength;
    } else if (atom->AtomicLength == 0) 
    { 
        //mdat.length = 0 = ends at EOF
        len = (uint32_t)file_size - atom->AtomicStart;
    } else if (atom->AtomicLengthExtended != 0 ) 
    {
        // Adding a (limited) uint64_t into a uint32_t
        len = atom->AtomicLengthExtended; 
    }

    if (md5sum != NULL)
    {
        mdd = mhash_init(MHASH_MD5);
        if (mdd == MHASH_FAILED) return 1;
    }
    if (sha1sum != NULL)
    {
        shd = mhash_init(MHASH_SHA1);
        if (shd == MHASH_FAILED) return 1;
    }

    if ((fp = fopen(fname, "r")) == NULL)
    {
        perror("fopen");
        return 1;
    }

    if (fseek(fp, (atom->AtomicStart - 1), SEEK_SET) != 0)
    {
        perror("fseek");
        return 2;
    }

    while ((cnt = fread(&bfr, 1, M4A_CHKSUM_BFR_SZ, fp)) > 0) 
    {
        if (md5sum != NULL) mhash(mdd, bfr, cnt);
        if (sha1sum != NULL) mhash(shd, bfr, cnt);
    }

    if (md5sum != NULL)
    {
        md5res = (unsigned char *) mhash_end(mdd);
        for (i = 0; i < (int)mhash_get_block_size(MHASH_MD5); i++) 
        {
            // printf("%.2x", md5res[i]);
            sprintf((char *)&md5sum[i<<1], "%.2x", md5res[i]);
        }
        // printf (" %s\n", md5sum);
    }

    if (sha1sum != NULL)
    {
        sha1res = (unsigned char *) mhash_end(shd);
        for (i = 0; i < (int)mhash_get_block_size(MHASH_SHA1); i++) 
        {
            // printf("%.2x", sha1res[i]);
            sprintf((char *)&sha1sum[i<<1], "%.2x", sha1res[i]);
        }
        // printf (" %s\n", sha1sum);
    }

    return 0;
}

int m4a_disp_tree()
{
    int i;
    for (i=0; i < atom_number; i++) { 
        AtomicInfo* atom = &parsedAtoms[i]; 
               
        fprintf(stdout, "%i  -  Atom \"%s\" (level %u) has next atom at #%i\n",
            i, atom->AtomicName, atom->AtomicLevel, 
            atom->NextAtomNumber);
        }

    return 0;
}
