/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
// TestDCAPI.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>


#include "DCAPI.h"

int counter = 0;

uint32_t myDataCallback(uint8_t *memoryBlock, uint32_t blockSize, uint32_t flags)
{
	uint32_t rv = 0;

	//printf("MyDataCallBack\nmemBlock = %p\nblockSize = %d\n", memoryBlock, blockSize);

	uint32_t *pData = (uint32_t*)memoryBlock;
	for ( int count = 0 ; count < blockSize ; count ++ ){
		printf("%X ", pData[count]);
	}
	printf("\n");

	counter ++;

	return ( rv );
}


int main(void)
{
	CounterSetDefinition csdef;
	CounterDefinition cdef;

	printf("Shim v1.0\n");

	int32_t err = DCAPI_Initialize(0);
	if ( err == DCAPI_SUCCESS ){
		printf("DCAPI_Init OK\n");

		// Enumerate counters
		uint32_t nCounterSets = DCAPI_GetNumCounterSets(0);
		printf("%d counter sets supported\n", nCounterSets);


		for ( int i = 0 ; i < nCounterSets ; i ++ ){
			if ( DCAPI_GetCounterSetDefinition(i, &csdef) == DCAPI_SUCCESS ){
				// dump counter set
				printf("\nID = %X\n", csdef.ID);
				printf("Name = [%s]\n", csdef.Name);
				printf("Description = [%s]\n", csdef.Description);
				printf("NumCounters = %d\n", csdef.NumCounters);
				printf("Category = %d\n", csdef.Category);

				// now associated counters

				for ( int c = 0 ; c < csdef.NumCounters ; c ++ ){
					if ( DCAPI_GetCounterSetCounterDefinition(csdef.ID, c, &cdef) == DCAPI_SUCCESS){
						printf("\nCounter ID = %X\n", cdef.ID);
						printf("Name = %s\n", cdef.Name);
						printf("Description = %s\n", cdef.Description);
						printf("Category = %d\n", cdef.Category);
						printf("Type = %d\n", cdef.Type);
						printf("Unit = %d\n", cdef.Unit);

					}
				}
			}
		}

		// Now enable some counters for collection
		if ( DCAPI_SetCounterSetState(csdef.ID, 1) == DCAPI_SUCCESS ){
			printf("Counter set ID=%X enabled\n", csdef.ID);


			// try collecting some data
			if ( DCAPI_SetDataReadyCallback(myDataCallback, 0) == DCAPI_SUCCESS){
				printf("Datacallback registered OK\n");

				counter = 0;

				if ( DCAPI_StartCollection(100000) == DCAPI_SUCCESS ){
					printf("Datacollection started\n");

					sleep(10);

					if ( DCAPI_StopCollection(0) == DCAPI_SUCCESS ){
						printf("Datacollection stopped\n");
					}
					else {
						printf("stopping Datacollection failed\n");	
					}

				}
				else {
					printf("Datacollection start failed\n");
				}
			}
			else {
				printf("Registering Datacallback failed\n");	
			}

		}
		else {
			printf("Failed enabling Counter set ID=%X\n", csdef.ID);	
		}



		err = DCAPI_Terminate(0);
	}
	else {
		printf("Failed DCAPI Init\n");
	}

	return ( 0 );
}