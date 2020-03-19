#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include <SDL/SDL.h>
#include <windows.h>

//Bibliothèque nécessaire à la liaison bluetooth
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>



/*=============================================================================
  Définition des constantes nécessaire à la liaison bluetooth
=============================================================================*/

#define RX_SIZE         4096    /* taille tampon d'entrée  */
#define TX_SIZE         4096    /* taille tampon de sortie */
#define MAX_WAIT_READ   1    /* temps max d'attente pour lecture (en ms) */



/*=============================================================================
  Variables globales nécessaire à la liaison bluetooth
=============================================================================*/

/* Handle du port COM ouvert */
HANDLE g_hCOM1 = NULL;

/* Handle du port COM ouvert */
HANDLE g_hCOM2 = NULL;

/* Délais d'attente sur le port COM */
COMMTIMEOUTS g_cto =
{
    MAX_WAIT_READ,  /* ReadIntervalTimeOut          */
    0,              /* ReadTotalTimeOutMultiplier   */
    MAX_WAIT_READ,  /* ReadTotalTimeOutConstant     */
    0,              /* WriteTotalTimeOutMultiplier  */
    0               /* WriteTotalTimeOutConstant    */
};

/* Configuration du port COM */
DCB g_dcb =
{
    sizeof(DCB),        /* DCBlength            */
    115200,               /* BaudRate             */
    TRUE,               /* fBinary              */
    FALSE,              /* fParity              */
    FALSE,              /* fOutxCtsFlow         */
    FALSE,              /* fOutxDsrFlow         */
    DTR_CONTROL_DISABLE, /* fDtrControl          */
    FALSE,              /* fDsrSensitivity      */
    FALSE,              /* fTXContinueOnXoff    */
    FALSE,              /* fOutX                */
    FALSE,              /* fInX                 */
    FALSE,              /* fErrorChar           */
    FALSE,              /* fNull                */
    RTS_CONTROL_DISABLE, /* fRtsControl          */
    FALSE,              /* fAbortOnError        */
    0,                  /* fDummy2              */
    0,                  /* wReserved            */
    0x100,              /* XonLim               */
    0x100,              /* XoffLim              */
    8,                  /* ByteSize             */
    NOPARITY,           /* Parity               */
    ONESTOPBIT,         /* StopBits             */
    0x11,               /* XonChar              */
    0x13,               /* XoffChar             */
    '?',                /* ErrorChar            */
    0x1A,               /* EofChar              */
    0x10                /* EvtChar              */
};



/*=============================================================================
  Fonctions du module nécessaire à la liaison bluetooth
=============================================================================*/

BOOL OpenCOM1(int nId);
BOOL ReadCOM1(void* buffer1, int nBytesToRead1, int* pBytesRead1);
BOOL OpenCOM2(int nId);
BOOL ReadCOM2(void* buffer, int nBytesToRead, int* pBytesRead);
BOOL CloseCOM();



//Définition des constantes utiles
const int LONGUEUR_JOUEUR=100;
const int HAUTEUR_JOUEUR=110;
const int TAILLE_BALLE=48;

const int16_t LONGUEUR_TERRAIN = 800;
const int16_t LARGEUR_TERRAIN = 600;
const int16_t LARGEUR_BUT =200;



//Variable déclarant le vainqueur
int victoire;



//Création de 2 structures
typedef struct
{
    int16_t x;
    int16_t y;
} vitesse;
typedef struct
{
    int8_t x_plus;
    int8_t x_moins;
    int8_t y_plus;
    int8_t y_moins;
} cmd_joueur;



