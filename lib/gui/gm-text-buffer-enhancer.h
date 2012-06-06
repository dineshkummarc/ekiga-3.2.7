/* gm-text-buffer-enhancer.h */
/* insert (c)/licensing information) */

#ifndef __GM_TEXT_BUFFER_ENHANCER_H__
#define __GM_TEXT_BUFFER_ENHANCER_H__

#include <gtk/gtk.h>
#include "gm-text-buffer-enhancer-helper-iface.h"

G_BEGIN_DECLS

/* convenience macros */
#define GM_TYPE_TEXT_BUFFER_ENHANCER             (gm_text_buffer_enhancer_get_type())
#define GM_TEXT_BUFFER_ENHANCER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),GM_TYPE_TEXT_BUFFER_ENHANCER,GmTextBufferEnhancer))
#define GM_TEXT_BUFFER_ENHANCER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),GM_TYPE_TEXT_BUFFER_ENHANCER,GObject))
#define GM_IS_TEXT_BUFFER_ENHANCER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GM_TYPE_TEXT_BUFFER_ENHANCER))
#define GM_IS_TEXT_BUFFER_ENHANCER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GM_TYPE_TEXT_BUFFER_ENHANCER))
#define GM_TEXT_BUFFER_ENHANCER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),GM_TYPE_TEXT_BUFFER_ENHANCER,GmTextBufferEnhancerClass))

typedef struct _GmTextBufferEnhancer      GmTextBufferEnhancer;
typedef struct _GmTextBufferEnhancerClass GmTextBufferEnhancerClass;

struct _GmTextBufferEnhancer {
  GObject parent;
};

struct _GmTextBufferEnhancerClass {
  GObjectClass parent_class;
};

/* member functions */
GType gm_text_buffer_enhancer_get_type () G_GNUC_CONST;

GmTextBufferEnhancer* gm_text_buffer_enhancer_new (GtkTextBuffer* buffer);

void gm_text_buffer_enhancer_add_helper (GmTextBufferEnhancer* self,
					 GmTextBufferEnhancerHelperIFace* helper);

void gm_text_buffer_enhancer_insert_text (GmTextBufferEnhancer* self,
					  GtkTextIter* iter,
					  const gchar* text,
					  gint len);

G_END_DECLS

#endif /* __GM_TEXT_BUFFER_ENHANCER_H__ */
