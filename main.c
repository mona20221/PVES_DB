
#include <stdio.h>
#include <string.h>
#define inodes 7 // maximalny pocet i-uzlov
#define blocks 10 // maximalny pocet datovych blokov
#define length 15 // velkost bloku
struct i_node
{
    int mode; // mode=0 volny i-uzol, mode=1 obsadeny i-uzol
    char filename[15]; // meno suboru najviac 8 znakov
    int filelength; // pocet znakov suboru
    int offset; // poloha pri citani zo suboru
    int first_direct; // prvy priamy datovy blok
    int second_direct; // druhy priamy datovy blok
    int third_direct;
    
};
struct datablock
{
    int mode; // mode=0 free node, mode=1 used node
    char data[length];
};
// session layer
void terminal (void); // komunikacia so systemom
// application layer
void write_file (void); // zapisuj novy subor
void print_file (void); // vypis obsah suboru
void help_me (void);

// filesystem interface
int create_file (char *name); // zalozime i-uzol a zalozime dva datove bloky
int open_file (char *name); // najdeme i-uzol suboru
int close_file (char *name); // subor uz necitame, offset=0
int myputc (char c, int filenumber); // zapis znaku do suboru
char mygetc (int filenumber); // citanie znaku zo suboru
// filesystem core
struct i_node i_node_chain [inodes]; // zoznam i-uzlov
struct datablock datablock_chain[blocks]; // zoznam datovych blokov
void initialise (void); // oznac datove struktury ako prazdne
void dump_data (void); // ukaz obsah datovych struktur
// hardware interface
void disc_backup (int mode); // mode=0 nacitanie disku zo suboru mode=1 zapis
// others
void profstring (char *line); // lebo medved

int main(void) // nacitanie obsahu disku zo suboru, praca na terminali a ulozenie na zaver
{
    char line[80];
    printf("\n ----- select options -----\n\n (m) - manual\n\n");
    initialise();
    disc_backup(0); // nacitame data
    terminal(); // *********** system pracuje ************
    disc_backup(1); // ulozime data
    printf("\nsystem shutdown, press any key...");
    profstring(line);
    return 0;
}
void terminal(void) // ovladanie modelu prikazmi
{
    char line[80];
    while(1) // slucka terminalu caka na prikaz stale dookola
    {
        printf("\n> ");
        profstring(line);
        if(line[0] == 'm') help_me(); // pomocka pre ovladanie MANUAL
        if(line[0] == 'f') initialise(); // formatuj virtualny disk
        if(line[0] == 'd') dump_data(); // obsah virtualneho disku
        if(line[0] == 'w') write_file(); // zapisuj novy subor
        if(line[0] == 'p') print_file(); // vypis obsah suboru
        if(line[0] == 'e') break; // EXIT
    }
}
void write_file(void) // zapisuj novy subor
{
    int i, fileno;
    char line[100];
    printf("\nfilename: "); // piseme meno suboru
    profstring(line);
    fileno = create_file(line);// vytvori sa prazdny subor
    printf("\ndata: "); // piseme obsah suboru
    profstring(line);
    for(i=0;;i++)
    {
        if(line[i]=='\0') break;
        if(myputc(line[i],fileno)==0) break;
    }
}
void print_file(void) // vypis obsah suboru
{
    int fileno;
    char c, line[80];
    printf("\nfilename: "); // piseme meno suboru
    profstring(line);
    fileno = open_file(line); // najdeme i-uzol suboru, vratime index
    if(fileno==(-1)) printf("\nfile not found");
    else
        while(1)
        {
            c = mygetc(fileno); // citanie znaku zo suboru
            if(c==EOF) break;
            printf("%c", c);

        }
    (void)close_file(line);
}
void help_me(void) // print MANUAL
{
    printf("\n List of commands\n ----------------\n");
    printf("\n m - this manual");
    printf("\n f - format virtual disc");
    printf("\n d - dump data on virtual disc");
    printf("\n w - write a new file");
    printf("\n p - print saved file");
    printf("\n e - exit");
    printf("\n\n");
}
int create_file(char *name) // zalozime i-uzol a dva prazdne datove bloky
{
    int i,d; // i=index i-node, d=index data block
    // zalozime i-uzol
    for(i=0;i<inodes;i++) // prehladaj pole i-uzlov
        if(i_node_chain[i].mode==0) break; // nasiel volny i-uzol
    if(i==inodes) // nenasiel volny i-uzol
    { printf("\n not free i-node for another file"); return(-1); }
    i_node_chain[i].mode=1; // tento i-uzol oznacime ako obsadeny
    i_node_chain[i].filelength=0; // subor je zatial bez obsahu
    strcpy(i_node_chain[i].filename,name); // meno suboru vlozime do i-uzla
    // obsadime dva datove bloky
    for(int run=1;run<=3;run++)
    {
        for(d=0;d<blocks;d++) // prehladaj pole datovych blokov
        {
            if(d==blocks) // nenasiel volny datovy blok
            {
                printf("\n not free datablock for this file");
                i_node_chain[i].mode=0; // vratim i-node
                return(-1);
            }
            if(datablock_chain[d].mode==0) break;// nasiel volny datovy blok
        }
        datablock_chain[d].mode=1; // oznacime ho ako obsadeny
        if(run==1)i_node_chain[i].first_direct=d; // pripojime prvy do i-node
        if(run==2)i_node_chain[i].second_direct=d; // pripojime druhy do i-node
        if(run==3)i_node_chain[i].third_direct=d;
    }
    return i; // vrati index i-uzla
}
int open_file(char *name) // najdeme i-uzol suboru, vratime index
{
    int i;
    for(i=0;i<inodes;i++) // prehladaj pole i-uzlov
        if(strcmp(i_node_chain[i].filename, name)==0) break; // zhoda!
    if(i==inodes) return(-1); // nenasiel take meno
    return i; // nasiel, vrati index i-uzla
}
int close_file(char *name) // najdeme i-uzol suboru, nulujeme offset
{
    int i;
    for(i=0;i<inodes;i++) // prehladaj pole i-uzlov
        if(strcmp(i_node_chain[i].filename, name)==0) break; // zhoda!
    if(i==inodes) return 0; // nenasiel, nevykonal
    i_node_chain[i].offset=0;
    return 1; // vykonal
}



