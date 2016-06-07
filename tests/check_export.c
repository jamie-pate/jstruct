#include "check_export.h"
#include <stdio.h>
#include "check_jstruct.h"
#include "../jstruct/jstruct.h"
#include "../jstruct/jstruct_private.h"
#include "../jstruct/export.h"
#include "data/basic.h"

START_TEST(export_basic_data) {
    struct my_json_data data = make_data();
    struct json_object *obj = jstruct_export(&data, my_json_data);
    fprintf(stdout, "JSON OUTPUT (basic): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    test_data(data, obj);
    json_object_put(obj);
} END_TEST

START_TEST(export_struct_data) {
    struct my_json_container c = make_container();
    struct json_object *obj = jstruct_export(&c, my_json_container);
    test_container(c, obj);
    // TODO: autofree
    free(c.alloc_array_data);
    json_object_put(obj);

} END_TEST

START_TEST(export_null) {
    struct my_json_data data = make_data();
    data.ratio_double__null__ = true;
    struct json_object *obj = jstruct_export(&data, my_json_data);
    struct json_object *prop;
    ck_assert(json_object_object_get_ex(obj, "ratio", &prop));
    ck_assert_int_eq(json_object_get_type(prop), json_type_null);
    ck_assert(prop == NULL);

    json_object_put(obj);
} END_TEST

START_TEST(export_foreign_data) {
    struct my_json_data data = make_data();
    //fdata are only coincidentally binary compatible.
    struct foreign_struct fdata = {
        .id=data.id,
        ._id=data._id,
        .err=data.err,
        .active=data.active,
        .ratio_double=data.ratio_double,
        .name=data.name,
        .ull=data.ull,
        .tags=data.tags
    };
    struct my_json_foreign_struct jfdata = {0};
    memcpy(&jfdata, &fdata, sizeof(fdata));
    jfdata.tags__length__ = data.tags__length__;
    // spot check 
    ck_assert_int_eq(jfdata.id, data.id);
    ck_assert(jfdata.ratio_double == fdata.ratio_double);
    ck_assert(jfdata.ratio_double == data.ratio_double);
    ck_assert_int_eq(jfdata.ull, data.ull);
    struct json_object *obj = jstruct_export(&jfdata, my_json_foreign_struct);
    fprintf(stdout, "JSON OUTPUT (foreign): %s\n", json_object_to_json_string(obj));
    fflush(stdout);
    test_data(data, obj);
    json_object_put(obj);

} END_TEST

TCase *export_test_case(void) {
    TCase *tc = tcase_create("export");

    tcase_add_test(tc, export_basic_data);
    tcase_add_test(tc, export_struct_data);
    tcase_add_test(tc, export_null);
    tcase_add_test(tc, export_foreign_data);

    return tc;
}
