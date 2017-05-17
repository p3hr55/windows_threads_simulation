#define _CRT_SECURE_NO_WARNINGS
#include "Windows.h"
#include "stdio.h"
#include "process.h"
#include "queue.h"
#include "inc.h"
#define NUM_BINS 4
#define NUM_TO_BAKE 10
#define BIN_CAPACITY 20

DWORD WINAPI BAKE(void *);
DWORD WINAPI WORK(void *);
DWORD WINAPI MANAGE(void *);

Baker Bakers[2];
Bins donut_bins[4];
Server Servers[6];
Manager Managers[4];
BakerStatus bs;
DonutStatus ds;
WorkerStatus ws;
BOOL Bins_not_empty[4] = { FALSE, FALSE, FALSE, FALSE };
BOOL Bakers_alive[2] = { TRUE, TRUE };
BOOL Servers_alive[4] = { TRUE, TRUE, TRUE, TRUE };
BOOL Managers_alive[4] = { TRUE, TRUE, TRUE, TRUE };
Queue * Bin_queues[4];
DWORD total_donuts;
DWORD sleep_time;


void main() {
	DWORD i;
	printf("How Many Donuts will each Baker bake?: ");
	scanf("%d", &total_donuts);

	printf("How long does it take each baker to produce a donut?: ");
	scanf("%d", &sleep_time);

	Bakers[0].num_to_bake = total_donuts;
	Bakers[0].num_baked = 0;
	Bakers[0].position = 0;

	Bakers[1].num_to_bake = total_donuts;
	Bakers[1].num_baked = 0;
	Bakers[1].position = 1;

	bs.done = FALSE;
	bs.guard = CreateMutex(NULL, FALSE, NULL);

	ds.all_donuts_gone = FALSE;
	ds.guard = CreateMutex(NULL, FALSE, NULL);

	ws.all_workers_done = FALSE;
	ws.guard = CreateMutex(NULL, FALSE, NULL);


	for (i = 0; i < NUM_BINS; i++) {
		Bin_queues[i] = malloc(sizeof(Queue));
		Bin_queues[i]->capacity = 6;
		Bin_queues[i]->front = -1;
		Bin_queues[i]->rear = -1;
		Bin_queues[i]->array = malloc(Bin_queues[i]->capacity * sizeof(int));
		Bin_queues[i]->guard = CreateMutex(NULL, FALSE, NULL);
	}

	for (i = 0; i < NUM_BINS; i++) {
		donut_bins[i].guard = CreateMutex(NULL, FALSE, NULL);
		donut_bins[i].ready = CreateEvent(NULL, FALSE, FALSE, NULL);
		donut_bins[i].capacity = BIN_CAPACITY;
		donut_bins[i].donuts = donut_bins[i].served = 0;
	}

	Bakers[0].h = (HANDLE)_beginthreadex(NULL, 0, BAKE, (DWORD)0, 0, NULL);
	Bakers[1].h = (HANDLE)_beginthreadex(NULL, 0, BAKE, (DWORD)1, 0, NULL);

	for (i = 0; i < 6; i++) {
		Servers[i].num_served = 0;
		Servers[i].position = i;
		Servers[i].go = CreateEvent(NULL, FALSE, FALSE, NULL);
		Servers[i].h = (HANDLE)_beginthreadex(NULL, 0, WORK, i, 0, NULL);
	}

	for (i = 0; i < NUM_BINS; i++) {
		Managers[i].done = CreateEvent(NULL, FALSE, FALSE, NULL);
		Managers[i].h = (HANDLE)_beginthreadex(NULL, 0, MANAGE, i, 0, NULL);
	}

	WaitForSingleObject(Bakers[0].h, INFINITE);
	WaitForSingleObject(Bakers[1].h, INFINITE);

	for (i = 0; i < 6; i++)
		WaitForSingleObject(Servers[i].h, INFINITE);
	
	printf("Baker #1\nMade %d donuts\n%d type #1\n%d type #2\n%d type #3\n%d type #4\n\n",
		Bakers[0].num_baked, Bakers[0].type_1, Bakers[0].type_2, Bakers[0].type_3, Bakers[0].type_4);

	printf("Baker #2\nMade %d donuts\n%d type #1\n%d type #2\n%d type #3\n%d type #4\n\n",
		Bakers[1].num_baked, Bakers[1].type_1, Bakers[1].type_2, Bakers[1].type_3, Bakers[1].type_4);

	printf("Bin #1\n%d donuts\n\nBin #2\n%d donuts\n\nBin #3\n%d donuts\n\nBin #4\n%d donuts\n\n", 
		donut_bins[0].served, donut_bins[1].served, donut_bins[2].served, donut_bins[3].served);

	for (i = 0; i < 6; i++)
		printf("Worker #%d\nSold %d donuts\n%d type #1\n%d type #2\n%d type #3\n%d type #4\n\n",
		i + 1, Servers[i].num_served, Servers[i].type_1, Servers[i].type_2, Servers[i].type_3, Servers[i].type_4);

	system("pause");
}