int main ( int argc, char** argv )
{

    /* variables locales */
    char buffer1[5];
    char buffer2[5];
    int nId1, nBytesRead1;
    int nId2, nBytesRead2;

    /* demande du numéro du port COM */
    scanf("%d", &nId1); // Joueur1
    scanf("%d", &nId2); // Joueur2

    // Si erreur, arrêter le programme
    if(!OpenCOM1(nId1)) return 1;
    if(!OpenCOM2(nId2)) return 1;

    /* Variable */
    int a1; // Variable du joueur 1 droite/gauche
    int b1; // Variable du joueur 1 haut/bas
    int a2; // Variable du joueur 2 droite/gauche
    int b2; // Variable du joueur 2 haut/bas

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }
    //make sure SDL cleans up before exit
    atexit(SDL_Quit);

    vitesse vitesse_balle= {-1,1},vitesse_joueur1= {1,1},vitesse_joueur2= {1,1};
    cmd_joueur cmd_joueur1= {0,0,0,0},cmd_joueur2= {0,0,0,0};

    SDL_Surface* screen=SDL_SetVideoMode(LONGUEUR_TERRAIN, LARGEUR_TERRAIN,32,SDL_HWSURFACE|SDL_DOUBLEBUF);

    if ( !screen )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

    // load an image
    SDL_Surface* but1=SDL_CreateRGBSurface(SDL_HWSURFACE,4,LARGEUR_BUT,32,0,0,0,0);
    SDL_Surface* but2=SDL_CreateRGBSurface(SDL_HWSURFACE,4,LARGEUR_BUT,32,0,0,0,0);

    SDL_Rect pos_but1;
    pos_but1.x = 0;
    pos_but1.y = (LARGEUR_TERRAIN-LARGEUR_BUT)/2;

    SDL_Rect pos_but2;
    pos_but2.x = (LONGUEUR_TERRAIN-but2->w);
    pos_but2.y = (LARGEUR_TERRAIN-LARGEUR_BUT)/2;

    SDL_Surface* balle_bmp= SDL_LoadBMP("balle2.bmp");
    SDL_Surface* joueur1_bmp=SDL_LoadBMP("joueur1.bmp");
    SDL_Surface* joueur2_bmp = SDL_LoadBMP("joueur2.bmp");
    SDL_Surface* victoire1_bmp= SDL_LoadBMP("victoire_joueur1.bmp");
    SDL_Surface* victoire2_bmp= SDL_LoadBMP("victoire_joueur2.bmp");

    SDL_Rect pos_balle;
    pos_balle.x=(LONGUEUR_TERRAIN-balle_bmp->w)/2;
    pos_balle.y=(LARGEUR_TERRAIN-balle_bmp->h)/2;

    SDL_Rect pos_joueur1;
    pos_joueur1.x=10;
    pos_joueur1.y=(LARGEUR_TERRAIN-joueur1_bmp->h)/2;

    SDL_Rect pos_joueur2;
    pos_joueur2.x=LONGUEUR_TERRAIN-10-joueur2_bmp->w;
    pos_joueur2.y=(LARGEUR_TERRAIN-joueur2_bmp->h)/2;

    SDL_Rect pos_vict1;
    pos_vict1.x=(LONGUEUR_TERRAIN-victoire1_bmp->w)/2;
    pos_vict1.y=(LARGEUR_TERRAIN-victoire1_bmp->h)/2;

    SDL_Rect pos_vict2;
    pos_vict2.x=(LONGUEUR_TERRAIN-victoire2_bmp->w)/2;
    pos_vict2.y=(LARGEUR_TERRAIN-victoire2_bmp->h)/2;

    //Définition des variables indiquant les dimenssions des images
    const int LONGUEUR_JOUEUR=joueur2_bmp->w;
    const int HAUTEUR_JOUEUR=joueur1_bmp->h;
    const int LONGUEUR_BALLE=balle_bmp->w;
    const int HAUTEUR_BALLE=balle_bmp->h;

    // program main loop
    bool done = false;
    while (!done){  //s'applique jusqu'à la fin

        for(int h=0;h<1000000;h++){}; //Ralentis la partie pour réduire l'effet du retard liée au bluetooth

        if(ReadCOM1(buffer1, sizeof(buffer1)-1, &nBytesRead1)){ // Si on reçoit/lit des données pour le joueur 1

                a1=buffer1[1]-'0';
                b1=buffer1[0]-'0';

                if(a1==2){
                    cmd_joueur1.x_plus=0;
                    cmd_joueur1.x_moins=1;}
                else if(a1==1){
                    cmd_joueur1.x_plus=1;
                    cmd_joueur1.x_moins=0;}
                else{
                    cmd_joueur1.x_plus=0;
                    cmd_joueur1.x_moins=0;}

                if(b1==2){
                    cmd_joueur1.y_plus=0;
                    cmd_joueur1.y_moins=1;}
                else if(b1==1){
                    cmd_joueur1.y_plus=1;
                    cmd_joueur1.y_moins=0;}
                else{
                    cmd_joueur1.y_plus=0;
                    cmd_joueur1.y_moins=0;}
            }

            if(ReadCOM2(buffer2, sizeof(buffer2)-1, &nBytesRead2)) { // Si on reçoit/lit des données pour le joueur 2

                a2=buffer2[1]-'0';
                b2=buffer2[0]-'0';

                if(a2==2){
                    cmd_joueur2.x_plus=0;
                    cmd_joueur2.x_moins=1;}
                else if(a2==1){
                    cmd_joueur2.x_plus=1;
                    cmd_joueur2.x_moins=0;}
                else{
                    cmd_joueur2.x_plus=0;
                    cmd_joueur2.x_moins=0;}

                if(b2==2){
                    cmd_joueur2.y_plus=0;
                    cmd_joueur2.y_moins=1;}
                else if(b2==1){
                    cmd_joueur2.y_plus=1;
                    cmd_joueur2.y_moins=0;}
                else{
                    cmd_joueur2.y_plus=0;
                    cmd_joueur2.y_moins=0;}
            }

        //Réalisation du rebond élastique de la balle
        if ((vitesse_balle.x==1)&&((pos_balle.x==(LONGUEUR_TERRAIN-LONGUEUR_BALLE)) || (pos_balle.x==(LONGUEUR_TERRAIN-LONGUEUR_BALLE-1))||
                                   (((pos_joueur2.x-pos_balle.x==LONGUEUR_BALLE) || (pos_joueur2.x-pos_balle.x==LONGUEUR_BALLE-1)) && ((pos_joueur2.y-pos_balle.y<HAUTEUR_BALLE) || (pos_joueur2.y-pos_balle.y<HAUTEUR_BALLE-1))
                                    && ((pos_balle.y-pos_joueur2.y<HAUTEUR_JOUEUR) || (pos_balle.y-pos_joueur2.y<HAUTEUR_JOUEUR-1))) ||
                                   (((pos_joueur1.x-pos_balle.x==LONGUEUR_BALLE) || (pos_joueur1.x-pos_balle.x==LONGUEUR_BALLE-1)) && ((pos_joueur1.y-pos_balle.y<HAUTEUR_BALLE) || (pos_joueur1.y-pos_balle.y<HAUTEUR_BALLE-1))
                                    && ((pos_balle.y-pos_joueur1.y<HAUTEUR_JOUEUR) || (pos_balle.y-pos_joueur1.y<HAUTEUR_JOUEUR-1)))))
        {
            vitesse_balle.x=-1;
        }


        if ((vitesse_balle.x==-1)&&((pos_balle.x==0)|| (pos_balle.x==-1)||
                                    (((pos_balle.x-pos_joueur2.x==LONGUEUR_JOUEUR) ||(pos_balle.x-pos_joueur2.x==LONGUEUR_JOUEUR-1)) && ((pos_joueur2.y-pos_balle.y<HAUTEUR_BALLE) || (pos_joueur2.y-pos_balle.y<HAUTEUR_BALLE-1))
                                     && ((pos_balle.y-pos_joueur2.y<HAUTEUR_JOUEUR)|| (pos_balle.y-pos_joueur2.y<HAUTEUR_JOUEUR-1))) ||
                                    (((pos_balle.x-pos_joueur1.x==LONGUEUR_JOUEUR) || (pos_balle.x-pos_joueur1.x==LONGUEUR_JOUEUR-1)) && ((pos_joueur1.y-pos_balle.y<HAUTEUR_BALLE) || (pos_joueur1.y-pos_balle.y<HAUTEUR_BALLE-1))
                                     && ((pos_balle.y-pos_joueur1.y<HAUTEUR_JOUEUR)|| (pos_balle.y-pos_joueur1.y<HAUTEUR_JOUEUR-1)))))
        {
            vitesse_balle.x=1;
        }


        if ((vitesse_balle.y==1)&&((pos_balle.y==(LARGEUR_TERRAIN-TAILLE_BALLE))|| (pos_balle.y==(LARGEUR_TERRAIN-TAILLE_BALLE-1))||
                                   (((pos_joueur2.y-pos_balle.y==HAUTEUR_BALLE) || (pos_joueur2.y-pos_balle.y==HAUTEUR_BALLE-1)) && ((pos_joueur2.x-pos_balle.x<LONGUEUR_BALLE) || (pos_joueur2.x-pos_balle.x<LONGUEUR_BALLE-1))
                                    && ((pos_balle.x-pos_joueur2.x<LONGUEUR_JOUEUR) || (pos_balle.x-pos_joueur2.x<LONGUEUR_JOUEUR-1))) ||
                                   (((pos_joueur1.y-pos_balle.y==HAUTEUR_BALLE) || (pos_joueur1.y-pos_balle.y==HAUTEUR_BALLE-1)) && ((pos_joueur1.x-pos_balle.x<LONGUEUR_BALLE) || (pos_joueur1.x-pos_balle.x<LONGUEUR_BALLE-1))
                                    && ((pos_balle.x-pos_joueur1.x<LONGUEUR_JOUEUR) || (pos_balle.x-pos_joueur1.x<LONGUEUR_JOUEUR-1)))))
        {
            vitesse_balle.y=-1;
        }


        if ((vitesse_balle.y==-1)&&((pos_balle.y==0)|| (pos_balle.y==-1)||
                                    (((pos_balle.y-pos_joueur2.y==HAUTEUR_JOUEUR) || (pos_balle.y-pos_joueur2.y==HAUTEUR_JOUEUR-1)) && ((pos_joueur2.x-pos_balle.x<LONGUEUR_BALLE)  || (pos_joueur2.x-pos_balle.x<LONGUEUR_BALLE-1))
                                     && ((pos_balle.x-pos_joueur2.x<LONGUEUR_JOUEUR) || (pos_balle.x-pos_joueur2.x<LONGUEUR_JOUEUR-1)))||
                                    (((pos_balle.y-pos_joueur1.y==HAUTEUR_JOUEUR) || (pos_balle.y-pos_joueur1.y==HAUTEUR_JOUEUR-1)) && ((pos_joueur1.x-pos_balle.x<LONGUEUR_BALLE) || (pos_joueur1.x-pos_balle.x<LONGUEUR_BALLE-1))
                                     && ((pos_balle.x-pos_joueur1.x<LONGUEUR_JOUEUR) || (pos_balle.x-pos_joueur1.x<LONGUEUR_JOUEUR-1)))))
        {
            vitesse_balle.y=1;
        }



        //Réalisation de l'arrê de la balle en cas de but et attribution de la victoire à un joueur
        if((pos_balle.x<4) && (pos_balle.y>=(LARGEUR_TERRAIN-LARGEUR_BUT)/2) && (pos_balle.y<=(LARGEUR_TERRAIN+LARGEUR_BUT)/2))
        {
            vitesse_balle.x=0;
            vitesse_balle.y=0;
            victoire=2;
        }



        if((pos_balle.x+LONGUEUR_BALLE>LONGUEUR_TERRAIN-4) && (pos_balle.y>=(LARGEUR_TERRAIN-LARGEUR_BUT+HAUTEUR_BALLE)/2) && (pos_balle.y<=(LARGEUR_TERRAIN+LARGEUR_BUT-HAUTEUR_BALLE)/2))
        {
            vitesse_balle.x=0;
            vitesse_balle.y=0;
            victoire=1;
        }



        //Déplacement de la balle
        pos_balle.x+=vitesse_balle.x;
        pos_balle.y+=vitesse_balle.y;



        //Déplacement du joueur sans chevauchement avec l'autre joueur ou la balle
        //joueur1
        if (cmd_joueur1.y_moins==1 &&
                ((pos_joueur1.y-pos_joueur2.y!=HAUTEUR_JOUEUR) || (abs(pos_joueur2.x-pos_joueur1.x)>LONGUEUR_JOUEUR))&&
                ((pos_joueur1.y-pos_balle.y!=HAUTEUR_BALLE) || (pos_joueur1.x-pos_balle.x>LONGUEUR_BALLE) || (pos_balle.x-pos_joueur1.x>LONGUEUR_JOUEUR)))
            pos_joueur1.y-=vitesse_joueur1.y;


        if (cmd_joueur1.y_plus==1&&
                ((pos_joueur2.y-pos_joueur1.y!=HAUTEUR_JOUEUR) || (abs(pos_joueur2.x-pos_joueur1.x)>LONGUEUR_JOUEUR))&&
                ((pos_balle.y-pos_joueur1.y!=HAUTEUR_JOUEUR) || (pos_joueur1.x-pos_balle.x>LONGUEUR_BALLE) || (pos_balle.x-pos_joueur1.x>LONGUEUR_JOUEUR)) &&
                pos_joueur1.y<LARGEUR_TERRAIN-HAUTEUR_JOUEUR)
            pos_joueur1.y+=vitesse_joueur1.y;


        if (cmd_joueur1.x_moins==1&&
                ((pos_joueur1.x-pos_joueur2.x!=LONGUEUR_JOUEUR) || (abs(pos_joueur2.y-pos_joueur1.y)>HAUTEUR_JOUEUR))&&
                ((pos_joueur1.x-pos_balle.x!=LONGUEUR_BALLE) || (pos_joueur1.y-pos_balle.y>HAUTEUR_BALLE) || (pos_balle.y-pos_joueur1.y>HAUTEUR_JOUEUR)))
            pos_joueur1.x-=vitesse_joueur1.x;


        if (cmd_joueur1.x_plus==1&&
                ((pos_joueur2.x-pos_joueur1.x!=LONGUEUR_JOUEUR) || (abs(pos_joueur2.y-pos_joueur1.y)>HAUTEUR_JOUEUR))&&
                ((pos_balle.x-pos_joueur1.x!=LONGUEUR_JOUEUR) || (pos_joueur1.y-pos_balle.y>HAUTEUR_BALLE) || (pos_balle.y-pos_joueur1.y>HAUTEUR_JOUEUR)) &&
                pos_joueur1.x<LONGUEUR_TERRAIN-LONGUEUR_JOUEUR)
            pos_joueur1.x+=vitesse_joueur1.x;



        //joueur2
        if (cmd_joueur2.y_moins==1 &&
                ((pos_joueur2.y-pos_joueur1.y!=HAUTEUR_JOUEUR) || (abs(pos_joueur2.x-pos_joueur1.x)>LONGUEUR_JOUEUR))&&
                ((pos_joueur2.y-pos_balle.y!=HAUTEUR_BALLE) || (pos_joueur2.x-pos_balle.x>LONGUEUR_BALLE) || (pos_balle.x-pos_joueur2.x>LONGUEUR_JOUEUR)))
        {
            pos_joueur2.y-=vitesse_joueur2.y;
        }


        if (cmd_joueur2.y_plus==1 &&
                ((pos_joueur1.y-pos_joueur2.y!=HAUTEUR_JOUEUR) || (abs(pos_joueur2.x-pos_joueur1.x)>LONGUEUR_JOUEUR))&&
                ((pos_balle.y-pos_joueur2.y!=HAUTEUR_JOUEUR) || (pos_joueur2.x-pos_balle.x>LONGUEUR_BALLE) || (pos_balle.x-pos_joueur2.x>LONGUEUR_JOUEUR)) &&
                pos_joueur2.y<LARGEUR_TERRAIN-HAUTEUR_JOUEUR)
            pos_joueur2.y+=vitesse_joueur2.y;


        if (cmd_joueur2.x_moins==1 &&
                ((pos_joueur2.x-pos_joueur1.x!=LONGUEUR_JOUEUR) || (abs(pos_joueur2.y-pos_joueur1.y)>HAUTEUR_JOUEUR))&&
                ((pos_joueur2.x-pos_balle.x!=LONGUEUR_BALLE) || (pos_joueur2.y-pos_balle.y>HAUTEUR_BALLE) || (pos_balle.y-pos_joueur2.y>HAUTEUR_JOUEUR)))
            pos_joueur2.x-=vitesse_joueur2.x;


        if (cmd_joueur2.x_plus==1 &&
                ((pos_joueur1.x-pos_joueur2.x!=LONGUEUR_JOUEUR) || (abs(pos_joueur2.y-pos_joueur1.y)>HAUTEUR_JOUEUR))&&
                ((pos_balle.x-pos_joueur2.x!=LONGUEUR_JOUEUR) || (pos_joueur2.y-pos_balle.y>HAUTEUR_BALLE) || (pos_balle.y-pos_joueur2.y>HAUTEUR_JOUEUR))&&
                pos_joueur2.x<LONGUEUR_TERRAIN-LONGUEUR_JOUEUR)
            pos_joueur2.x+=vitesse_joueur2.x;



        // DRAWING STARTS HERE

        // clear screen

        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 100, 0));
        SDL_FillRect(but1,0,SDL_MapRGB(screen->format,255,255,255));
        SDL_BlitSurface(but1,NULL,screen,&pos_but1);
        SDL_FillRect(but2,0,SDL_MapRGB(screen->format,255,255,255));
        SDL_BlitSurface(but2,NULL,screen,&pos_but2);

        //draw bitmap
        SDL_BlitSurface(balle_bmp,0,screen,&pos_balle);
        SDL_BlitSurface(joueur1_bmp,0,screen,&pos_joueur1);
        SDL_BlitSurface(joueur2_bmp,0,screen,&pos_joueur2);
        //DRAWING ENDS HERE



        //Cas de victoire
        if (vitesse_balle.x==0)
        {
            if (victoire==1) //si joueur1 gagne
            {
                SDL_BlitSurface(victoire1_bmp,0,screen,&pos_vict1);
            }
            if (victoire==2)//si joueur2 gagne
            {
                SDL_BlitSurface(victoire2_bmp,0,screen,&pos_vict2);
            }
        }


        //finally, update the screen :)
        SDL_Flip(screen);

    } // end main loop



    //free loaded bitmap
    SDL_FreeSurface(balle_bmp);
    SDL_FreeSurface(joueur1_bmp);
    SDL_FreeSurface(joueur2_bmp);
    SDL_FreeSurface(but1);
    SDL_FreeSurface(but2);



    //all is well ;)
    printf("Exited cleanly\n");
    CloseCOM();
    return 0;
}