int myputc(char c, int filenumber) // zapis znaku do suboru
{
    int d, pos, block, position;
    pos = i_node_chain[filenumber].filelength;
    block = (int)(pos / length); // modulo: ktory blok
    position = pos % (length); // zvysok: kde v bloku
    if(block > 1) return 0; // nie je uz kam pisat
    if(block==0) // je to prvy datovy blok
        d=i_node_chain[filenumber].first_direct;

    if(block==1) // je to druhy datovy blok
        d=i_node_chain[filenumber].second_direct;
    datablock_chain[d].data[position] = c; // pridaj znak
    i_node_chain[filenumber].filelength++; // uprav data v i-uzle
    return 1; // zapisane
}
char mygetc(int filenumber) // citanie znaku zo suboru
{
    int d, pos, block, position;
    pos = i_node_chain[filenumber].offset;
    if(pos == i_node_chain[filenumber].filelength)// uz sme za blokmi
        return EOF;
    block = (int)(pos / length); // ktory block
    position = pos % length; // ktora poloha v bloku
    if(block==0) // je to prvy datovy blok
        d=i_node_chain[filenumber].first_direct;
    if(block==1) // je to druhy datovy blok
        d=i_node_chain[filenumber].second_direct;
    i_node_chain[filenumber].offset++; // upravime data v i-uzle
    return(datablock_chain[d].data[position]); // vrat znak
}
void initialise(void) // oznac datove struktury ako prazdne
{
    int i,k;
    for(i=0;i<inodes;i++) // prazdne pole i-uzlov
    {
        i_node_chain[i].mode =0;
        strcpy(i_node_chain[i].filename,"--------");
        i_node_chain[i].filelength =0;
        i_node_chain[i].offset =0;
        i_node_chain[i].first_direct =(-1);
        i_node_chain[i].second_direct=(-1);
        i_node_chain[i].third_direct=(-1);
    };
    for(i=0;i<blocks;i++) // prazdne pole datovych blokov
    {
        datablock_chain[i].mode =0;
        for(k=0;k<length;k++) datablock_chain[i].data[k]='-';
    }
}
void dump_data(void) // vypis obsahy datovych struktur
{
    int i,k;
    for(i=0;i<inodes;i++) // vypis pole i-uzlov
    {
        printf("\ni-node %2d: mode %d", i, i_node_chain[i].mode);
        printf("\n filename %s", i_node_chain[i].filename);
        printf("\n offset %d", i_node_chain[i].offset);
        printf("\n filelength %d", i_node_chain[i].filelength);
        printf("\n first_direct %d", i_node_chain[i].first_direct);
        printf("\n second_direct %d\n", i_node_chain[i].second_direct);
        printf("\n third_direct %d\n", i_node_chain[i].third_direct);
    };
    for(i=0;i<blocks;i++) // vypis pole datovych blokov
    {
        printf("\ndatablock %2d: mode %d", i, datablock_chain[i].mode);
        printf("\n data {");
        for(k=0;k<length;k++)
            printf("%c", datablock_chain[i].data[k]);
        printf("}\n");
    }
}
void disc_backup(int mode) // mode=0 nacitanie disku zo suboru, mode=1 ulozenie do suboru
{
    int i;
    FILE *dumpfile;
    if(mode==0) {
        dumpfile = fopen("zaloha_disku.txt", "r");
        if(dumpfile!=NULL)
        {
            for(i=0;i<inodes;i++) // hardcopy pola struktur i-uzlov zo suboru
                fread(&i_node_chain[i], sizeof(struct i_node), 1, dumpfile);
            for(i=0;i<blocks;i++) // hardcopy pola struktur datovych blokov zo suboru
                fread(&datablock_chain[i], sizeof(struct datablock), 1, dumpfile);
            fclose(dumpfile);
        }
        else printf("\n> empty disc\n");

    }
    if(mode==1) {
        dumpfile = fopen("zaloha_disku.txt", "w");
        for(i=0;i<inodes;i++) // hardcopy pola struktur i-uzlov zo suboru
            fwrite(&i_node_chain[i], sizeof(struct i_node), 1, dumpfile);
        for(i=0;i<blocks;i++) // hardcopy pola struktur datovych blokov zo suboru
            fwrite(&datablock_chain[i], sizeof(struct datablock), 1, dumpfile);
        fclose(dumpfile);
    }
}
void profstring(char *line)
// vlastny getstring, namiesto velmi divneho scanf()
{
    for(int i=0;i<80;i++) // cita riadok z terminalu
    {
        line[i]=getchar();
        if(line[i]=='\n') { line[i]='\0'; break; }
    }
}

