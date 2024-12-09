// TODO : PROPER ERROR HANDLING, GOOD ABSTRACTIONS THAT DONT SUCK, ETC..

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
    size_t size;
    size_t capacity;
    size_t element_size;

    void *data;
} stivector;

int stivector_init(stivector *vector, size_t capacity, size_t element_size) {
    vector->size = 0;
    vector->capacity = capacity;
    vector->element_size = element_size;

    vector->data = malloc(capacity * element_size);
    if (vector->data == NULL) {
        // TODO : error handling
        fprintf(stderr, "could not allocate vector\n");
        return -1;
    }

    return 0;
}

int stivector_resize(stivector *vector, size_t capacity) {
    if (vector == NULL || vector->data == NULL) {
        // TODO
        fprintf(stderr, "vector is null\n");
        return -1;
    }

    if (capacity < vector->size) {
        // TODO
        fprintf(stderr, "invalid capacity\n");
        return -1;
    }
    
    vector->data = realloc(vector->data, capacity * vector->element_size);
    if (vector->data == NULL) {
        // TODO
        fprintf(stderr, "realloc failed oops\n");
        return -1;
    }
    
    vector->capacity = capacity;
    return 0;
}

int stivector_shrink_to_fit(stivector *vector) {
    if (vector == NULL || vector->data == NULL) {
        //TODO
        fprintf(stderr, "vector is null\n");
        return -1;
    }

    if (stivector_resize(vector, vector->size) != 0) {
        //TODO
        return -1;
    }

    return 0;
}

int stivector_push_back(stivector *vector, void *element) {
    if (vector == NULL || vector->data == NULL || element == NULL) {
        //TODO
        fprintf(stderr, "vector or element is null\n");
        return -1;
    }
    
//    if (element_size != vector->element_size) {
//        //TODO
//        fprintf(stderr, "wrong size dumb fuck\n");
//        return -1;
//    }

    if (vector->size == vector->capacity) {
        if (stivector_resize(vector, vector->capacity * 2) != 0) {
            //TODO
            return -1;
        }
    }

    memmove(vector->data + vector->size * vector->element_size, element, vector->element_size);
    vector->size++;

    return 0;
}

// fire ass function
int stivector_pop_back(stivector *vector) {
    if (vector == NULL || vector->data == NULL) {
        //TODO
        fprintf(stderr, "vector is null\n");
        return -1;
    }

    vector->size--;
    return 0;
}

int stivector_insert(stivector *vector, size_t index, void *element) {
    if (vector == NULL || vector->data == NULL || element == NULL) {
        //TODO
        fprintf(stderr, "vector or element is null\n");
        return -1;
    }
   
    if (index > vector->size) {
        //TODO
        fprintf(stderr, "index is too big to fit\n");
        return -1;
    }
    
//    if (element_size != vector->element_size) {
//        //TODO
//        fprintf(stderr, "wrong size dumb fuck\n");
//        return -1;
//    }

    if (index == vector->size) {
        if (stivector_push_back(vector, element) != 0) {
            //TODO
            return -1;
        }

        return 0;
    }

    if (vector->size == vector->capacity) {
        if (stivector_resize(vector, vector->capacity * 2) != 0) {
            //TODO
            return -1;
        }
    }
    
    // welcome to hell
    memmove(vector->data + (index + 1) * vector->element_size,
            vector->data + index * vector->element_size,
            vector->element_size * (vector->size - 1- index));

    //element insertion
    memmove(vector->data + index * vector->element_size, element, vector->element_size);

    vector->size++;

    return 0;
}

int stivector_erase(stivector *vector, size_t index) {    
    if (vector == NULL || vector->data == NULL) {
        //TODO
        fprintf(stderr, "vector is null\n");
        return -1;
    }
    
    if (index >= vector->size) {
        //TODO
        fprintf(stderr, "index is too big to fit\n");
        return -1;
    }
    
    if (index == vector->size - 1) {
        if (stivector_pop_back(vector) != 0) {
            //TODO
            return -1;
        }

        return 0;
    }
    
    // welcome to hell part 2
    memmove(vector->data + index * vector->element_size,
            vector->data + (index + 1) * vector->element_size,
            vector->element_size * (vector->size - 1 - index));

    vector->size--;

    return 0;
}

void *stivector_at(stivector *vector, size_t index) {
    if (vector == NULL || vector->data == NULL) {
        //TODO
        fprintf(stderr, "vector is null\n");
        return NULL;
    }

    if (index >= vector->size) {
        //TODO
        fprintf(stderr, "index is too big to fit\n");
        return NULL;
    }

    return vector->data + vector->element_size * index;
}

int stivector_free(stivector *vector) {
    if (vector == NULL || vector->data == NULL) {
        //TODO
        fprintf(stderr, "vector is null\n");
        return -1;
    }

    free(vector->data);
    vector->data = NULL;
    
    return 0;
}
