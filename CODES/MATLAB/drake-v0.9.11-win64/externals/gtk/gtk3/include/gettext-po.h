/* Public API for GNU gettext PO files - contained in libgettextpo.
   Copyright (C) 2003-2008, 2010 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _GETTEXT_PO_H
#define _GETTEXT_PO_H 1

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/* =========================== Meta Information ============================ */

/* Version number: (major<<16) + (minor<<8) + subminor */
#define LIBGETTEXTPO_VERSION 0x001202
extern __declspec (dllimport) int libgettextpo_version;

/* ================================= Types ================================= */

/* A po_file_t represents the contents of a PO file.  */
typedef struct po_file *po_file_t;

/* A po_message_iterator_t represents an iterator through a domain of a
   PO file.  */
typedef struct po_message_iterator *po_message_iterator_t;

/* A po_message_t represents a message in a PO file.  */
typedef struct po_message *po_message_t;

/* A po_filepos_t represents a string's position within a source file.  */
typedef struct po_filepos *po_filepos_t;

/* A po_error_handler handles error situations.  */
struct po_error_handler
{
  /* Signal an error.  The error message is built from FORMAT and the following
     arguments.  ERRNUM, if nonzero, is an errno value.
     Must increment the error_message_count variable declared in error.h.
     Must not return if STATUS is nonzero.  */
  void (*error) (int status, int errnum,
                 const char *format, ...)
#if ((__GNUC__ == 3 && __GNUC_MINOR__ >= 1) || __GNUC__ > 3) && !__STRICT_ANSI__
  __attribute__ ((__format__ (__printf__, 3, 4)))
#endif
  ;

  /* Signal an error.  The error message is built from FORMAT and the following
     arguments.  The error location is at FILENAME line LINENO. ERRNUM, if
     nonzero, is an errno value.
     Must increment the error_message_count variable declared in error.h.
     Must not return if STATUS is nonzero.  */
  void (*error_at_line) (int status, int errnum,
                         const char *filename, unsigned int lineno,
                         const char *format, ...)
#if ((__GNUC__ == 3 && __GNUC_MINOR__ >= 1) || __GNUC__ > 3) && !__STRICT_ANSI__
  __attribute__ ((__format__ (__printf__, 5, 6)))
#endif
  ;

  /* Signal a multiline warning.  The PREFIX applies to all lines of the
     MESSAGE.  Free the PREFIX and MESSAGE when done.  */
  void (*multiline_warning) (char *prefix, char *message);

  /* Signal a multiline error.  The PREFIX applies to all lines of the
     MESSAGE.  Free the PREFIX and MESSAGE when done.
     Must increment the error_message_count variable declared in error.h if
     PREFIX is non-NULL.  */
  void (*multiline_error) (char *prefix, char *message);
};
typedef const struct po_error_handler *po_error_handler_t;

/* A po_xerror_handler handles warnings, error and fatal error situations.  */
#define PO_SEVERITY_WARNING     0 /* just a warning, tell the user */
#define PO_SEVERITY_ERROR       1 /* an error, the operation cannot complete */
#define PO_SEVERITY_FATAL_ERROR 2 /* an error, the operation must be aborted */
struct po_xerror_handler
{
  /* Signal a problem of the given severity.
     MESSAGE and/or FILENAME + LINENO indicate where the problem occurred.
     If FILENAME is NULL, FILENAME and LINENO and COLUMN should be ignored.
     If LINENO is (size_t)(-1), LINENO and COLUMN should be ignored.
     If COLUMN is (size_t)(-1), it should be ignored.
     MESSAGE_TEXT is the problem description (if MULTILINE_P is true,
     multiple lines of text, each terminated with a newline, otherwise
     usually a single line).
     Must not return if SEVERITY is PO_SEVERITY_FATAL_ERROR.  */
  void (*xerror) (int severity,
                  po_message_t message,
                  const char *filename, size_t lineno, size_t column,
                  int multiline_p, const char *message_text);
  /* Signal a problem that refers to two messages.
     Similar to two calls to xerror.
     If possible, a "..." can be appended to MESSAGE_TEXT1 and prepended to
     MESSAGE_TEXT2.  */
  void (*xerror2) (int severity,
                   po_message_t message1,
                   const char *filename1, size_t lineno1, size_t column1,
                   int multiline_p1, const char *message_text1,
                   po_message_t message2,
                   const char *filename2, size_t lineno2, size_t column2,
                   int multiline_p2, const char *message_text2);
};
typedef const struct po_xerror_handler *po_xerror_handler_t;

/* Memory allocation:
   The memory allocations performed by these functions use xmalloc(),
   therefore will cause a program exit if memory is exhausted.
   The memory allocated by po_file_read, and implicitly returned through
   the po_message_* functions, lasts until freed with po_file_free.  */


/* ============================= po_file_t API ============================= */

/* Create an empty PO file representation in memory.  */
extern po_file_t po_file_create (void);

/* Read a PO file into memory.
   Return its contents.  Upon failure, return NULL and set errno.  */
#define po_file_read po_file_read_v3
extern po_file_t po_file_read (const char *filename,
                               po_xerror_handler_t handler);

/* Write an in-memory PO file to a file.
   Upon failure, return NULL and set errno.  */
#define po_file_write po_file_write_v2
extern po_file_t po_file_write (po_file_t file, const char *filename,
                                po_xerror_handler_t handler);

/* Free a PO file from memory.  */
extern void po_file_free (po_file_t file);

/* Return the names of the domains covered by a PO file in memory.  */
extern const char * const * po_file_domains (po_file_t file);


/* =========================== Header entry API ============================ */

/* Return the header entry of a domain of a PO file in memory.
   The domain NULL denotes the default domain.
   Return NULL if there is no header entry.  */
extern const char * po_file_domain_header (po_file_t file, const char *domain);

/* Return the value of a field in a header entry.
   The return value is either a freshly allocated string, to be freed by the
   caller, or NULL.  */
extern char * po_header_field (const char *header, const char *field);

/* Return the header entry with a given field set to a given value.  The field
   is added if necessary.
   The return value is a freshly allocated string.  */
extern char * po_header_set_field (const char *header, const char *field, const char *value);


/* ======================= po_message_iterator_t API ======================= */

/* Create an iterator for traversing a domain of a PO file in memory.
   The domain NULL denotes the default domain.  */
extern po_message_iterator_t po_message_iterator (po_file_t file, const char *domain);

/* Free an iterator.  */
extern void po_message_iterator_free (po_message_iterator_t iterator);

/* Return the next message, and advance the iterator.
   Return NULL at the end of the message list.  */
extern po_message_t po_next_message (po_message_iterator_t iterator);

/* Insert a message in a PO file in memory, in the domain and at the position
   indicated by the iterator.  The iterator thereby advances past the freshly
   inserted message.  */
extern void po_message_insert (po_message_iterator_t iterator, po_message_t message);


/* =========================== po_message_t API ============================ */

/* Return a freshly constructed message.
   To finish initializing the message, you must set the                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          