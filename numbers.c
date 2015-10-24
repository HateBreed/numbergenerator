#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define COUNTMAX 10

#define KENO 70+1
#define LOTTO 39+1
#define VIKING 48+1
#define EUROJACK 50+1

int simlimit = -1;
int skiplimiter = 30;
int printtype = 0;

void sort(int *, int, int);
void merge(int *, int, int, int);

typedef struct numberlist {
	int numbers[COUNTMAX];
	struct numberlist* next;
} nlist;

void sort(int *array, int start, int end)
{
	if(start < end) {
		int middle = (start + end) / 2;
		// left
		sort(array, start, middle);
		// right
		sort(array, middle + 1, end);
		// combine
		merge(array, start, middle, end);
	}
}

void merge(int *array, int start, int middle, int end)
{
	int i = start, j = middle + 1, k = start;
	int arrayTemp[end+1];

	/* Tehdään kunnes rajat (puoliväli/loppu) tulee vastaan */
	while((i <= middle)||(j <= end))
	{
		/* Vasen puoli väliaikaiseen listaan */
		if(i > middle) {
			arrayTemp[k] = array[j];
			j++;
		}
		/* Oikea puoli väliaikaiseen listaan */
		else if(j > end) {
			arrayTemp[k] = array[i];
			i++;
		}
		/* Valitaan pienempi numero väliaikaiseen listaan */
		else if(array[i] < array[j]) {
			arrayTemp[k] = array[i];
			i++;
		}
		else {
			arrayTemp[k] = array[j];
			j++;
		}
		k++;
	}
	/* Kopioidaan numerot väliaikaisesta listasta */
	for(k = start; k <= end; k++) array[k] = arrayTemp[k];
}

void printarray(int *numbers,int count,int type) {
	for(int index = 0; index < count; index++) {
		if(type == EUROJACK) {
			if (index < count-2) {
				if(printtype == 1) fprintf(stderr,"%d%s",numbers[index], index+1 == count-2 ? " " : ",");
				else printw("%d%s",numbers[index], index+1 == count-2 ? " " : ",");
			}
			else {
				if(printtype == 1) fprintf(stderr,"%d%s",numbers[index], index+1 == count ? "\n" : ",");
				else printw("%d%s",numbers[index], index+1 == count ? "\n" : ",");
			}
		}
		else {
			if(printtype == 1) fprintf(stderr,"%d%s",numbers[index], index+1 == count ? "\n" : ",");
			else printw("%d%s",numbers[index], index+1 == count ? "\n" : ",");
		}
	}
}

int contains(int *numbers,int count,int value) {
	for(int j = 0; j < count ; j++) if(numbers[j] == value)	return 1;
	return 0;
}

void zeroarray(int *numbers,int count) {
	for(int x = 0; x < count;x++) numbers[x]=0;
}

nlist* initlist() {
	nlist* new = (nlist*)malloc(sizeof(nlist));
	if(new) {
		memset(&new->numbers,0,COUNTMAX);
		new->next = NULL;
	}
	return new;
}

void setlistcontent(nlist* current, int *numbers, int count) {
	if(current && numbers) {
		memcpy(&current->numbers,numbers,count*sizeof(int));
	}
}

nlist* addlist(nlist* current, int *numbers, int count) {
	if(current) {
		nlist* new = initlist();
		setlistcontent(new,numbers,count);
		current->next = new;
		return new;
	}
	else {
		current = initlist();
		setlistcontent(current,numbers,count);
		return current;
	}
}

void printlist(nlist* first,int count, int type) {
	for(nlist* iter = first; iter != NULL; iter = iter->next) printarray(iter->numbers,count,type);
}

void screenupdate(nlist* first,int count, int type, int* numbers, int countn, int typen, int skipcount) {
	clear();
	printw("- [%3.d] [%2.d] ----------------\n",skipcount,simlimit);
	printlist(first,count,type);
	printw("-----------------------------\n");
	printw("Press 'y' to print to stdout instead of ncurses screen");
	if(numbers) {
		printw("Skip #%d caused by: ",skipcount);
		printarray(numbers,countn,typen);
	}
	refresh();
}

int checksimilarity(nlist* first, int* numbers, int count) {
	int similarity = 0,rows = 0, simtotal = 0;
	//printf("checking line: ");
	//printarray(numbers,count);
	for(nlist* iter = first; iter != NULL; iter = iter->next) {
		int rowsim = 0;
		for(int i = 0; i < count; i++) {
			for(int j = 0; j < count; j++) if(numbers[j] == iter->numbers[i]) rowsim++;
		}
		if(rowsim > similarity) similarity = rowsim;
		//printf("similarity = %d|",rowsim);
		//printarray(iter->numbers,count);
		rows++;
		simtotal += rowsim;
	}
	//printf("average similarity: %1.2f\nhighest: %d\n",(float)simtotal/rows,similarity);
	return similarity;
}

