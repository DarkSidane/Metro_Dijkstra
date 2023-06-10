//Importation des bibliothèques
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
//Déclaration des constantes
#define TEMPSDIRECT 1 //durée du trajet de 2 stations connectées directement
#define TEMPSCHANGEMENT 5 //durée changement de ligne
#define L_MAX 50 //On définit une longueur maximale de caractère
#define N_STATIONS 304
#define INFINI 4000000 //Utile pour dijkstra
#define POIDS_DIRECT 1
#define POIDS_TRANSITION 5

typedef struct station
{
    uint32_t id;
    char nom[L_MAX];
    uint32_t visite;//Booleen
    uint32_t pi;

    uint32_t pds;
    uint32_t precedent;
    char ligne_precedent[L_MAX];

}station;

typedef struct arc
{
    uint32_t id_depart;
    uint32_t id_arrivee;
    char ligne[L_MAX];
}arc;


typedef struct maillon
{
    arc *arc_courant; //
    uint32_t poids;
    struct maillon* suivant; //
}maillon;


typedef struct liste
{
    maillon* tete;
    maillon* queue;
    uint32_t taille;
}liste;


// Fonction de manipulation de liste chainée basique
maillon *new_maillon(uint32_t id_d, uint32_t id_a, char ligne_t[L_MAX]);
liste *new_liste();
void add_tete(liste* l, uint32_t id_d, uint32_t id_a, char ligne[L_MAX]);

//uint32_t dijkstra(station** stations, uint32_t id_depart, uint32_t id_arrivee);


// Définition des en-tête de fonction
station** charger_stations();
liste** charger_arcs(station** stations);
void calcul_arcsvirtuels(station** stations);
int** matrice_des_couts(station** stations);
void dijkstra(station** stations, liste** arcs, uint32_t id_depart, uint32_t id_arrivee);
void afficher_lettre(station** stations, char c);



// Fonction principale
int main()
{
    station** stations = charger_stations();
    liste** arcs = charger_arcs(stations);
    /*calcul_arcsvirtuels(stations);*/
    uint32_t sommet1;
    uint32_t sommet2;
    char c;
    printf("Veuillez rentrer la première lettre de votre station de départ  : ");
    scanf("%c", &c);
    afficher_lettre(stations, c);
    fflush(stdin);
    printf("\n");

    printf("Veuillez rentrer l'id du sommet de depart : \n");
    scanf("%d", &sommet1);
    fflush(stdin);

    printf("Veuillez rentrer la première lettre de votre station d'arrivée : ");
    scanf(" %c", &c);
    afficher_lettre(stations, c);
    fflush(stdin);
    printf("\n");

    printf("\nVeuillez rentrer un sommet d'arrivée : \n");
    scanf("%d", &sommet2);

    printf("\n==========================\n");


    dijkstra(stations, arcs, sommet1-1, sommet2-1);

    return EXIT_SUCCESS;
}
void afficher_lettre(station** stations, char c)
{
    //Affiche l'id de tous les stations commençant par une lettre donnée
    c = toupper(c);
    for (int i = 0; i<N_STATIONS; i++)
    {
        if (stations[i]->nom[0] == c)
            printf("%s :%d \n", stations[i]->nom, stations[i]->id+1);
    }
    printf("\n-----------\n");
}
maillon *new_maillon(uint32_t id_d, uint32_t id_a, char ligne_t[L_MAX])
{
    maillon *m = malloc(sizeof(maillon));
    if (m== NULL)
    {
        assert(0);
    }
    arc *a = malloc(sizeof(arc));
    a->id_depart = id_d;
    a->id_arrivee = id_a;
    strcpy(a->ligne, ligne_t);

    m->arc_courant = a;
    return m;
}

liste *new_liste()
{
    liste* l = malloc(sizeof(liste));
    assert(l!=NULL);
    l->taille = 0;
    l->tete = NULL;
    l->queue = NULL;
    return l;
}

