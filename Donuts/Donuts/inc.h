#pragma once
typedef struct {
	HANDLE guard;
	HANDLE ready;
	DWORD capacity;
	volatile DWORD donuts;
	volatile DWORD served;
} Bins;

typedef struct {
	HANDLE h;
	DWORD position;
	DWORD num_to_bake;
	volatile DWORD num_baked;
	volatile DWORD type_1;
	volatile DWORD type_2;
	volatile DWORD type_3;
	volatile DWORD type_4;
} Baker;

typedef struct {
	HANDLE go;
	HANDLE h;
	DWORD position;
	volatile DWORD num_served;
	volatile DWORD type_1;
	volatile DWORD type_2;
	volatile DWORD type_3;
	volatile DWORD type_4;
} Server;

typedef struct {
	HANDLE h;
	HANDLE done;
} Manager;

typedef struct {
	HANDLE guard;
	BOOL done;
} BakerStatus;

typedef struct {
	HANDLE guard;
	BOOL all_donuts_gone;
} DonutStatus;

typedef struct {
	HANDLE guard;
	BOOL all_workers_done;
} WorkerStatus;