/******************************************************************************
  OpenCOM : ouverture et configuration du port COM.
  entrée : nId : Id du port COM à ouvrir.
  retour : vrai si l'opération a réussi, faux sinon.
******************************************************************************/

BOOL OpenCOM1(int nId)
{
    /* variables locales */
    char szCOM[16];

    /* construction du nom du port, tentative d'ouverture */
    sprintf(szCOM, "\\\\.\\COM%d", nId);

    g_hCOM1 =CreateFile(
      szCOM,     // address of name of the communications device
      GENERIC_READ|GENERIC_WRITE,          // access (read-write) mode
      0,                  // share mode
      NULL,               // address of security descriptor
      OPEN_EXISTING,      // how to create
      FILE_ATTRIBUTE_SYSTEM,                  // file attributes
      NULL                // handle of file with attributes to copy
   );

    if(g_hCOM1 == INVALID_HANDLE_VALUE)
    {

        printf("Erreur lors de l'ouverture du port COM%d", nId);
        return FALSE;
    }

    /* affectation taille des tampons d'émission et de réception */
    SetupComm(g_hCOM1, RX_SIZE, TX_SIZE);
    //for(int k=0;k<100000000;k++){};
    /* configuration du port COM */
    if(!SetCommTimeouts(g_hCOM1, &g_cto) || !SetCommState(g_hCOM1, &g_dcb))
    {
        printf("Erreur lors de la configuration du port COM%d", nId);
        CloseHandle(g_hCOM1);
        return FALSE;
    }

    /* on vide les tampons d'émission et de réception, mise à 1 DTR */

    PurgeComm(g_hCOM1, PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_RXABORT);
       // for(int l=0;l<1000000000;l++){};
    EscapeCommFunction(g_hCOM1, SETDTR);
        //for(int m=0;m<1000000000;m++){};
    return TRUE;
}

