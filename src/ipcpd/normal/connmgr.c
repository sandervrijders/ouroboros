/*
 * Ouroboros - Copyright (C) 2016 - 2017
 *
 * Handles AE connections
 *
 *    Dimitri Staessens <dimitri.staessens@ugent.be>
 *    Sander Vrijders   <sander.vrijders@ugent.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., http://www.fsf.org/about/contact/.
 */

#define _POSIX_C_SOURCE 200112L

#define OUROBOROS_PREFIX "normal-ipcp"

#include <ouroboros/dev.h>
#include <ouroboros/cacep.h>
#include <ouroboros/cdap.h>
#include <ouroboros/errno.h>
#include <ouroboros/list.h>
#include <ouroboros/logs.h>

#include "ae.h"
#include "connmgr.h"
#include "enroll.h"
#include "ipcp.h"
#include "ribmgr.h"

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct ae_conn {
        struct list_head next;
        struct conn      conn;
};

struct ae {
        struct list_head next;
        struct conn_info info;

        struct list_head conn_list;
        pthread_cond_t   conn_cond;
        pthread_mutex_t  conn_lock;
};

struct {
        pthread_t        acceptor;

        struct list_head aes;
        pthread_rwlock_t aes_lock;
} connmgr;


static int get_info_by_name(const char *       name,
                            struct conn_info * info)
{
        struct list_head * p;

        pthread_rwlock_rdlock(&connmgr.aes_lock);

        list_for_each(p, &connmgr.aes) {
                struct ae * ae = list_entry(p, struct ae, next);
                if (strcmp(ae->info.ae_name, name) == 0) {
                        memcpy(info, &ae->info, sizeof(*info));
                        pthread_rwlock_unlock(&connmgr.aes_lock);
                        return 0;
                }
        }

        pthread_rwlock_unlock(&connmgr.aes_lock);

        return -1;
}

static int add_ae_conn(const char *       name,
                       int                fd,
                       qosspec_t          qs,
                       struct conn_info * rcv_info)
{
        struct ae_conn *   ae_conn;
        struct list_head * p;
        struct ae *        ae = NULL;

        ae_conn = malloc(sizeof(*ae_conn));
        if (ae_conn == NULL) {
                log_err("Not enough memory.");
                return -1;
        }

        ae_conn->conn.conn_info    = *rcv_info;
        ae_conn->conn.flow_info.fd = fd;
        ae_conn->conn.flow_info.qs = qs;

        list_head_init(&ae_conn->next);

        pthread_rwlock_wrlock(&connmgr.aes_lock);

        list_for_each(p, &connmgr.aes) {
                ae = list_entry(p, struct ae, next);
                if (strcmp(ae->info.ae_name, name) == 0)
                        break;
        }

        /* Check if entry was removed during allocation. */
        if (ae == NULL || strcmp(ae->info.ae_name, name) != 0) {
                pthread_rwlock_unlock(&connmgr.aes_lock);
                return -1;
        }

        list_add(&ae_conn->next, &ae->conn_list);

        pthread_rwlock_unlock(&connmgr.aes_lock);

        return 0;
}

static void * flow_acceptor(void * o)
{
        int               fd;
        qosspec_t         qs;
        struct conn_info  snd_info;
        struct conn_info  rcv_info;
        struct conn_info  fail_info;

        (void) o;

        memset(&fail_info, 0, sizeof(fail_info));

        while (true) {
                if (ipcp_get_state() != IPCP_OPERATIONAL) {
                        log_info("Shutting down flow acceptor.");
                        return 0;
                }

                fd = flow_accept(&qs, NULL);
                if (fd < 0) {
                        if (fd != -EIRMD)
                                log_warn("Flow accept failed: %d", fd);
                        continue;
                }

                if (cacep_rcv(fd, &rcv_info)) {
                        log_err("Error establishing application connection.");
                        flow_dealloc(fd);
                        continue;
                }

                if (get_info_by_name(rcv_info.ae_name, &snd_info)) {
                        log_err("Failed to get info for %s.", rcv_info.ae_name);
                        cacep_snd(fd, &fail_info);
                        flow_dealloc(fd);
                        continue;
                }

                if (cacep_snd(fd, &snd_info)) {
                        log_err("Failed to respond to request.");
                        flow_dealloc(fd);
                        continue;
                }

                if (add_ae_conn(rcv_info.ae_name, fd, qs, &rcv_info)) {
                        log_err("Failed to add new connection.");
                        flow_dealloc(fd);
                        continue;
                }
        }

        return (void *) 0;
}

int connmgr_init(void)
{
        if (pthread_rwlock_init(&connmgr.aes_lock, NULL))
                return -1;

        list_head_init(&connmgr.aes);

        return 0;
}

