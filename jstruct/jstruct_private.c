#include "jstruct_private.h"
void *
jstruct_prop_ptr(const void *data, const struct jstruct_object_property *property, int index) {
    unsigned char *ptr = ((unsigned char *)data + property->offset);
    if (property->dereference && index != JSTRUCT_PROP_PTR_GET_NO_DEREF) {
        ptr = *(void **)ptr;
    }
    if (index > 0) {
        assert(property->stride);
        ptr += index * property->stride;
    }
    return ptr;
}

int jstruct_length_get(const void *data, const struct jstruct_object_property *property) {
    assert(data);assert(property);
    return *(int *)((unsigned char *)data + property->length_offset);
}

void jstruct_length_set(const void *data, const struct jstruct_object_property *property, int length) {
    assert(data);assert(property);
    // length_offset can't be 0 because the pointer to the array must come before it.
    assert(property->length_offset);
    int *value = (int *)((unsigned char *)data + property->length_offset);
    *value = length;
}

bool jstruct_null_get(const void *data, const struct jstruct_object_property *property) {
    assert(data);assert(property);
    return *(bool *)((unsigned char *)data + property->null_offset);
}

void jstruct_null_set(const void *data, const struct jstruct_object_property *property,
        bool value) {
    assert(data);assert(property);
    bool *ptr = (bool *)((unsigned char *)data + property->null_offset);
    *ptr = value;
}

bool jstruct_allocated_add(struct array_list *arr, enum jstruct_allocated_type type, void *data) {
    struct jstruct_allocated *allocated = malloc(sizeof(struct jstruct_allocated));
    allocated->type = type;
    allocated->data = data;
    return array_list_add(arr, allocated) == 0;
}

void jstruct_allocated_free(void *data) {
    struct jstruct_allocated *allocated = (struct jstruct_allocated *)data;
    switch (allocated->type) {
        case jstruct_allocated_type_raw:
            free(allocated->data);
            break;
        case jstruct_allocated_type_arraylist:
            array_list_free((array_list *)allocated->data);
            break;
    }
    free(data);
}