BOOL OpenCOM2(int nId2)
{
    /* variables locales */
    char szCOM[16];

    /* construction du nom du port, tentative d'ouverture */
    sprintf(szCOM, "\\\\.\\COM%d", nId2);

    g_hCOM2 =CreateFile(
      szCOM,     // address of name of the communications device
      GENERIC_READ|GENERIC_WRITE,          // access (read-write) mode
      0,                  // share mode
      NULL,               // address of security descriptor
      OPEN_EXISTING,      // how to create
      FILE_ATTRIBUTE_SYSTEM,                  // file attributes
      NULL                // handle of file with attributes to copy
   );

    //for(int o=0;o<10000000000;o++){};
    if(g_hCOM2 == INVALID_HANDLE_VALUE)
    {
        printf("Erreur lors de l'ouverture du port COM%d", nId2);
        return FALSE;
    }

    /* affectation taille des tampons d'émission et de réception */
    SetupComm(g_hCOM2, RX_SIZE, TX_SIZE);
    //for(int p=0;p<100000000;p++){};
    /* configuration du port COM */
    if(!SetCommTimeouts(g_hCOM2, &g_cto) || !SetCommState(g_hCOM2, &g_dcb))
    {
        printf("Erreur lors de la configuration du port COM%d", nId2);
        CloseHandle(g_hCOM2);
        return FALSE;
    }

    /* on vide les tampons d'émission et de réception, mise à 1 DTR */
    PurgeComm(g_hCOM2, PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_RXABORT);
       // for(int q=0;q<1000000000;q++){};
    EscapeCommFunction(g_hCOM2, SETDTR);
      //  for(int r=0;r<1000000000;r++){};
    return TRUE;
}

