#include "parse_json.h"

void json_parse_and_init_thread()
{
    struct json_object *parsed_json; // parsed result
    struct json_object *thread;      // thread object
    struct json_object *name;        // thread name
    struct json_object *entry;       // thread entry function
    struct json_object *priority;    // thread priority
    struct json_object *cancel_mode; // thread cancel mode
    size_t num;                      // num of thread

    // parse json into result
    parsed_json = json_object_from_file("init_threads.json");
    json_object_object_get_ex(parsed_json, "Threads", &parsed_json);
    num = json_object_array_length(parsed_json);

    // creat thread
    for (size_t i = 0; i < num; i++)
    {
        thread = json_object_array_get_idx(parsed_json, i);
        json_object_object_get_ex(thread, "name", &name);
        json_object_object_get_ex(thread, "entry function", &entry);
        json_object_object_get_ex(thread, "priority", &priority);
        json_object_object_get_ex(thread, "cancel mode", &cancel_mode);

        if (~OS2021_ThreadCreate((char *)json_object_get_string(name), (char *)json_object_get_string(entry),
                                 (char *)json_object_get_string(priority), (char *)json_object_get_string(cancel_mode)))
            printf("Thread init error");
    }
}