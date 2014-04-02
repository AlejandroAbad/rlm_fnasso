/*
 * rlm_fnasso.c        module to dynamicaly add principals to a KDC after EAP-MSK negotiation
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * Copyright 2014  Alejandro Primitivo Abad <alejandroprimitivo.abad@um.es>
 */

#include	<freeradius-devel/ident.h>
#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>


#include <fnasso.h>
#include <stdarg.h>
#include <string.h>
#include "debug.h"



/*
 * Values of the attribute fields that carries the MSK.
 */
#define FNASSO_MS_MPPE_RECV 20381713
#define FNASSO_MS_MPPE_SEND 20381712

/*
 * Value of the vendor for fields with the MSK.
 */
#define FNASSO_MS_MPPE_VENDOR 311

/*
 * Module instance data structure.
 * All info that should be passed throught sucesive calls to this module must be stored in this structure.
 */
typedef struct rlm_fnasso_t
{
    char *kdm_realm;
    char *kdm_server;
    char *kdm_username;
    char *kdm_password;
} rlm_fnasso_t;


/*
 * Structure that allows freeradius to correctly parse fnasso's config file.
 * See http://wiki.freeradius.org/contributing/Modules2
 */
static const CONF_PARSER module_config[] = {
    { "kdm_realm", PW_TYPE_STRING_PTR, offsetof(rlm_fnasso_t, kdm_realm), NULL, NULL},
    { "kdm_server", PW_TYPE_STRING_PTR, offsetof(rlm_fnasso_t, kdm_server), NULL, NULL},
    { "kdm_username", PW_TYPE_STRING_PTR, offsetof(rlm_fnasso_t, kdm_username), NULL, NULL},
    { "kdm_password", PW_TYPE_STRING_PTR, offsetof(rlm_fnasso_t, kdm_password), NULL, NULL},
    { NULL, -1, 0, NULL, NULL}
};

/**
 * Module instantiation.
 * Executed when freeradius daemon starts.
 */
static int rlm_fnasso_instantiate(CONF_SECTION *conf, void **instance)
{
    F_BEGIN();

    // Malloc for instance data structure rlm_fnasso_t
    rlm_fnasso_t* data;
    data = rad_malloc(sizeof (rlm_fnasso_t));
    memset(data, 0, sizeof (rlm_fnasso_t));

    // Config file parsing.
    if (cf_section_parse(conf, data, module_config) < 0)
    {
	free(data);
	return -1;
    }

    *instance = data;

    RETURN(0);
}

/**
 * Module release.
 * Executed when freeradius daemon ends.
 */
static int rlm_fnasso_detach(void *instance)
{
    F_BEGIN()
    free(instance);
    RETURN(0);
}

/**
 * Post-proxy call.
 * When a proxyed request returns to the server, this function is called.
 */
static int rml_fnasso_postproxy(void *instance, REQUEST *request)
{
    F_BEGIN();

    rlm_fnasso_t *data;
    VALUE_PAIR *vp, *ms_mppe_recv, *ms_mppe_send;
    unsigned char *msk;
    int msk_len;
    fnasso_ctx *ctx;
    fnasso_error error;

    // Initialize a fnasso context using configuration data
    data = (rlm_fnasso_t *) instance;

    error = fnasso_context_init(&ctx);
    if (error) goto error_exit_point;

    error = fnasso_kadmin_set_realm(ctx, data->kdm_realm);
    if (error) goto error_exit_point;

    error = fnasso_kadmin_set_server(ctx, data->kdm_server);
    if (error) goto error_exit_point;

    error = fnasso_kadmin_set_admin(ctx, data->kdm_username, data->kdm_password);
    if (error) goto error_exit_point;

    // Try to find the AVP with the EAP-MSK in the post-proxyed reply
    // in order to find values of MS_MPPR_RECV and MS_MPPR_SEND.
    msk_len = 0;
    ms_mppe_send = NULL;
    ms_mppe_recv = NULL;
    vp = request->proxy_reply->vps;
    while (vp != NULL)
    {
	if (vp->vendor == FNASSO_MS_MPPE_VENDOR)
	{
	    if (vp->attribute == FNASSO_MS_MPPE_RECV)
	    {
		ms_mppe_recv = vp;
	    }
	    else if (vp->attribute == FNASSO_MS_MPPE_SEND)
	    {
		ms_mppe_send = vp;
	    }
	}
	vp = vp->next;
    }


    if (!ms_mppe_recv || !ms_mppe_send)
    {
	EVENT("MSK not found. Nothing to do here.");
	goto exit_point;
    }
    
    // Construct the MSK from the MS_MPPE_* chunks gathered
    msk_len = ms_mppe_recv->length + ms_mppe_send->length;
    msk = (unsigned char*) rad_malloc(msk_len);
    memcpy(msk, ms_mppe_send->data.strvalue, ms_mppe_send->length);
    memcpy(msk + ms_mppe_send->length, ms_mppe_recv->data.strvalue, ms_mppe_recv->length);


    EVENT("MSK found. Will generate temp username and password.");
    error = fnasso_user_set_with_msk(ctx, msk, msk_len);
    if (error) goto error_exit_point;

    EVENT("Creating new principal in kerberos database.");
    error = fnasso_kadmin_create_user(ctx);
    if (error) goto error_exit_point;

    EVENT("Kerberos principal suscessfully created.");
    goto exit_point;

error_exit_point:
exit_point :
    fnasso_context_free(&ctx);
    RETURN(RLM_MODULE_OK);
}


module_t rlm_fnasso = {
    RLM_MODULE_INIT,
    "fnasso",
    RLM_TYPE_THREAD_SAFE, // type: thread safe. No shared memory is used
    rlm_fnasso_instantiate, // instantiation
    rlm_fnasso_detach, // detach
    {
        NULL, // authenticate
        NULL, // authorize
        NULL, // pre-accounting
        NULL, // accounting
        NULL, // checksimul
        NULL, // pre-proxy
        rml_fnasso_postproxy, // post-proxy
        NULL // post-auth
    },
};
