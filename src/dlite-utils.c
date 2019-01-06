#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/compat.h"
#include "utils/err.h"
#include "utils/strtob.h"
#include "utils/tgen.h"
#include "getuuid.h"
#include "dlite.h"
#include "dlite-macros.h"



/********************************************************************
 * Utility functions
 ********************************************************************/

/*
  Writes an UUID to `buff` based on `id`.

  Whether and what kind of UUID that is generated depends on `id`:
    - If `id` is NULL or empty, a new random version 4 UUID is generated.
    - If `id` is not a valid UUID string, a new version 5 sha1-based UUID
      is generated from `id` using the DNS namespace.
    - Otherwise is `id` already a valid UUID and it is simply copied to
      `buff`.

  Length of `buff` must at least (DLITE_UUID_LENGTH + 1) bytes (36 bytes
  for UUID + NUL termination).

  Returns the UUID version if a new UUID is generated or zero if `id`
  is already a valid UUID.  On error, -1 is returned.
 */
int dlite_get_uuid(char *buff, const char *id)
{
  return getuuid(buff, id);
}


/*
  Returns an unique uri for metadata defined by `name`, `version`
  and `namespace` as a newly malloc()'ed string or NULL on error.

  The returned url is constructed as follows:

      namespace/version/name
 */
char *dlite_join_meta_uri(const char *name, const char *version,
                          const char *namespace)
{
  char *uri = NULL;
  size_t size = 0;
  size_t n = 0;
  if (name) {
    size += strlen(name);
    n++;
  }
  if (version) {
    size += strlen(version);
    n++;
  }
  if (namespace) {
    size += strlen(namespace);
    n++;
  }
  if ((n == 3) && (size > 0)) {
    size += 3;
    if (!(uri = malloc(size))) return err(1, "allocation failure"), NULL;
    snprintf(uri, size, "%s/%s/%s", namespace, version, name);
  }
  return uri;
}

/*
  Splits `metadata` uri into its components.  If `name`, `version` and/or
  `namespace` are not NULL, the memory they points to will be set to a
  pointer to a newly malloc()'ed string with the corresponding value.

  Returns non-zero on error.
 */
int dlite_split_meta_uri(const char *uri, char **name, char **version,
                         char **namespace)
{
  char *p, *q, *namep=NULL, *versionp=NULL, *namespacep=NULL;

  if (!(p = strrchr(uri, '/')))
    FAIL1("invalid metadata uri: '%s'", uri);
  q = p-1;
  while (*q != '/' && q > uri) q--;
  if (q == uri)
    FAIL1("invalid metadata uri: '%s'", uri);

  if (name) {
    if (!(namep = strdup(p + 1))) FAIL("allocation failure");
  }
  if (version) {
    int size = p - q;
    if (!(versionp = malloc(size))) FAIL("allocation failure");
    memcpy(versionp, q + 1, size - 1);
    versionp[size - 1] = '\0';
  }
  if (namespace) {
    int size = q - uri + 1;
    if (!(namespacep = malloc(size))) FAIL("allocation failure");
    memcpy(namespacep, uri, size - 1);
    namespacep[size - 1] = '\0';
  }

  if (name) *name = namep;
  if (version) *version = versionp;
  if (namespace) *namespace = namespacep;
  return 0;
 fail:
  if (namep) free(namep);
  if (versionp) free(versionp);
  if (namespacep) free(namespacep);
  return 1;
}


/*
  Parses the options string `options` and assign corresponding values
  of the array `opts`.  The options string should be a valid url query
  string of the form

      key1=value1;key2=value2...

  where the values are terminated by NUL or any of the characters in ";&#".
  A hash (#) terminates the options.

  `opts` should be a NULL-terminated DLiteOpt array initialised with
  default values.  At return, the values of the provided options are
  updated.

  If `modify` is non-zero, `options` is modifies such that all values in
  `opts` are NUL-terminated.  Otherwise they may be terminated by any of
  the characters in ";&#".

  Returns non-zero on error.
*/
int dlite_option_parse(char *options, DLiteOpt *opts, int modify)
{
  char *q, *p = options;
  if (!options) return 0;
  while (*p && *p != '#') {
    size_t i, len = strcspn(p, "=;&#");
    if (p[len] != '=')
      return errx(1, "no value for key '%.*s' in option string '%s'",
                  (int)len, p, options);
    for (i=0; opts[i].key; i++) {
      if (strncmp(opts[i].key, p, len) == 0 && strlen(opts[i].key) == len) {
        p += len;
        if (*p == '=') p++;
        opts[i].value = p;
        p += strcspn(p, ";&#");
        q = p;
        if (*p && strchr(";&", *p)) p++;
        if (modify) q[0] = '\0';
        break;
      }
    }
    if (!opts[i].key) {
      int len = strcspn(p, "=;&#");
      return errx(1, "unknown option key: '%.*s'", len, p);
    }
  }
  return 0;
}


/*
  Returns a newly allocated string to an url created by joining
  `driver`, `uri` and `options`.  `driver` and `options` may be NULL.
  Returns NULL on error.
 */
char *dlite_join_url(const char *driver, const char *uri, const char *options)
{
  TGenBuf s;
  tgen_buf_init(&s);
  if (driver) tgen_buf_append_fmt(&s, "%s://", driver);
  tgen_buf_append(&s, uri, -1);
  if (options) tgen_buf_append_fmt(&s, "?%s", options);
  return tgen_buf_steal(&s);
}


/*
  Splits `url` into three parts: `driver`, `uri` and `options`.  If
  `driver`, `uri` and/or `options` are not NULL, the pointers they
  points will be assigned to point within `url`.

  `url` will be modified.

  Returns non-zero on error.

  Note:
  URLs are assumed to have the following syntax (ref. [wikipedia]):

      URL = scheme:[//authority]path[?query][#fragment]

  where the authority component divides into three subcomponents:

      authority = [userinfo@]host[:port]

  This function maps `scheme` to `driver`, `[authority]path` to `uri` and
  `query` to `options`.

  [wikipedia]: https://en.wikipedia.org/wiki/URL
 */
int dlite_split_url(char *url, char **driver, char **uri, char **options)
{
  size_t i;
  char *p;

  /* strip off fragment */
  if ((p = strchr(url, '#'))) *p = '\0';

  /* strip off query and assign options */
  if ((p = strchr(url, '?'))) {
    *p = '\0';
    if (options) *options = (p[1]) ? p+1 : NULL;
  } else {
    if (options) *options = NULL;
  }

  /* assign driver and uri */
  i = strcspn(url, ":/");
  if (url[i] == ':') {
    url[i] = '\0';
    if (driver) *driver = url;
    if (url[i+1] == '/' && url[i+2] == '/')
      p = url + i + 3;
    else
      p = url + i + 1;
    if (uri) *uri = (p[0]) ? p : NULL;
  } else {
    if (driver) *driver = NULL;
    if (uri) *uri = (url[0]) ? url : NULL;
  }

  return 0;
}
