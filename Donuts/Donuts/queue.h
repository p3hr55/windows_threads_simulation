#pragma once
struct {
	int *array;
	HANDLE guard;
	int front, rear, capacity, count;
} typedef Queue;

int empty(Queue *structure) {
	return(structure->front == -1);
}

int size(Queue *structure) {
	return structure->count;
}


void insert(Queue *structure, int data) {
	structure->rear = (structure->rear + 1) % structure->capacity;
	structure->array[structure->rear] = data;
	if (structure->front == -1) {
		structure->front = structure->rear;
	}

	structure->count++;
}

int del(Queue *structure) {
	int data = 0;
	data = structure->array[structure->front];
	if (structure->front == structure->rear)
		structure->front = structure->rear = -1;
	else
		structure->front = (structure->front + 1) % structure->capacity;

	return data;
}