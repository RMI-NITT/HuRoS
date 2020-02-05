/* Images
 *
 * GtkImage is used to display an image; the image can be in a number of formats.
 * Typically, you load an image into a GdkPixbuf, then display the pixbuf.
 *
 * This demo code shows some of the more obscure cases, in the simple
 * case a call to gtk_image_new_from_file() is all you need.
 *
 * If you want to put image data in your program as a C variable,
 * use the make-inline-pixbuf program that comes with GTK+.
 * This way you won't need to depend on loading external files, your
 * application binary can be self-contained.
 */

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <errno.h>
#include "demo-common.h"

static GtkWidget *window = NULL;
static GdkPixbufLoader *pixbuf_loader = NULL;
static guint load_timeout = 0;
static FILE* image_stream = NULL;

static void
progressive_prepared_callback (GdkPixbufLoader *loader,
                               gpointer         data)
{
  GdkPixbuf *pixbuf;
  GtkWidget *image;

  image = GTK_WIDGET (data);

  pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);

  /* Avoid displaying random memory contents, since the pixbuf
   * isn't filled in yet.
   */
  gdk_pixbuf_fill (pixbuf, 0xaaaaaaff);

  gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
}

static void
progressive_updated_callback (GdkPixbufLoader *loader,
                              gint                 x,
                              gint                 y,
                              gint                 width,
                              gint                 height,
                              gpointer     data)
{
  GtkWidget *image;

  image = GTK_WIDGET (data);

  /* We know the pixbuf inside the GtkImage has changed, but the image
   * itself doesn't know this; so queue a redraw.  If we wanted to be
   * really efficient, we could use a drawing area or something
   * instead of a GtkImage, so we could control the exact position of
   * the pixbuf on the display, then we could queue a draw for only
   * the updated area of the image.
   */

  gtk_widget_queue_draw (image);
}

static gint
progressive_timeout (gpointer data)
{
  GtkWidget *image;

  image = GTK_WIDGET (data);

  /* This shows off fully-paranoid error handling, so looks scary.
   * You could factor out the error handling code into a nice separate
   * function to make things nicer.
   */

  if (image_stream)
    {
      size_t bytes_read;
      guchar buf[256];
      GError *error = NULL;

      bytes_read = fread (buf, 1, 256, image_stream);

      if (ferror (image_stream))
        {
          GtkWidget *dialog;

          dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_ERROR,
                                           GTK_BUTTONS_CLOSE,
                                           "Failure reading image file 'alphatest.png': %s",
                                           g_strerror (errno));

          g_signal_connect (dialog, "response",
                            G_CALLBACK (gtk_widget_destroy), NULL);

          fclose (image_stream);
          image_stream = NULL;

          gtk_widget_show (dialog);

          load_timeout = 0;

          return FALSE; /* uninstall the timeout */
        }

      if (!gdk_pixbuf_loader_write (pixbuf_loader,
                                    buf, bytes_read,
                                    &error))
        {
          GtkWidget *dialog;

          dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_ERROR,
                                           GTK_BUTTONS_CLOSE,
                                           "Failed to load image: %s",
                                           error->message);

          g_error_free (error);

          g_signal_connect (dialog, "response",
                            G_CALLBACK (gtk_widget_destroy), NULL);

          fclose (image_stream);
          image_stream = NULL;

          gtk_widget_show (dialog);

          load_timeout = 0;

          return FALSE; /* uninstall the timeout */
        }

      if (feof (image_stream))
        {
          fclose (image_stream);
          image_stream = NULL;

          /* Errors can happen on close, e.g. if the image
           * file was truncated we'll know on close that
           * it was incomplete.
           */
          error = NULL;
          if (!gdk_pixbuf_loader_close (pixbuf_loader,
                                        &error))
            {
              GtkWidget *dialog;

              dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to load image: %s",
                                               error->message);

              g_error_free (error);

              g_signal_connect (dialog, "response",
                                G_CALLBACK (gtk_widget_destroy), NULL);

              gtk_widget_show (dialog);

              g_object_unref (pixbuf_loader);
              pixbuf_loader = NULL;

              load_timeout = 0;

              return FALSE; /* uninstall the timeout */
            }

          g_object_unref (pixbuf_loader);
          pixbuf_loader = NULL;
        }
    }
  else
    {
      gchar *filename;
      gchar *error_message = NULL;
      GError *error = NULL;

      /* demo_find_file() looks in the current directory first,
       * so you can run gtk-demo without installing GTK, then looks
       * in the location where the file is installed.
       */
      filename = demo_find_file ("alphatest.png", &error);
      if (error)
        {
          error_message = g_strdup (error->message);
          g_error_free (error);
        }
      else
        {
          image_stream = g_fopen (filename, "rb");
          g_free (filename);

          if (!image_stream)
            error_message = g_strdup_printf ("Unable to open image file 'alphatest.png': %s",
                                             g_strerror (errno));
        }

      if (image_stream == NULL)
        {
          GtkWidget *dialog;

          dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_ERROR,
                                           GTK_BUTTONS_CLOSE,
                                           "%s", error_message);
          g_free (error_message);

          g_signal_connect (dialog, "response",
                            G_CALLBACK (gtk_widget_destroy), NULL);

          gtk_widget_show (dialog);

          load_timeout = 0;

          return FALSE; /* uninstall the timeout */
        }

      if (pixbuf_loader)
        {
          gdk_pixbuf_loader_close (pixbuf_loader, NULL);
          g_object_unref (pixbuf_loader);
          pixbuf_loader = NULL;
        }

      pixbuf_loader = gdk_pixbuf_loader_new ();

      g_signal_connect (pixbuf_loader, "area-prepared",
                        G_CALLBACK (progressive_prepared_callback), image);

      g_signal_connect (pixbuf_loader, "area-updated",
                        G_CALLBACK (progressive_updated_callback), image);
    }

  /* leave timeout installed */
  return TRUE;
}

static void
start_progressive_loading (GtkWidget *image)
{
  /* This is obviously totally contrived (we slow down loading
   * on purpose to show how incremental loading works).
   * The real purpose of incremental loading is the case where
   * you are reading data from a slow source such as the network.
   * The timeout simply simulates a slow data source by inserting
   * pauses in the reading process.
   */
  load_timeout = gdk_threads_add_timeout (150,
                                progressive_timeout,
                                image);
}

static void
cleanup_callback (GObject   *objec                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               