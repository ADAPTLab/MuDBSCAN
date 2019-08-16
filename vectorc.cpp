#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "vectorc.h"


void vector_init(vectorc* v, VECTORTYPE type)
{
    v->capacity = VECTOR_INIT_CAPACITY;
    v->total = 0;
    v->type = type;

    switch (v->type)
    {
        case GROUP:
            v->doubleItems = NULL;
            v->intItems = NULL;
            v->groupItems = (Group*)calloc(v->capacity, sizeof(Group));
            // assert(v->groupItems != NULL);
        break;


        case DOUBLE:
            v->intItems = NULL;
            v->groupItems = NULL;
            v->doubleItems = (double*)calloc(v->capacity, sizeof(double));
            // assert(v->doubleItems != NULL);
        break;

        case INTEGER:
            v->doubleItems = NULL;
            v->groupItems = NULL;
            v->intItems = (int*)calloc(v->capacity, sizeof(int));
            // assert(v->intItems != NULL);
        break;

        // default:
        //     fprintf(stderr, "In VECTOR_INIT(): Pass in a valid type\n");
        //     exit(-1);
        // break;
    }
}

bool vector_isEmpty(vectorc* v)
{
    return (v->total==0)?true:false;
}

int vector_type(vectorc* v)
{
    return v->type;
    
}

int vector_total(vectorc* v)
{
    return v->total;
}

void vector_resize(vectorc* v, int capacity, void* val)
{
    int i;

    double* doubleItems;
    int* intItems;
    Group* groupItems;

    double d;
    int j;
    Group g; 
    switch (v->type)
    {
        case DOUBLE:
            doubleItems = (double*)realloc(v->doubleItems, sizeof(double)*capacity);
            // assert(doubleItems != NULL);

            if(v->total >= capacity)
            {
                v->total = capacity;
                v->capacity = capacity;
            }            
            if(doubleItems)
            {
                v->intItems = NULL;
                v->groupItems = NULL;
                v->doubleItems = doubleItems;
                v->capacity = capacity;

                for(i=v->total;i<capacity;i++){
                    d = *((double*)val);
                    v->doubleItems[i] = d;
                }
                v->total = v->capacity;
            }
            break;

        case INTEGER:
            intItems = (int*)realloc(v->intItems, sizeof(int) * capacity);
            // assert(intItems != NULL);

            if(v->total >= capacity)
            {
                v->total = capacity;
                v->capacity = capacity;
            }
            if (intItems) 
            {
                v->doubleItems = NULL;
                v->groupItems = NULL;
                v->intItems = intItems;
                v->capacity = capacity;
                for(i=v->total;i<capacity;i++)
                {
                    j = *((int*)val);
                    v->intItems[i] = d;
                }
                v->total = v->capacity;
            }
            break;

        case GROUP:
            groupItems = (Group*)realloc(v->groupItems, sizeof(Group) * capacity);
            // assert(groupItems != NULL);

            g = *((Group*)val);
            if(v->total >= capacity)
            {
                v->total = capacity;
                v->capacity = capacity;
            }
            if (groupItems) 
            {
                v->doubleItems = NULL;
                v->intItems = NULL;
                v->groupItems = groupItems;
                v->capacity = capacity;
                for(i=v->total;i<capacity;i++)
                {
                    g = ((Group)val);
                    v->groupItems[i] = g;
                }
                v->total = v->capacity;
            }
            break;

        // default:
        //     fprintf(stderr, "Vector doesn't have a type\n");
        //     exit(-1);
        //     break;
    }
}

void vector_resize_internal(vectorc* v, int capacity)
{

    // #ifdef DEBUG_ON
    //     printf("vector_resize: %d to %d\n", v->capacity, capacity);
    // #endif
    int i;
    double* doubleItems = NULL;
    int* intItems = NULL;
    Group* groupItems = NULL;
    BCell* bcellItems = NULL;

    switch (v->type)
    {
        case DOUBLE:
            doubleItems = (double*) realloc(v->doubleItems, sizeof(double)*capacity);
            // assert(doubleItems != NULL);
            if(doubleItems)
            {
                v->intItems = NULL;
                v->groupItems = NULL;
                v->bcellItems = NULL;
                v->doubleItems = doubleItems;
                v->capacity = capacity;
            }
            break;

        case INTEGER:
            intItems = (int*) realloc(v->intItems, sizeof(int) * capacity);
            // assert(intItems != NULL);
            // fprintf(stderr, "Old: %p\t New: %p\n", v->intItems, intItems);
            if (intItems) 
            {
                v->doubleItems = NULL;
                v->groupItems = NULL;
                v->bcellItems = NULL;
                v->intItems = intItems;
                v->capacity = capacity;

            }
            break;

        case GROUP:
            groupItems = (Group*) realloc(v->groupItems, sizeof(Group) * capacity);
            // assert(groupItems != NULL);
            if (groupItems) 
            {
                v->doubleItems = NULL;
                v->intItems = NULL;
                v->bcellItems = NULL;
                v->groupItems = groupItems;
                v->capacity = capacity;
            }
            break;

        // default:
        //     fprintf(stderr, "Vector doesn't have a type\n");
        //     exit(-1);
        //     break;
    }
}

void vector_add(vectorc* v, void* item)
{
    int item1;
    double d;
    Group g;
    if(v->capacity == 0)
    {
        VECTOR_INIT(v, v->type);
    }
    if (v->capacity == v->total)
        vector_resize_internal(v, v->capacity*2);
    
    switch(v->type)
    {
        case INTEGER:
            item1 = *((int*)item);
            v->intItems[v->total++] = item1;
            break;

        case DOUBLE:
            d = *((double*)item);
            v->doubleItems[v->total++] = d;
            break;

        case GROUP:
            g = ((Group)item);
            v->groupItems[v->total++] = g;
            break;

        default:
            break;
    }
}

// void* vector_get(vectorc* v, int index)
// {
//     switch(v->type)
//     {
//         case INTEGER:
//             if (index >= 0 && index < v->total)
//                 return &(v->intItems[index]);
//             break;
//         case GROUP:
//             if (index >= 0 && index < v->total)
//                 return v->groupItems[index];
//             break;
//         default:
//             return NULL;
//             break;
//     }
    
// }

void vector_free(vectorc* v)
{
    int i;
    v->total = 0;
    v->capacity = 0;
    switch(v->type)
    {
        case INTEGER:
            free(v->intItems);
            break;

        case DOUBLE:
            free(v->doubleItems);
            break;

        case GROUP:
            for(i=0;i<v->total;i++)
            {
                free(v->groupItems[i]);
            }
            free(v->groupItems);
            break;
            
        default:
            break;
    }
}