void add_tete(liste* l, uint32_t id_d, uint32_t id_a, char ligne[L_MAX])
{
    //Ajout d'un maillon atteint par un arc directe dans la liste des voisins
    maillon* m = new_maillon(id_d, id_a,ligne);
    m->suivant = l->tete;
    l->tete = m;
    if (l->taille == 0)
        l->queue = m;
    l->taille++;
}
void add_tete_maillon(liste *l, maillon *m)
{
    m->suivant = l->tete;
    l->tete = m;
    if (l->taille == 0)
        l->queue = m;
    l->taille++;
}

void add_tete_voisins(liste *l, uint32_t v)
{
    maillon *m = malloc(sizeof(maillon));
    if (m== NULL)
    {
        assert(0);
    }
}
station** charger_stations()
{
    //Stocke dans un tableau de taille 305, l'ensemble des stations stocké dans "Metro Paris Data - Stations.csv"
    FILE *f_station;
    station **stations = malloc(N_STATIONS*sizeof(station));
    for (int j = 0; j<N_STATIONS; j++)
    {
        stations[j] = malloc(sizeof(station));
        stations[j]->visite = 0;
        stations[j]->pi = INFINI;
    }
    char c = '0';
    int state = 0;

    if ((f_station = fopen("Metro Paris Data - Stations.csv", "r")) == NULL)
    {
        printf("Erreur : fichier non trouvé");
        exit(1);
    }
    //saut de la première ligne
    while (c != '\n')
    {
        fscanf(f_station, "%c", &c);
    }
    int i = 0;
    char id_scan[L_MAX]; //accumulateur
    id_scan[0] = '\0';
    uint32_t b_boucle = 1;

    int scan;

    while (b_boucle == 1)
    {
        scan = fscanf(f_station, "%c", &c);
        if (c == ',')
        {
            state = (state + 1)%2;
        }
        else if ((c == '\n')||(scan==EOF))
        {
            state = (state + 1)%2;
            stations[i]->id = atoi(id_scan)-1;
            memset(id_scan,0,sizeof(id_scan));
            id_scan[0] = '\0';
            i++;
            if (scan == EOF)
                b_boucle = 0; //Ca sert à corriger un bug ou la dernière station est skip
        }
        //State permet de faire savoir si on est sur la première, deuxième colonne
        else if (state == 0)
        {
            strncat(stations[i]->nom, &c, 1); //Ajoute au nom le caractère c
        }
        else if (state == 1)
        {
            strncat(id_scan, &c, 1);
        }
    }

    return stations;
}
liste** charger_arcs(station** stations)
{
    //Stocke dans un tableau de taille 735, l'ensemble des stations stocké dans "Metro Paris Data - Stations.csv"
    FILE *f_arc;
    liste** arcs = malloc(N_STATIONS*sizeof(liste*));
    for (int i = 0; i<N_STATIONS; i++)
    {
        arcs[i] = new_liste();
    }
    char c = '0';
    int state = 0;

    if ((f_arc = fopen("Metro Paris Data - Aretes.csv", "r")) == NULL)
    {
        printf("Erreur : fichier non trouvé");
        exit(1);
    }
    //saut de la première ligne
    while (c != '\n')
    {
        fscanf(f_arc, "%c", &c);
    }
    int i = 0;
    int k = 0; //Cet indice k sert à indicer la k-ème station et à lui attribuer sa i-ème arrête
    char station_depart[L_MAX];
    char station_arrivee[L_MAX];
    char ligne_tempo[L_MAX]; //accumulateur
    station_depart[0] = '\0';
    station_arrivee[0] = '\0';
    ligne_tempo[0] = '\0';
    uint32_t id_s;

    int scan;
    int b_boucle = 1;
    while (b_boucle == 1)
    {
        scan = fscanf(f_arc, "%c",&c);
        if (c == ',')
        {
            state = (state+1)%3;
        }
        else if ((c == '\n')||(scan==EOF))
        {
            state = (state+1)%3;
            //printf("(%s)\n", ligne_tempo);
            add_tete(arcs[atoi(station_depart)-1], atoi(station_depart)-1, atoi(station_arrivee)-1,ligne_tempo);
            memset(station_arrivee,0,sizeof(station_arrivee));
            memset(station_depart,0,sizeof(station_depart));
            memset(ligne_tempo,0,sizeof(ligne_tempo));
            station_depart[0] = '\0';
            station_arrivee[0] = '\0';
            ligne_tempo[0] = '\0';

            if (scan == EOF)
                b_boucle = 0;
            i++;
        }
        else if (state == 0)
        {
            strncat(station_arrivee, &c, 1);
        }
        else if (state == 1)
        {
            strncat(station_depart, &c, 1);
        }
        else if (state == 2)
        {
            strncat(ligne_tempo, &c, 1);
        }
    }
    return arcs;
}

