/* gm-text-buffer-enhancer.c */

/* insert (c)/licensing information) */

#include "gm-text-buffer-enhancer.h"

#include <string.h>

static void gm_text_buffer_enhancer_class_init (GmTextBufferEnhancerClass* g_class);
static void gm_text_buffer_enhancer_init (GmTextBufferEnhancer* obj);
static void gm_text_buffer_enhancer_dispose (GObject* obj);
static void gm_text_buffer_enhancer_finalize (GObject* obj);

typedef struct _GmTextBufferEnhancerPrivate GmTextBufferEnhancerPrivate;
struct _GmTextBufferEnhancerPrivate {
  GtkTextBuffer* buffer;
  GSList* helpers;
};

#define GM_TEXT_BUFFER_ENHANCER_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
										 GM_TYPE_TEXT_BUFFER_ENHANCER, \
										 GmTextBufferEnhancerPrivate))

static GObjectClass* parent_class = NULL;

GType
gm_text_buffer_enhancer_get_type (void)
{
  static GType result = 0;
  if (!result) {
    static const GTypeInfo my_info = {
      sizeof(GmTextBufferEnhancerClass),
      NULL,		/* base init */
      NULL,		/* base finalize */
      (GClassInitFunc) gm_text_buffer_enhancer_class_init,
      NULL,		/* class finalize */
      NULL,		/* class data */
      sizeof(GmTextBufferEnhancer),
      1,		/* n_preallocs */
      (GInstanceInitFunc) gm_text_buffer_enhancer_init,
      NULL
    };
    result = g_type_register_static (G_TYPE_OBJECT,
				     "GmTextBufferEnhancer",
				     &my_info, 0);
  }
  return result;
}

static void
gm_text_buffer_enhancer_class_init (GmTextBufferEnhancerClass* g_class)
{
  GObjectClass* gobject_class = NULL;

  parent_class = g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*)g_class;
  gobject_class->dispose = gm_text_buffer_enhancer_dispose;
  gobject_class->finalize = gm_text_buffer_enhancer_finalize;

  g_type_class_add_private (gobject_class,
			    sizeof(GmTextBufferEnhancerPrivate));
}

static void
gm_text_buffer_enhancer_init (GmTextBufferEnhancer* obj)
{
  GmTextBufferEnhancerPrivate* priv = GM_TEXT_BUFFER_ENHANCER_GET_PRIVATE(obj);

  priv->buffer = NULL;
  priv->helpers = NULL;
}

static void
gm_text_buffer_enhancer_dispose (GObject* obj)
{
  GmTextBufferEnhancerPrivate* priv = GM_TEXT_BUFFER_ENHANCER_GET_PRIVATE(obj);

  if (priv->buffer != NULL) {

    g_object_unref (priv->buffer);
    priv->buffer = NULL;
  }

  if (priv->helpers != NULL) {

    g_slist_foreach (priv->helpers, (GFunc)g_object_unref, NULL);
    g_slist_free (priv->helpers);
    priv->helpers = NULL;
  }

  G_OBJECT_CLASS(parent_class)->dispose (obj);
}

static void
gm_text_buffer_enhancer_finalize (GObject* obj)
{
  G_OBJECT_CLASS(parent_class)->finalize (obj);
}

GmTextBufferEnhancer*
gm_text_buffer_enhancer_new (GtkTextBuffer* buffer)
{
  GmTextBufferEnhancer* result = NULL;
  GmTextBufferEnhancerPrivate* priv = NULL;

  g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

  result
    = (GmTextBufferEnhancer*)g_object_new(GM_TYPE_TEXT_BUFFER_ENHANCER, NULL);

  priv =  GM_TEXT_BUFFER_ENHANCER_GET_PRIVATE (result);
  g_object_ref (buffer);
  priv->buffer = buffer;

  return result;
}

void
gm_text_buffer_enhancer_add_helper (GmTextBufferEnhancer* self,
				    GmTextBufferEnhancerHelperIFace* helper)
{
  GmTextBufferEnhancerPrivate* priv = NULL;

  g_return_if_fail (GM_IS_TEXT_BUFFER_ENHANCER (self));
  g_return_if_fail (GM_IS_TEXT_BUFFER_ENHANCER_HELPER_IFACE (helper));

  priv = GM_TEXT_BUFFER_ENHANCER_GET_PRIVATE(self);
  g_object_ref (helper);
  priv->helpers = g_slist_prepend (priv->helpers, helper);
}

void
gm_text_buffer_enhancer_insert_text (GmTextBufferEnhancer* self,
				     GtkTextIter* iter,
				     const gchar* text,
				     gint len)
{
  GmTextBufferEnhancerPrivate* priv = NULL;
  gint position = 0;
  gint length = 0;
  GSList* active_tags = NULL;
  GmTextBufferEnhancerHelperIFace* best_helper = NULL;
  gint best_start = 0;
  gint best_length = 0;
  GSList* helper_ptr = NULL;
  GmTextBufferEnhancerHelperIFace* considered_helper = NULL;
  gint considered_start = 0;
  gint considered_length = 0;
  GSList* tag_ptr = NULL;
  GtkTextMark* mark = NULL;
  GtkTextIter tag_start_iter;

  g_return_if_fail (GM_IS_TEXT_BUFFER_ENHANCER (self));
  g_return_if_fail (iter != NULL); // can't do better
  g_return_if_fail (text != NULL);

  priv = GM_TEXT_BUFFER_ENHANCER_GET_PRIVATE (self);

  if (len < 0)
    length = strlen (text);
  else
    length = len;

  mark = gtk_text_buffer_create_mark (priv->buffer, NULL, iter, TRUE);

  while (position < length) {

    /* try to find the best helper,
     * starting for the worse case that none is good */
    best_helper = NULL;
    best_start = length;
    best_length = 0;
    for (helper_ptr = priv->helpers ;
	 helper_ptr != NULL ;
	 helper_ptr = g_slist_next (helper_ptr)) {

      considered_helper
	= GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE (helper_ptr->data);
      gm_text_buffer_enhancer_helper_check (considered_helper,
					    text, position,
					    &considered_start,
					    &considered_length);
      if (((considered_start < best_start)
	   && (considered_length > 0))
	  || ((considered_start == best_start)
	      && (considered_length > best_length))) {

	best_helper = considered_helper;
	best_start = considered_start;
	best_length = considered_length;
      }
    }

    /* whatever we found can be further down : just apply the tags to
     * the part of the text before */
    if (position < best_start) {

      gtk_text_buffer_move_mark (priv->buffer, mark, iter);
      gtk_text_buffer_insert (priv->buffer, iter,
			      text + position, best_start - position);
      gtk_text_buffer_get_iter_at_mark (priv->buffer, &tag_start_iter, mark);
      for (tag_ptr = active_tags;
	   tag_ptr != NULL;
	   tag_ptr = g_slist_next (tag_ptr)) {

	gtk_text_buffer_apply_tag (priv->buffer, GTK_TEXT_TAG (tag_ptr->data),
				   &tag_start_iter, iter);
      }
      position = best_start;
    }

    /* ok, now we're either at the end without a best helper or still in the
     * middle with a best helper : apply that one if it's there!
     */
    if (best_helper != NULL)
      gm_text_buffer_enhancer_helper_enhance (best_helper,
					      priv->buffer,
					      iter,
					      &active_tags,
					      text,
					      &position,
					      best_length);
  }

  gtk_text_buffer_delete_mark (priv->buffer, mark);
  g_slist_free (active_tags);
}
