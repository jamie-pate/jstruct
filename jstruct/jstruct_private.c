#include "jstruct_private.h"
void *
jstruct_prop_ptr(const void *data, const struct jstruct_object_property *property, int index) {
    unsigned char *ptr = ((unsigned char *)data + property->offset);
    if (property->dereference) {
        ptr = *(void **)ptr;
    }
    if (index) {
        assert(property->stride);
        ptr += index * property->stride;
    }
    return ptr;
}

int jstruct_length_get(const void *data, const struct jstruct_object_property *property) {
    assert(data);assert(property);
    return *(int *)((unsigned char *)data + property->length_offset);
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
