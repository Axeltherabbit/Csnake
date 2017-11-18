#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <term.h>
#include <locale.h>


#define termclean() printf("\033[H\033[J")
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
void *snd();
void *eatsnd();
void draw(SNAKE p, int *add, FOODP food);
void movepieces(SNAKE p, int add);
void newfoodpos(FOODP);
int collision(SNAKE p);
int snakelen(SNAKE p);

int waitchar=1;
int row,col;
int main()
{
  
  setlocale(LC_ALL, "");

  /* One-time initialization near the beginning of your program */
  setupterm(NULL, STDOUT_FILENO, NULL);

  /* Enter bold mode */
  putp(enter_bold_mode);

  initscr(); /* Start curses mode */
  getmaxyx(stdscr,row,col);
  
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
  
  //background color
  init_pair(1, COLOR_BLACK, COLOR_BLACK);
  //snake color
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  //food color
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);

  bkgd(COLOR_PAIR(1));
 
  keypad(stdscr, TRUE);  /* abilita la mappatura della tastiera */
  noecho(); /* Don't echo() while we do getch */
  

  int add = 0;
  char direction='w';
    
  draw(head, &add, food); 
  
  //keyboard and music threads
  pthread_t readkey, playsound;
  pthread_create(&playsound, NULL, snd, NULL);
  
  int alive = 1;
  while(alive)
     {	  
	  
      movepieces(head,add);
	
      switch (direction)
	{
	  case 'w':
	      head -> posx -= 1;
	      break;
	  case 'a':
              head -> posy -= 1;
	      break;
	  case 'd':
	      head -> posy += 1;
	      break;
	  case 's':
	      head -> posx += 1;
	      break;
    	      		  
	}
     
     draw(head, &add, food);

     refresh();
     if (waitchar) pthread_create(&readkey,NULL, getkey, &direction);
     alive = collision(head);
     usleep(1000/10*1000); 
     
     }

  //snake color
  init_pair(2, COLOR_RED, COLOR_BLACK);
  
  draw(head, &add, food);
  getch();
  endwin();  
  system("killall aplay");  
  printf("\n\nscreen size %d X %d\n snake : %d\n\n",row,col,snakelen(head));
  /* Turn it off! */
  putp(exit_attribute_mode);	
  
}

void*snd()
{
   while (1) system("aplay music.wav >/dev/null 2>&1");
}
void*eatsnd(){system("aplay suono.wav >/dev/null 2>&1");}

void *getkey(void *p) 
{   
   waitchar = 0;
   char c = getch();

   int change = 1;
   switch (*(char *) p)
   {
     case 'w':
	  if (c == 's') change = 0;
	  break;
     case 's':
	  if (c == 'w') change = 0;
	  break;
     case 'a':
	  if (c == 'd') change = 0;
	  break;
     case 'd':
	  if (c == 'a') change = 0;
	  break;
   } 

   if (change)
   	switch (c) 
    	{ 
     		case 'w': case 'a': case 's': case 'd':
       		*(char *) p = c;
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


void draw(SNAKE p, int *add, FOODP food)
	{ 
	  if (p -> posx == food -> posx && p -> posy == food -> posy) 
		{
	  pthread_t eatsound;
	  pthread_create(&eatsound, NULL, eatsnd, NULL);
	  newfoodpos(food);
	  *add = 1;
	}
  else *add = 0;
  
  attron(COLOR_PAIR(1)); //BACKGROUND COLOR
  clear(); //clear window	
  attron(COLOR_PAIR(2)); //SNAKE COLOR
  while (p -> next != NULL)
  {
     mvprintw(p -> posx, p -> posy,"■");
     p = p -> next;
  }
  mvprintw(p -> posx, p -> posy,"■");
  
  attron(COLOR_PAIR(3)); //FOOD COLOR
  mvprintw(food -> posx, food -> posy,"■");
  move(row+1,col+1); //hide cursor
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
