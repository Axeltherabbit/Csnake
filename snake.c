#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <locale.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

/* UNICODE blocks for block elements.
   See https://en.wikipedia.org/wiki/Block_Elements */
#define UPPER_BLOCK "\u2580"
#define LOWER_BLOCK "\u2584"
#define FULL_BLOCK "\u2588"

struct snake
{
	int posx;
	int posy;
	struct snake* next;
        struct snake* back;
};
typedef struct snake* SNAKE;

struct posfood
{
  int posx;
  int posy;
};
typedef struct posfood* FOODP;

void *getkey(void *p);
void draw(SNAKE p, int *add, FOODP food, Mix_Chunk * snd);
void movepieces(SNAKE p, int add);
void newfoodpos(FOODP);
int collision(SNAKE p);
int snakelen(SNAKE p);

int stop = 0; //stop game variable
int waitchar=1;
int row,col;
int main()
{
  //init SDL sound mixer
  SDL_Init(SDL_INIT_AUDIO);

  Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048);

  Mix_Music * music = Mix_LoadMUS("music.wav"); //background music
  Mix_Chunk * snd = Mix_LoadWAV("sound.wav"); //eat sound

  Mix_PlayMusic(music,-1);

  setlocale(LC_ALL, "");

  initscr(); /* Start curses mode */
  getmaxyx(stdscr,row,col);
  row *= 2;

  struct posfood snake_food;
  FOODP food = &snake_food;
  newfoodpos(food);

  //snake head
  SNAKE head=malloc(sizeof(struct snake));
  head -> posx=row/2;
  head -> posy=col/2;
  head -> next = NULL;
  head -> back = NULL;

  start_color(); /* Start color*/

  //background pair
  init_pair(1, COLOR_BLACK, COLOR_BLACK);
  //snake and background pair
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  //food and background pair
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);

  bkgd(COLOR_PAIR(1));

  keypad(stdscr, TRUE);  /* abilita la mappatura della tastiera */
  noecho(); /* Don't echo() while we do getch */

  int add = 0;
  int direction='w';

  draw(head, &add, food, snd);

  //keyboard and music threads
  pthread_t readkey, playsound;

  int alive = 1;
  while(alive)
     {
      movepieces(head,add);

      switch (direction)
	{
	  case 'w': case KEY_UP:
	      head -> posx -= 1;
	      break;
	  case 'a': case KEY_LEFT:
              head -> posy -= 1;
	      break;
	  case 'd': case KEY_RIGHT:
	      head -> posy += 1;
	      break;
	  case 's': case KEY_DOWN:
	      head -> posx += 1;
	      break;
	}

     draw(head, &add, food, snd);

     refresh();
     if (waitchar) pthread_create(&readkey,NULL, getkey, &direction);
     alive = collision(head);
     if (stop)
	{
	 mvprintw(0,0,"press SPACE to resume the game");
 	 char c='a';
	 while(c != ' ') c = getch();
	 stop = 0;
	}
     usleep(1000/10*1000);
     }

  //snake color
  init_pair(2, COLOR_RED, COLOR_BLACK);

  draw(head, &add, food, snd);
  getch();
  endwin();
  Mix_FreeMusic(music);
  Mix_FreeChunk(snd);
  Mix_CloseAudio();
  printf("\n\nscreen size %d X %d\n snake : %d\n\n",row,col,snakelen(head));

}


void *getkey(void *p)
{
   waitchar = 0;
   int c = getch();

   int change = 1;
   switch (*(int *) p)
   {
     case 'w': case KEY_UP:
	  if (c == 's' || c ==  KEY_DOWN) change = 0;
	  break;
     case 's': case KEY_DOWN:
	  if (c == 'w' || c ==  KEY_UP) change = 0;
	  break;
     case 'a': case KEY_LEFT:
	  if (c == 'd' || c ==  KEY_RIGHT) change = 0;
	  break;
     case 'd': case KEY_RIGHT:
	  if (c == 'a' || c == KEY_LEFT) change = 0;
	  break;
   }

   if (change)
   	switch (c)
    	{
		case 'w': case 'a': case 's': case 'd':
		case  KEY_LEFT: case  KEY_RIGHT:  case KEY_UP: case  KEY_DOWN:
		  *(int *) p = c;
		  break;

		case ' ': //pause button
		  stop = 1;
		  break;
    	}
  waitchar = 1;

}

void movepieces(SNAKE p, int add)
{
  int oldposx, oldposy;
  SNAKE piece = p;
  SNAKE start = p;

  while(p -> next != NULL) p = p -> next;
  if (add)
  {
    oldposx = p -> posx;
    oldposy = p -> posy;
  }
  while(p -> back != NULL)
   {
     p -> posx = p -> back -> posx;
     p -> posy = p -> back -> posy;
     p = p -> back;
   }

  if (add)
  { //create a new piece
    while (p -> next != NULL) p = p -> next;
    p -> next = malloc(sizeof(struct snake));
    p -> next -> next = NULL;
    p -> next -> back = p;
    p -> next -> posx = oldposx;
    p -> next -> posy = oldposy;
  }

}

void newfoodpos(FOODP food)
{
 food -> posy = rand() % col;
 food -> posx = rand() % row;
}

void draw(SNAKE p, int *add, FOODP food, Mix_Chunk * snd)
{
  if (p -> posx == food -> posx && p -> posy == food -> posy)
    {
      Mix_PlayChannel(-1,snd,0);
      newfoodpos(food);
      *add = 1;
    }
  else *add = 0;

  /* clear window */
  attron(COLOR_PAIR(1));
  clear();

  /* draw snake */
  attron(COLOR_PAIR(2));
  while (p != NULL)
  {
    if (p->next != NULL && (p->posx&3 ^ p->next->posx&3) == 1) {
      mvprintw(p->posx>>1, p->posy, FULL_BLOCK);
      p = p->next;
    }
    else {
      mvprintw(p->posx>>1, p->posy, p->posx&1 ? LOWER_BLOCK : UPPER_BLOCK);
    }
     p = p->next;
  }

  /* draw food */
  attron(COLOR_PAIR(3));
  mvprintw(food->posx>>1, food->posy, food->posx&1 ? LOWER_BLOCK : UPPER_BLOCK);

  /* move cursor to the corner, to avoid interference with already drawn characters*/
  move(row/2-1, col-1);
}

int collision(SNAKE p)
{
   int headposx = p -> posx;
   int headposy = p -> posy;
   int rt = 1;
   if (headposx > row-1 || headposx < 0 || headposy > col-1 || headposy < 0)
      rt = 0;
   while (p -> next != NULL && rt)
	{
	  p = p -> next;
	  if (p -> posx == headposx && p -> posy == headposy)
		rt = 0;

	}
   return rt;

}

int snakelen(SNAKE p)
{
   int count = 1;
   while (p -> next != NULL)
   {
	   count++;
	   p = p-> next;
   }

   return count;

}