int skipline(nlist* first, int* numbers, int count) {
	if(simlimit == -1) simlimit = count / 2 - 1;
	if(checksimilarity(first,numbers,count) >= simlimit) return 1;
	else return 0;
}

void freelist(nlist* first) {
	nlist* iter = first;
	while(iter != NULL) {
		nlist* temp = iter;
		iter = iter->next;
		free(temp);
	}
}

void reinit_rand(int add) {
	srand(time(NULL));
	//printf("reinit rand() with %d\n",add);
}

int main(int argc, char *argv[]) {
	if(argc < 3 || argc == 4 || argc > 5) {
		printf("parameters: <numbers to select> <row count> <min num> <max num>\n");
		printf("<numbers to select> = LOTTO: 7, VIKING: 6, EUROJACKPOT: 5\n");
		exit(1);
	}
	int count = atoi(argv[1]);
	int rows = atoi(argv[2]);
	int highlimit = -1, lowlimit = -1;
	int maxnum = 0;
	int value = 0;
	int exists = 0;
	int skips = 0, skiplimit = 10000, totalskips = 0;
	int cuser = 0;

	int numbers[COUNTMAX] = {0};

	nlist *first = NULL, *current = NULL;

	reinit_rand(0);

	initscr();
	cbreak();

	switch(count) {
		case 10:
		case 9:
		case 8:
			maxnum = KENO;
			break;
		case 7:
			maxnum = LOTTO;
			break;
		case 6:
			maxnum = VIKING;
			break;
		case 5:
			maxnum = EUROJACK;
			count = 7;
			break;
		default:
			break;

	}
	if(strcmp(argv[4],"MAX") == 0) highlimit = maxnum;
	else if(strcmp(argv[4],"MID") == 0) highlimit = maxnum / 2;
	else highlimit = (argc == 5) ? atoi(argv[4]) : -1;

	if(strcmp(argv[3],"MIN") == 0) lowlimit = 0;
	else if(strcmp(argv[3],"MID") == 0) lowlimit = maxnum / 2;
	else lowlimit = (argc == 5) ? atoi(argv[3]) : -1;

	if(lowlimit >= highlimit) highlimit = lowlimit = -1;

	int initial = 1;
	while (initial == 1 || cuser == 'r') {
		initial = 0;
		for(int x = 0; x < rows; x++) {
			for(int i = 0 ; i < count ; i++) {
				exists = 0;
				while(1) {
					if(maxnum == (EUROJACK) && i > 4) value = rand() % 8;
					else value = rand() % maxnum;
					if(lowlimit != -1 && highlimit != -1) {
						if(value > lowlimit && value < highlimit) break;
					}
					else if(value > 0) break;
				}
				if(maxnum == EUROJACK && i > 4) exists = contains(&numbers[5],i,value);
				else exists = contains(numbers,i,value);
				if(exists == 0) numbers[i] = value;
				else i--;
			}
			if(maxnum != EUROJACK) sort(numbers,0,count-1);
			else {
				sort(numbers,0,4);
				sort(&numbers[5],5,count-1);
			}

			if(skipline(first,numbers,count) == 0) {

				if(!first) {
					first = addlist(first,numbers,count);
					current = first;
				}
				else current = addlist(current,numbers,count);
				skips = 0;
			}
			else {
				//printf("skipping too similar line\n");
				x--;
				skips++;
			}

			if(skips >= skiplimit) {
				//printarray(numbers,count,maxnum);
				screenupdate(first,count,maxnum,numbers,count,maxnum,x+1);
				reinit_rand(rand()-x+1);
				//printlist(first,count,maxnum);
				//printf("skip limit reached\n");
				skips = 0;
				totalskips++;
				if(skiplimiter == totalskips) {
					if(simlimit < rows) simlimit++;
					skiplimiter = skiplimiter*2;
					totalskips = 0;
				}
			}
			//printf("\n");
			zeroarray(numbers,COUNTMAX);
			if(x+1 == rows) screenupdate(first,count,maxnum,NULL,0,0,x+1);
		}
		cuser = getch();
		//printw("Press 'y' to print to stdout instead of ncurses screen");
		if(cuser != 'y') {
			freelist(first);
			first = NULL;
		}
	}
	//while ((cuser = getch()) == 'r');
	screenupdate(first,count,maxnum,NULL,0,0,0);
	endwin();

	if(cuser == 'y') printtype = 1;
	printlist(first,count,maxnum);
	if(first) freelist(first);
	return 0;
}

