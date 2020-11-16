#include <iostream>
#include <stdlib.h>
#include <allegro.h>

using namespace std;
int  vert[8]={-2,-2,-1,+1,+2,+2,+1,-1};
int horiz[8]={-1,+1,+2,+2,+1,-1,-2,-2};
int time_step=200;

class Tdata
{
    public:
    int n;
    int timer_mode;
    int ile;
    int zapis;
    int dump_data;
    int backtrace;
    int update;
};

class Tboard
{
    public:
//konstruktor
        Tboard(int n);
        ~Tboard(){

            delete[] pola;
            delete[] stos_x;
            delete[] stos_y;
            delete[] stos_k;
            delete[] free_moves;
        };
		void chess();
		unsigned long int moves;
	private:
	    //wykonanie pojedynczego ruchu, ewentualne wypisanie wyniku do pliku
	    int move(int x,int y,int i);
	    //sprawdza czy na danym polu skoczek ju¿ stawa³
	    int check(long int x,long int y);
	    int check2(long int,long int);
	    unsigned long int arraypos(int x,int y);
	    int push(int,int,int);
		int pop();
		int assert(int,int,int);
		void zapisz_stan();
		int*stos_x;
		int*stos_y;
		short int*stos_k;
		unsigned long int*pola;
		unsigned short int*free_moves;
		int forced_move_x,forced_move_y;
		unsigned long int backtrace;
		int forced_num;
		int top;
		int last_x,last_y;
		unsigned long int closed_paths,opened_paths;
		//int a[7];//mozliwe ruchy w drugim ruchu
		unsigned long int size;
};


FILE*zapis;
Tboard*board;
Tdata data;
unsigned long int time_sec;
unsigned long int timer;
//int debug=0;

/******************************************************
 * Tboard::Tboard(int n)                              *
 * konstruktor                                        *
 * n - rozmiar inicjowanej szachownicy(n*n)      *
 *                                                    *
 * Inicjuje szachownice, zmienne oraz bitmape         *
 ******************************************************/
Tboard::Tboard(int n){
            stos_x=new int[n*n];
            stos_y=new int[n*n];
            stos_k=new short int[n*n];
            pola=new unsigned long int[n*n];
            free_moves=new unsigned short int[n*n];
            int i,j,k;
            for (i=0;i<n*n-1;i++){/*stos[i]=0,*/pola[i]=0;};
            backtrace=forced_move_x=forced_move_y=moves=closed_paths=opened_paths=top=0;
            size=n;
            for(i=1;i<=size;i++)
            for(j=1;j<=size;j++){
                free_moves[(i-1)*size+j-1]=0;
                for(k=0;k<8;k++){
                    if(check(i+vert[k],j+horiz[k])==0)free_moves[(i-1)*size+j-1]++;
                };
            };
            free_moves[(size-3)*size+2-1]++;
            free_moves[(size-2)*size+3-1]++;
            //DEP_BUFFER=0;
        };

inline unsigned long int Tboard::arraypos(int x,int y){
    return (x-1)*size+y-1;
};

/******************************************************
 * Tboard::check2(int x,int y)                        *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 *                                                    *
 * Funckja zwraca ile jest mo¿liwych róchów z danego  *
 * pola szachownicy.                                  *
 ******************************************************/
inline int Tboard::check2(long int x,long int y){
    if((x>0)&&(x<=size)&&(y>0)&&(y<=size))
    return free_moves[arraypos(x,y)];
    else
    return -1;
};

/******************************************************
 * Tboard::check(int x,int y)                         *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 *                                                    *
 * Funckja sprawdza czy skoczek stawa³ ju¿ na danym   *
 * polu szachownicy, i je¿eli tak to kiedy, a je¿eli  *
 * nie to zwraca 0.                                   *
 ******************************************************/
inline int Tboard::check(long int x,long int y){
    if((x>0)&&(x<=size)&&(y>0)&&(y<=size))
    return pola[arraypos(x,y)];
    else
    return -1;
};

/******************************************************
 * Tboard::push(int x,int y,int i)                    *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 * i - numer ruchu                                    *
 *                                                    *
 * Funkcja dopisuje do aktualnego stanuy szachownicy  *
 * kolejny ruch na pole (x,y).                        *
 ******************************************************/
int Tboard::push(int x,int y,int i){
    int ret=1;
    if((x==size-1)&&(y==3)&&(top+1!=size*size));else
    if(check(x,y)==0)
    {
        moves++;
        top++;
        int forced=0;
        stos_x[top]=x;stos_y[top]=y;stos_k[top]=i;
        pola[arraypos(x,y)]=top;
        
        forced_move_x=forced_move_y=0;
        for(int k=0;k<8;k++){
         if(check2(x+vert[k],y+horiz[k])>0){
             --free_moves[arraypos(x+vert[k],y+horiz[k])];
             if(check2(x+vert[k],y+horiz[k])==1&&check(x+vert[k],y+horiz[k])==0){
             	forced++;forced_move_x=x+vert[k];forced_move_y=y+horiz[k];
           	 };
       	 };
        };
        if(forced==1){stos_k[top]=6;};
        if(forced>1){ret=0;};
        return ret;
    };
};

