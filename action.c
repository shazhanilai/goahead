/*
    action.c -- Action handler

    This module implements the /action handler. It is a simple binding of URIs to C functions.
    This enables a very high performance implementation with easy parsing and decoding of query
    strings and posted data.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*********************************** Includes *********************************/

#include    "goahead.h"

/************************************ Locals **********************************/

static WebsHash formSymtab = -1;            /* Symbol table for form handlers */

/************************************* Code ***********************************/
/*
    Process an action request. Returns 1 always to indicate it handled the URL
 */
static bool actionHandler(Webs *wp)
{
    WebsKey     *sp;
    char        formBuf[BIT_LIMIT_FILENAME];
    char        *cp, *formName;
    WebsAction  fn;

    assure(websValid(wp));
    assure(formSymtab >= 0);

    /*
        Extract the form name
     */
    scopy(formBuf, sizeof(formBuf), wp->path);
    if ((formName = strchr(&formBuf[1], '/')) == NULL) {
        websError(wp, HTTP_CODE_NOT_FOUND, "Missing form name");
        return 1;
    }
    formName++;
    if ((cp = strchr(formName, '/')) != NULL) {
        *cp = '\0';
    }
    /*
        Lookup the C form function first and then try tcl (no javascript support yet).
     */
    sp = hashLookup(formSymtab, formName);
    if (sp == NULL) {
        websError(wp, HTTP_CODE_NOT_FOUND, "Action %s is not defined", formName);
    } else {
        fn = (WebsAction) sp->content.value.symbol;
        assure(fn);
        if (fn) {
#if BIT_LEGACY
            (*fn)((void*) wp, formName, wp->query);
#else
            (*fn)((void*) wp);
#endif
        }
    }
    return 1;
}


/*
    Define a function in the "action" map space
 */
int websDefineAction(char *name, void *fn)
{
    assure(name && *name);
    assure(fn);

    if (fn == NULL) {
        return -1;
    }
    hashEnter(formSymtab, name, valueSymbol(fn), 0);
    return 0;
}


static void closeAction()
{
    if (formSymtab != -1) {
        hashFree(formSymtab);
        formSymtab = -1;
    }
}


void websActionOpen()
{
    formSymtab = hashCreate(WEBS_HASH_INIT);
    websDefineHandler("action", actionHandler, closeAction, 0);
}


#if BIT_LEGACY
/*
    Don't use these routes. Use websWriteHeaders, websEndHeaders instead.

    Write a webs header. This is a convenience routine to write a common header for a form back to the browser.
 */
void websHeader(Webs *wp)
{
    assure(websValid(wp));

    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);
    websWrite(wp, "<html>\n");
}


void websFooter(Webs *wp)
{
    assure(websValid(wp));
    websWrite(wp, "</html>\n");
}
#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