/******************************************************************************
  CloseCOM : fermeture du port COM.
  retour : vrai si l'opération a réussi, faux sinon.
******************************************************************************/
BOOL CloseCOM()
{
    /* fermeture du port COM */
    CloseHandle(g_hCOM1);
    CloseHandle(g_hCOM2);
    return TRUE;
}

/******************************************************************************
  ReadCOM : lecture de données sur le port COM.
  entrée : buffer       : buffer où mettre les données lues.
           nBytesToRead : nombre max d'octets à lire.
           pBytesRead   : variable qui va recevoir le nombre d'octets lus.
  retour : vrai si l'opération a réussi, faux sinon.
-------------------------------------------------------------------------------
  Remarques : - la constante MAX_WAIT_READ utilisée dans la structure
                COMMTIMEOUTS permet de limiter le temps d'attente si aucun
                caractères n'est présent dans le tampon d'entrée.
              - la fonction peut donc retourner vrai sans avoir lu de données.
******************************************************************************/
BOOL ReadCOM1(void* buffer1, int nBytesToRead1, int* pBytesRead1)
{
    return ReadFile(g_hCOM1, buffer1, nBytesToRead1, (PDWORD)pBytesRead1, NULL);
}

BOOL ReadCOM2(void* buffer2, int nBytesToRead2, int* pBytesRead2)
{
    return ReadFile(g_hCOM2, buffer2, nBytesToRead2, (PDWORD)pBytesRead2, NULL);
}


