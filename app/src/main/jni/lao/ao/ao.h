#ifndef AO_H
#define AO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AO_FMT_LITTLE 1
#define AO_FMT_BIG    2
#define AO_FMT_NATIVE 3

#define AO_TYPE_LIVE 1
#define AO_TYPE_FILE 2

#define AO_ENODRIVER    1
#define AO_ENOTLIVE     2
#define AO_EBADOPTION   3
#define AO_EOPENDEVICE  4
#define AO_EFAIL        5
#define AO_ENOTFILE     6
#define AO_EOPENFILE    7
#define AO_EFILEEXISTS  8

typedef struct ao_device ao_device;
typedef struct ao_sample_format ao_sample_format;
typedef struct ao_option ao_option;
typedef struct ao_info ao_info;

struct ao_sample_format {
    int bits;
    int channels;
    int rate;
    int byte_format;
    char *matrix;
};

struct ao_option {
    char *key;
    char *value;
    ao_option *next;
};

struct ao_info {
    int type;
    const char *name;
    const char *short_name;
    const char *comment;
    int priority;
    int preferred_byte_format;
    int option_count;
    const char **options;
};

void ao_initialize(void);
void ao_shutdown(void);
int ao_append_global_option(const char *key, const char *value);
int ao_append_option(ao_option **options, const char *key, const char *value);
void ao_free_options(ao_option *opt);

ao_device *ao_open_live(int driver_id, ao_sample_format *format, ao_option *option);
ao_device *ao_open_file(int driver_id, const char *filename, int overwrite,
                        ao_sample_format *format, ao_option *option);
int ao_play(ao_device *device, char *samples, uint32_t num_bytes);
int ao_close(ao_device *device);

int ao_driver_id(const char *driver_name);
int ao_default_driver_id(void);
ao_info *ao_driver_info(int driver_id);
ao_info **ao_driver_info_list(int *count);
bool ao_is_big_endian(void);

#ifdef __cplusplus
}
#endif

#endif
