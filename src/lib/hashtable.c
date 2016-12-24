/*
 * Ouroboros - Copyright (C) 2016
 *
 * Hash table with separate chaining on collisions
 *
 *    Sander Vrijders <sander.vrijders@intec.ugent.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <ouroboros/config.h>
#include <ouroboros/hashtable.h>
#include <ouroboros/list.h>
#include <ouroboros/errno.h>

#include <assert.h>

struct htable_entry {
        struct list_head next;
        uint64_t         key;
        void *           val;
};

struct htable {
        struct list_head * buckets;
        bool               hash_key;
        uint64_t           buckets_size;
};

struct htable * htable_create(uint64_t buckets, bool hash_key)
{
        struct htable * tmp;
        unsigned int i;

        if (buckets == 0)
                return NULL;

        buckets--;
        buckets |= buckets >> 1;
        buckets |= buckets >> 2;
        buckets |= buckets >> 4;
        buckets |= buckets >> 8;
        buckets |= buckets >> 16;
        buckets |= buckets >> 32;
        buckets++;

        tmp = malloc(sizeof(*tmp));
        if (tmp == NULL)
                return NULL;

        tmp->hash_key = hash_key;
        tmp->buckets_size = buckets;

        tmp->buckets = malloc(buckets * sizeof(*tmp->buckets));
        if (tmp->buckets == NULL) {
                free(tmp);
                return NULL;
        }

        for (i = 0; i < buckets; i++)
                INIT_LIST_HEAD(&(tmp->buckets[i]));

        return tmp;
}

int htable_destroy(struct htable * table)
{
        unsigned int          i;
        struct list_head *    pos = NULL;
        struct list_head *    n = NULL;
        struct htable_entry * entry;

        assert(table);

        for (i = 0; i < table->buckets_size; i++) {
                list_for_each_safe(pos, n, &(table->buckets[i])) {
                        entry = list_entry(pos, struct htable_entry, next);
                        free(entry->val);
                        free(entry);
                }
        }

        free(table->buckets);
        free(table);

        return 0;
}

static uint64_t hash(uint64_t x)
{
        x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
        x = x ^ (x >> 31);

        return x;
}

static uint64_t calc_key(struct htable * table, uint64_t key)
{
        if (table->hash_key == true)
                key = hash(key);

        return (key & (table->buckets_size - 1));
}

int htable_insert(struct htable * table, uint64_t key, void * val)
{
        struct htable_entry * entry;
        uint64_t              lookup_key;
        struct list_head *    pos = NULL;

        assert(table);

        lookup_key = calc_key(table, key);

        list_for_each(pos, &(table->buckets[lookup_key])) {
                entry = list_entry(pos, struct htable_entry, next);
                if (entry->key == key)
                        return -1;
        }

        entry = malloc(sizeof(*entry));
        if (entry == NULL)
                return -ENOMEM;

        entry->key = key;
        entry->val = val;
        INIT_LIST_HEAD(&entry->next);

        list_add(&entry->next, &(table->buckets[lookup_key]));

        return 0;
}

void * htable_lookup(struct htable * table, uint64_t key)
{
        struct htable_entry * entry;
        struct list_head *    pos = NULL;
        uint64_t              lookup_key;

        assert(table);

        lookup_key = calc_key(table, key);

        list_for_each(pos, &(table->buckets[lookup_key])) {
                entry = list_entry(pos, struct htable_entry, next);
                if (entry->key == key)
                        return entry->val;
        }

        return NULL;
}

int htable_delete(struct htable * table, uint64_t key)
{
        struct htable_entry * entry;
        uint64_t              lookup_key;
        struct list_head *    pos = NULL;
        struct list_head *    n = NULL;

        assert(table);

        lookup_key = calc_key(table, key);

        list_for_each_safe(pos, n, &(table->buckets[lookup_key])) {
                entry = list_entry(pos, struct htable_entry, next);
                if (entry->key == key) {
                        list_del(&entry->next);
                        free(entry->val);
                        free(entry);
                        return 0;
                }
        }

        return -1;
}