DWORD WINAPI BAKE(void *i)
{
	DWORD pos = (DWORD *)i;
	DWORD current_donut;
	srand((DWORD)time(NULL) + pos);

	while (Bakers[pos].num_baked != Bakers[pos].num_to_bake) {
		current_donut = rand() % 4;
		WaitForSingleObject(donut_bins[current_donut].guard, INFINITE);

		if (donut_bins[current_donut].donuts < donut_bins[current_donut].capacity) {

			__try {
				InterlockedIncrement(&donut_bins[current_donut].donuts);
				InterlockedIncrement(&Bakers[pos].num_baked);
				switch (current_donut) {
				case 0: InterlockedIncrement(&Bakers[pos].type_1);
					break;
				case 1: InterlockedIncrement(&Bakers[pos].type_2);
					break;
				case 2: InterlockedIncrement(&Bakers[pos].type_3);
					break;
				case 3: InterlockedIncrement(&Bakers[pos].type_4);
					break;
				}

				Bins_not_empty[current_donut] = TRUE;
			}

			__finally {
				ReleaseMutex(donut_bins[current_donut].guard);
			}
		}

		Sleep(sleep_time);
	}

	Bakers_alive[pos] = FALSE;

	if (Bakers_alive[1] == FALSE && Bakers_alive[0] == FALSE) {
		WaitForSingleObject(bs.guard, INFINITE);
		bs.done = TRUE;
		ReleaseMutex(bs.guard);
	}

	return 0;
}

DWORD WINAPI WORK(void * a) 
{
	DWORD position;
	DWORD donut_number; 
	
	position = (DWORD *)a;
	srand((DWORD)time(NULL) + position);

	while (TRUE) {
		if (bakers_done() && empty_bins()) {
			Servers_alive[position] = FALSE;
			return 0;
		}

		donut_number = rand() % 4;

		__try {
			WaitForSingleObject(Bin_queues[donut_number]->guard, INFINITE);
			insert(Bin_queues[donut_number], position);
			ReleaseMutex(Bin_queues[donut_number]->guard);

			WaitForSingleObject(Servers[position].go, INFINITE);
			WaitForSingleObject(donut_bins[donut_number].guard, INFINITE);

			if (donut_bins[donut_number].donuts > 0) {

				InterlockedIncrement(&Servers[position].num_served);
				InterlockedDecrement(&donut_bins[donut_number].donuts);
				InterlockedIncrement(&donut_bins[donut_number].served);
				switch (donut_number) {
				case 0: InterlockedIncrement(&Servers[position].type_1);
					break;
				case 1: InterlockedIncrement(&Servers[position].type_2);
					break;
				case 2: InterlockedIncrement(&Servers[position].type_3);
					break;
				case 3: InterlockedIncrement(&Servers[position].type_4);
					break;
				}

				if (donut_bins[donut_number].donuts == 0)
					Bins_not_empty[donut_number] = FALSE;
			}

			ReleaseMutex(donut_bins[donut_number].guard);
			//Sleep(200);
		}

		__finally {
			ReleaseMutex(Bin_queues[donut_number]->guard);
			ReleaseMutex(donut_bins[donut_number].guard);
		}
		
	}

	return 0;
}

DWORD WINAPI MANAGE(void * a) {

	DWORD position = (DWORD *)a;
	DWORD next = -1;

	while (TRUE) {

		__try {
			WaitForSingleObject(donut_bins[position].guard, INFINITE);
			WaitForSingleObject(Bin_queues[position]->guard, INFINITE);

			if (!empty(Bin_queues[position])) {
				next = del(Bin_queues[position]);
				SetEvent(Servers[next].go);
			}

			ReleaseMutex(Bin_queues[position]->guard);
			ReleaseMutex(donut_bins[position].guard);

			//Sleep(200);
		}

		__finally {
			ReleaseMutex(Bin_queues[position]->guard);
			ReleaseMutex(donut_bins[position].guard);
		}
	}
}

BOOL workers_done() {
	if (Servers_alive[0] == FALSE && Servers_alive[1] == FALSE && Servers_alive[2] == FALSE && Servers_alive[3] == FALSE)
		return TRUE;
	return FALSE;
}

BOOL bakers_done() {
	if (Bakers_alive[0] == FALSE && Bakers_alive[1] == FALSE)
		return TRUE;
	return FALSE;
}

BOOL empty_bins()
{
	if (Bins_not_empty[0] == FALSE && Bins_not_empty[1] == FALSE && Bins_not_empty[2] == FALSE && Bins_not_empty[3] == FALSE)
		return TRUE;
	return FALSE;
}