/******************************************************
 * Tboard::pop(int x,int y)                           *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 *                                                    *
 * Funkcja cofa skoczka o jeden ruch z pola (x,y).    *
 ******************************************************/
int Tboard::pop(){
    if(top>0){
    pola[arraypos(stos_x[top],stos_y[top])]=0;
    forced_move_x=forced_move_y=0;

    for(int k=0;k<8;k++){
    	if(check2(stos_x[top]+vert[k],stos_y[top]+horiz[k])>=0)
          free_moves[arraypos(stos_x[top]+vert[k],stos_y[top]+horiz[k])]++;
   	};

    //if(data.grafika)
    //if(data.szachownica==1)
   	//rect(screen,x_s*(stos_x[top]),    y_s*(stos_y[top]),
    //            x_s*(stos_x[top])+x_s-1,y_s*(stos_y[top])+y_s-1,black);
   	stos_x[top]=0;
   	stos_y[top]=0;
    top--;
    int wart=1;
    int forced=0;

    for(int k=0;k<8;k++){
    	if(check2(stos_x[top]+vert[k],stos_y[top]+horiz[k])==1&&
           check (stos_x[top]+vert[k],stos_y[top]+horiz[k])==0){
          	forced++;forced_move_x=stos_x[top]+vert[k];forced_move_y=stos_y[top]+horiz[k];
   	    };
    };
    clear_keybuf();
    if(forced>1)wart=0;
    return wart;
};
};

void timer_handle()
{
    time_sec++;
    timer+=time_step;
    printf("wykonano %i ruchów.\r",board->moves);
};

inline int Tboard::assert(int ret,int x,int y)
{
    if(x==size-1&&y==3&&pola[size+2]!=0&&pola[size+2]!=(size*size))return false;else
    if(ret)return false;else
    return true;
};

void Tboard::zapisz_stan()
{
    if(data.zapis)
    fprintf(zapis,"rozwi¹zanie znaleziono po %i ruchach, w czasie %i ms\n",moves,timer);
    for(int x=1;x<=size;x++){
     fprintf(zapis,"|");
     for(int y=1;y<=size;y++)fprintf(zapis,"%3u ",check(x,y));
     fprintf(zapis,"|\n");
    };
};

int Tboard::move(int x,int y,int i)
{
    int ret=1;
    while(top<(size*size-1)&&(key[KEY_ESC]==0)){
        if((stos_k[top]<8)&&ret){
         if(forced_move_x)
          ret=push(forced_move_x,forced_move_y,-1);
         else
          ret=push(stos_x[top]+vert[stos_k[top]],stos_y[top]+horiz[stos_k[top]],-1);
        }else{
          ret=pop();
        };
        if(time_sec*time_step>data.backtrace){while(top>size*size*1/4)pop(); time_sec=0;};
        if(ret==0){ret=pop();};
        stos_k[top]++;
    };
    zapisz_stan();
};

void Tboard::chess()
{
    moves=0;
	push(size,1,0);
	move(1,1,1);
};

void init()
{
    if(install_keyboard()!=0){
        alert("Nie uda³o siê zainicjowaæ klawiatury",NULL,NULL,"OK",NULL,1,2);
    };
    if(data.zapis)zapis=fopen("knight.txt","w+");
    if(install_timer()!=0){
        alert("Nie uda³o siê zainicjowaæ timer'a",NULL,NULL,"OK",NULL,1,2);
    };
    time_step=data.update;
};

void load_data(){
    set_config_file("config.cfg");
    data.n=get_config_int(NULL,"n",8);
    data.timer_mode=get_config_int("grafika","timer",0);
    data.zapis=get_config_int("zapis","zapis",1);
    data.dump_data=get_config_int("debug","dump_data",0);
    data.backtrace=get_config_int(NULL,"backtrace_time",500);
    data.update=get_config_int("grafika","update",50);
};

void close()
{
    fclose(zapis);
};


int main(int argc, char *argv[])
{
  allegro_init();
  load_data();
  init();
  board=new Tboard(data.n);
  if(data.timer_mode)
   install_int(timer_handle,time_step);
  time_sec=0;
  board->chess();
  if(data.timer_mode)
   remove_int(timer_handle);
  close();
  //system("PAUSE");
  return 0;
}


      END_OF_MAIN()
