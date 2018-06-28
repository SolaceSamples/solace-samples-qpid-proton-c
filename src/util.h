/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#ifndef UTIL_H
#define UTIL_H 1


#include <proton/codec.h>

#include <stdlib.h>


/*
 * get string/symbol value from pn_data_t type map
 * parameters in:
 *      properties: the pointer to pn_data_t, this must be a map.
 *      key: the search key string for the target value. 
 * parameter out:
 *      value: string value of map value at key. Is set to NULL on miss.
 * returns:
 *      Integer code indicating hit or miss in search.
 *      -2, properties was not of type PN_MAP or invalid parameter
 *      -1, search key of type PN_STRING or PN_SYMBOL was not found in map
 *      0, a key was found matching the search key but the target value type was not PN_STRING or PN_SYMBOL
 *      1, a key was found matching the search key and the value was assigned
 * */
int get_data_map_string_property(pn_data_t* properties, const char* const key, char* value, const size_t value_size);

/*
 * Formats an AMQP terminus address with a destination type prefix.
 * The address_prefix is only added if the base address does not start
 * with the address_prefix.
 * parameters in:
 *      dest, destination amqp address string, takes the form of <address_prefix><address>, eg. topic://my_topic
 *      dest_len, the dest buffer length
 *      address, base amqp address imutable buffer, eg 'my_topic'
 *      address_len, base amqp address imutable buffer length
 *      address_prefix, amqp address destination type imutable buffer, eg. 'topic://'
 *      address_prefix, amqp address destination type imutable buffer length
 * returns:
 *      The number of chars written to dest or < 0 for an error.
 */
int amqp_destination_address(char* dest, const size_t dest_len,
                       const char* const address, const size_t address_len,
                       const char* const address_prefix, const size_t address_prefix_len);

/* 
 * Formats an AMPQ container id from a given source and write the id to dest.
 * AMQP Container id format can vary across different brokers.
 * The id format given by this function is '<source>:<pid>' or 
 * in the case source is empty or null 'amqp_conatainer:<pid>'
 * 
 * @param[out]: dest, the buffer the container id is written to
 * @param[in]: dest_len, the length of the container id buffer
 * @param[in]: source, the initial string to make the container id from, can be null or empty
 * @param[in]: source_len, the initial string length
 *
 * @returns: Interger for the number of characters written to dest or negative if error
 * */
int container_id(char *dest, const size_t dest_len, 
                char *source, const size_t source_len);

#endif /* util.h */
