
#include "util.h"

#include <proton/codec.h>
#include <proton/types.h>
#include <proton/object.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

/* 
 * Walks through a pn_data_t of type PN_MAP checking 
 * key value pairs for a match to the given search 'key'.
 * The given pn_data_t is restored to its initial starting
 * point. TODO improve linear search.
 * */
int get_data_map_string_property( pn_data_t* properties, 
                                    const char* const key, 
                                    char* value, const size_t value_size) {
      int found = -2; /* default return code for in valid arguments */
      if (properties && key && value && value_size > 0) {
        pn_handle_t data_start = pn_data_point(properties);
        if(pn_data_type(properties) == PN_MAP) {
            const size_t key_len = strlen(key);
            const char *map_value = NULL;
            size_t count = pn_data_get_map(properties);
            pn_data_enter(properties);
            for (int i=0; i<count/2; i++) {
                found = -1; /* return code for not found search key */
                if(pn_data_next(properties)) {
                    /* parse map key */
                    switch(pn_data_type(properties)) {
                    case PN_STRING: {
                        pn_bytes_t key_bytes = pn_data_get_string(properties);
                        map_value = key_bytes.start;
                        printf("string Key:%s\n", map_value);
                        found = key_bytes.size < key_len ? -1 : 0;
                        break;
                    }
                    case PN_SYMBOL: {
                        pn_bytes_t key_bytes = pn_data_get_symbol(properties);
                        map_value = key_bytes.start;
                        printf("symbol Key:%s\n", map_value);
                        found = key_bytes.size < key_len ? -1 : 0;
                        break;
                    }
                    default: printf("Ignoring data key field type: %s\n", pn_type_name(pn_data_type(properties))); break;
                    }
                    if(found == 0 && strncmp(key, map_value, key_len)==0) {
                        /* found matching key */
                        found=1;
                    }
                }
                if (pn_data_next(properties) && found == 1) {
                    map_value = NULL;
                    /* parse map value */
                    switch (pn_data_type(properties)) {
                    case PN_STRING: {
                        pn_bytes_t value_bytes = pn_data_get_string(properties);
                        map_value = value_bytes.start;
                        printf("string Value:%s\n", map_value);
                        break;
                    }
                    case PN_SYMBOL: {
                        pn_bytes_t value_bytes = pn_data_get_string(properties);
                        map_value = value_bytes.start;
                        printf("Symbol Value:%s\n", map_value);
                        break;
                    }
                    default: printf("Ignoring data value field type: %s\n", pn_type_name(pn_data_type(properties))); break;
                    }
                    /* assign prefix if present */
                    if (map_value) {
                        found = sprintf(value, "%s", map_value) > 0 ? 1 : 0;
                        break; /* for loop */
                    }

                }
            } /* end for loop */
        } else {
            fprintf(stderr, "Unexpected type: %s, for properties", pn_type_name(pn_data_type(properties)));
        }
        pn_data_restore(properties, data_start);
    }
    return found; 
}

/* 
 * Formats an amqp address to given 'dest' pointer with given 'address_prefix'.
 * The 'address_prefix' is only added if the base 'address' is not already present.
 * A typical 'address_prefix' would be 'topic://' or 'queue://' to indicate
 * a destination type for the general amqp address.
 * */
int amqp_destination_address(char* dest, const size_t dest_len,
                       const char* const address, const size_t address_len,
                       const char* const address_prefix, const size_t address_prefix_len) {
    /* check function parameters */
    if (dest_len < address_len + address_prefix_len || dest == NULL) {
        return -1;
    }
    /* check address base and only add prefix if address base does not already
     * start with address_prefix
     * */
    if ( address_len > address_prefix_len
        && strncmp(address, address_prefix, address_prefix_len)==0
        || address_prefix == NULL) {
        return sprintf(dest, "%s", address);
    }
    return sprintf(dest, "%s%s", address_prefix, address);
}



#define AMQP_CONTAINER_PREFIX "amqp_container"

#define AMQP_CONTAINER_PREFIX_SIZE sizeof(AMQP_CONTAINER_PREFIX)

int container_id(char *dest, const size_t dest_len, char *source, const size_t source_len) {
    if (source && source_len > 0) {
        char *base = basename(source);
        const size_t base_len = strlen(base);
        if (dest_len < base_len) {
            /* dest buffer too small to fit container id */
            return -1;
        } else if (base_len == 0) {
            /* source is a path return fixed prefix container id */
            return sprintf(dest, "%s:%d", AMQP_CONTAINER_PREFIX, getpid());
        } else {
            /* use base for container id */
            return sprintf(dest, "%s:%d", base, getpid());
        }
    } else {
        /* no source return fixed prefix container id */
        if (dest_len < AMQP_CONTAINER_PREFIX_SIZE) {
            return -1;
        }
        return sprintf(dest, "%s:%d", AMQP_CONTAINER_PREFIX, getpid());
    }
}


