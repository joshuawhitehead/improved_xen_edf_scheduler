#include <string.h>
#include <stdlib.h>

#include "vector.h"
#include "helper.h"
#include "other.h"
#include "cbs.h"
#include "edf.h"
#include "parser.h"

/**
 * Starts the simulation
 * @param queue
 * @param events An array of SimulationEvents that are used define the course of the simulation.
 * 				 It is not required to provide them in ascending order they are sorted at the start of the simulation.
 * @param count Number of elements in the events array.
 */
void StartSimulation(struct EDFRunQueue* queue, struct SimulationEvent events[], size_t count);

/**
 * Method for qsort() for sorting SimulationEvents based on the arrival time of the jobs.
 * @param elem1
 * @param elem2
 * @return
 */
int AscendingArrivalTime(const void* elem1, const void* elem2);

void InsertRunEvent(struct CBS* server, int time, enum RunAction action);
void PrintResult(struct EDFRunQueue* queue);
void PrintCharXTimes(char c, int times);

int main(void)
{
	struct ParserResult result = parseFile("input.conf");
	struct EDFRunQueue queue = {&result.cpu, result.cpu.capacity, create_vector()};

	for(int i = 0; i < result.virtualCpuCount; ++i)
	{
		RegisterVCPU(&queue, &result.virtualCpus[i]);
	}

	StartSimulation(&queue, result.events, result.eventCount);
	PrintResult(&queue);

	for(size_t i = 0; i < queue.servers->length; i++)
	{
		struct CBS* server = ((struct CBS*)vector_get(queue.servers, i));
		destroy_vector(server->jobs);

		struct list_head *pos, *save;
		list_for_each_safe(pos, save, &server->runTimes->list)
		{
			struct RunData* tmp = list_entry(pos, struct RunData, list);
			list_del(pos);
			free(tmp);
		}
		free(server->runTimes);
	}

	freeResult(&result);
	destroy_vector(queue.servers);

	return EXIT_SUCCESS;
}

void StartSimulation(struct EDFRunQueue* queue, struct SimulationEvent events[], size_t count)
{
	int currentTime = 0;
	size_t i = 0;

	qsort(events, count, sizeof(struct SimulationEvent), AscendingArrivalTime);

	while(true)
	{
		for(struct SimulationEvent* event = &events[i]; i < count && events[i].job.arrivalTime <= currentTime; i++, event = &events[i])
		{
			for(size_t j = 0; j < queue->servers->length; j++)
			{
				struct CBS* server = vector_get(queue->servers, j);

				if(!strcmp(server->cpu->name, event->vCpuName))
				{
					AddJob(server, &(event->job));
					InsertRunEvent(server, currentTime, NewJob);

					break;
				}
			}
		}

		struct CBS* activeServer = GetServerWithED(queue);

		if(activeServer == NULL)
		{
			printf("[idle] until: %d\n", events[i].job.arrivalTime);

			currentTime = events[i].job.arrivalTime;
			continue;
		}

		if(DoWork(activeServer))
			InsertRunEvent(activeServer, currentTime, JobEnd);
		else
			InsertRunEvent(activeServer, currentTime, Work);

		currentTime++;

		// All events are processed now we need to wait for all jobs to be completed
		if(i >= count)
		{
			bool finished = true;

			for(size_t j = 0; j < queue->servers->length; j++)
			{
				if(((struct CBS*)vector_get(queue->servers, j))->state == ACTIVE)
				{
					finished = false;
					break;
				}
			}

			if(finished)
			{
				break;
			}
		}
	}
}

void InsertRunEvent(struct CBS* server, int time, enum RunAction action)
{
	struct RunData* data = malloc(sizeof(struct RunData));
	data->time = time;
	data->action = action;

	/* add the new item 'tmp' to the list of items in mylist */
	list_add_tail(&(data->list), &(server->runTimes->list));
}

void PrintCharXTimes(char c, int times)
{
	while(times--)
		putchar(c);
}

void PrintResult(struct EDFRunQueue* queue)
{
	putchar('\n');
	putchar('\n');
	puts("legend: idle = _, work = =, job arrival = J, job ends = E");
	putchar('\n');

	size_t serverCount = queue->servers->length;
	int maxTime = 0;

	for(size_t i = 0; i < serverCount; ++i)
	{
		struct CBS* server = vector_get(queue->servers, i);
		struct list_head *pos;
		struct RunData *tmp;
		int time = 0;

		printf("vCPU: %s\n", server->cpu->name);

		list_for_each(pos, &server->runTimes->list)
		{
			tmp = list_entry(pos, struct RunData, list);

			if(tmp->time >= time)
			{
				PrintCharXTimes('_', tmp->time - time);

				switch (tmp->action)
				{
				case Work:
					putchar('=');
					break;
				case JobEnd:
					putchar('E');
					break;
				case NewJob:
					putchar('J');
					break;
				}

			time = tmp->time + 1;
			}
		}

		maxTime = max(time, maxTime);

		putchar('\n');
		putchar('\n');
	}

	for(int i = 0; i < maxTime; i++)
	{
		if((i % 10) == 0)
			putchar('|');
		else
			putchar('0' + (i % 10));
	}

	printf(" --> maxTime = %d\n", maxTime);
}

int AscendingArrivalTime(const void* elem1, const void* elem2)
{
	const struct SimulationEvent* first = (const struct SimulationEvent*)elem1;
	const struct SimulationEvent* second = (const struct SimulationEvent*)elem2;

	return first->job.arrivalTime - second->job.arrivalTime;
}