int connmgr_start(void)
{
        if (pthread_create(&connmgr.acceptor, NULL, flow_acceptor, NULL))
                return -1;

        return 0;
}

void connmgr_stop(void)
{
        pthread_cancel(connmgr.acceptor);
}

static void destroy_ae(struct ae * ae)
{
        struct list_head * p;
        struct list_head * h;

        pthread_mutex_lock(&ae->conn_lock);

        list_for_each_safe(p, h, &ae->conn_list) {
                struct ae_conn * e = list_entry(p, struct ae_conn, next);
                list_del(&e->next);
                free(e);
        }

        pthread_mutex_unlock(&ae->conn_lock);

        pthread_cond_destroy(&ae->conn_cond);
        pthread_mutex_destroy(&ae->conn_lock);

        free(ae);
}

void connmgr_fini(void)
{
        struct list_head * p;
        struct list_head * h;

        pthread_join(connmgr.acceptor, NULL);

        pthread_rwlock_wrlock(&connmgr.aes_lock);

        list_for_each_safe(p, h, &connmgr.aes) {
                struct ae * e = list_entry(p, struct ae, next);
                list_del(&e->next);
                destroy_ae(e);
        }

        pthread_rwlock_unlock(&connmgr.aes_lock);

        pthread_rwlock_destroy(&connmgr.aes_lock);
}

struct ae * connmgr_ae_create(struct conn_info info)
{
        struct ae * ae;

        ae = malloc(sizeof(*ae));
        if (ae == NULL)
                goto fail_malloc;

        list_head_init(&ae->next);
        list_head_init(&ae->conn_list);

        ae->info = info;

        if (pthread_mutex_init(&ae->conn_lock, NULL))
                goto fail_mutex_init;

        if (pthread_cond_init(&ae->conn_cond, NULL))
                goto fail_cond_init;

        pthread_rwlock_wrlock(&connmgr.aes_lock);

        list_add(&ae->next, &connmgr.aes);

        pthread_rwlock_unlock(&connmgr.aes_lock);

        return ae;

 fail_cond_init:
        pthread_mutex_destroy(&ae->conn_lock);
 fail_mutex_init:
        free(ae);
 fail_malloc:
        return NULL;
}

void connmgr_ae_destroy(struct ae * ae)
{
        assert(ae);

        pthread_rwlock_wrlock(&connmgr.aes_lock);

        list_del(&ae->next);

        pthread_rwlock_unlock(&connmgr.aes_lock);

        destroy_ae(ae);
}

int connmgr_alloc(struct ae *   ae,
                  const char *  dst_name,
                  qosspec_t *   qs,
                  struct conn * conn)
{
        assert(ae);
        assert(dst_name);
        assert(conn);

        memset(&conn->conn_info, 0, sizeof(conn->conn_info));

        conn->flow_info.fd = flow_alloc(dst_name, qs, NULL);
        if (conn->flow_info.fd < 0) {
                log_err("Failed to allocate flow to %s.", dst_name);
                return -1;
        }

        if (qs != NULL)
                conn->flow_info.qs = *qs;
        else
                memset(&conn->flow_info.qs, 0, sizeof(conn->flow_info.qs));

        if (cacep_snd(conn->flow_info.fd, &ae->info)) {
                log_err("Failed to create application connection.");
                flow_dealloc(conn->flow_info.fd);
                return -1;
        }

        if (cacep_rcv(conn->flow_info.fd, &conn->conn_info)) {
                log_err("Failed to connect to application.");
                flow_dealloc(conn->flow_info.fd);
                return -1;
        }

        if (strcmp(ae->info.protocol, conn->conn_info.protocol) ||
            ae->info.pref_version != conn->conn_info.pref_version ||
            ae->info.pref_syntax != conn->conn_info.pref_syntax) {
                flow_dealloc(conn->flow_info.fd);
                return -1;
        }

        return 0;
}

int connmgr_wait(struct ae *   ae,
                 struct conn * conn)
{
        struct ae_conn * ae_conn;

        assert(ae);
        assert(conn);

        pthread_mutex_lock(&ae->conn_lock);

        pthread_cleanup_push((void(*)(void *))pthread_mutex_unlock,
                             (void *) &ae->conn_lock);

        while (list_is_empty(&ae->conn_list))
                pthread_cond_wait(&ae->conn_cond, &ae->conn_lock);

        pthread_cleanup_pop(false);

        ae_conn = list_first_entry((&ae->conn_list), struct ae_conn, next);
        if (ae_conn == NULL) {
                pthread_mutex_unlock(&ae->conn_lock);
                return -1;
        }

        *conn = ae_conn->conn;

        list_del(&ae_conn->next);
        free(ae_conn);

        pthread_mutex_unlock(&ae->conn_lock);

        return 0;
}
