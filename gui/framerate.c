#include <stdbool.h>
#include <gtk/gtk.h>

#include "../worlds.h"

static bool repaint = true;

// Do this when the frame clock ticks:
gboolean
framerate_on_tick (GtkWidget *glarea)
{
	// Get current time:
	gint64 now = g_get_monotonic_time();

	// Feed timer tick to worlds, query repaint:
	repaint |= world_timer_tick(now);

	// Quit timer loop if canvas no longer exists:
	if (!glarea || !GTK_IS_WIDGET(glarea))
		return FALSE;

	// Yield if nothing to do:
	if (!repaint)
		return TRUE;

	// Else queue the widget for drawing:
	gtk_widget_queue_draw(glarea);
	repaint = false;

	return TRUE;
}

// Request a screen repaint on next timer tick:
void
framerate_repaint (void)
{
	repaint = true;
}