uint32_t minpi(station** stations)
{
    uint32_t min = INFINI;
    uint32_t id;
    for (int i = 0; i< N_STATIONS;i++)
    {
        if (stations[i]->pi<min&&stations[i]->visite != 1)
        {
            min = stations[i]->pi;
            id = i;
        }
    }
    return id;
}


void dijkstra(station** stations, liste** arcs, uint32_t id_depart, uint32_t id_stop)
{
    liste* s = new_liste(); //liste pivot
    int v = 0;
    stations[id_depart]->visite = 1;
    uint32_t pivot_precedent = id_depart;
    uint32_t pivot = id_depart;
    stations[pivot]->pi = 0;
    strcpy(stations[pivot]->ligne_precedent,"DEFAULT");
    char derniereLigne[L_MAX] = "DEFAULT";
    //stations pivot = new_maillon(id_depart, )


    while((v<N_STATIONS-1)||stations[id_stop]->visite==0)
    {
        maillon* e = arcs[pivot]->tete;
        while (e!=NULL)
        {
            if (stations[e->arc_courant->id_arrivee]->visite ==0)
            {
            uint32_t poids = POIDS_DIRECT;

            if (strcmp(e->arc_courant->ligne, stations[pivot]->ligne_precedent)!= 0 && strcmp(stations[pivot]->ligne_precedent, "DEFAULT")!=0)
            {
                poids += POIDS_TRANSITION;
            }
            if (stations[e->arc_courant->id_arrivee]->pi > stations[pivot]->pi+poids)
            {
                stations[e->arc_courant->id_arrivee]->pi = stations[pivot]->pi + poids;
                stations[e->arc_courant->id_arrivee]->pds = poids;
                strcpy(stations[e->arc_courant->id_arrivee]->ligne_precedent, e->arc_courant->ligne);
                stations[e->arc_courant->id_arrivee]->precedent = stations[pivot]->id;
            }
            }
            e = e->suivant;

        }
        pivot = minpi(stations);

        add_tete(s, pivot, stations[pivot_precedent]->id,stations[pivot]->ligne_precedent);

        stations[pivot]->visite = 1;
        v++;

    }
    //maillon* e = id_stop;
    printf(" Trajet de %d (id : %s) a %s(id : %d) : %d minutes \n", id_depart+1,stations[id_depart]->nom, stations[id_stop]->nom,id_stop+1, stations[id_stop]->pi);

    printf("\n------------------------------\n");
    printf("Chemin parcouru : \n");
    uint32_t p = id_stop;
    while (p != id_depart)
    {
        printf("%s(%d)<-%s(%d) (ligne %s) poids +%d\n", stations[p]->nom,p+1,stations[stations[p]->precedent]->nom, stations[p]->precedent+1, stations[p]->ligne_precedent, stations[p]->pds);
        p = stations[p]->precedent;
    }
    printf("\n");
}

