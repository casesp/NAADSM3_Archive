/** @file ring-destruction-model.c
 * Module that simulates a policy of destroying units within a certain
 * distance of a diseased unit.
 *
 * When a unit is detected as diseased, this module requests the destruction of
 * all units within a given radius of the diseased unit.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @date September 2003
 *
 * Copyright &copy; University of Guelph, 2003-2009
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define new ring_destruction_model_new
#define run ring_destruction_model_run
#define reset ring_destruction_model_reset
#define events_listened_for ring_destruction_model_events_listened_for
#define is_listening_for ring_destruction_model_is_listening_for
#define has_pending_actions ring_destruction_model_has_pending_actions
#define has_pending_infections ring_destruction_model_has_pending_infections
#define to_string ring_destruction_model_to_string
#define local_printf ring_destruction_model_printf
#define local_fprintf ring_destruction_model_fprintf
#define local_free ring_destruction_model_free
#define handle_before_any_simulations_event ring_destruction_model_before_any_simulations_event
#define handle_detection_event ring_destruction_model_handle_detection_event
#define check_and_choose ring_destruction_model_check_and_choose

#include "model.h"
#include "model_util.h"
#include "gis.h"
#include "spatial_search.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#include "ring-destruction-model.h"

#if !HAVE_ROUND && HAVE_RINT
#  define round rint
#endif

/* Temporary fix -- "round" and "rint" are in the math library on Red Hat 7.3,
 * but they're #defined so AC_CHECK_FUNCS doesn't find them. */
double round (double x);

/** This must match an element name in the DTD. */
#define MODEL_NAME "ring-destruction-model"



#define NEVENTS_LISTENED_FOR 2
EVT_event_type_t events_listened_for[] = { EVT_BeforeAnySimulations, EVT_Detection };



#define EPSILON 0.01 /* 10 m */



/** Specialized information for this model. */
typedef struct
{
  gboolean *from_production_type, *to_production_type;
  GPtrArray *production_types;
  int priority;
  double radius;
}
local_data_t;



/**
 * Before any simulations, this module declares all the reasons for which it
 * may request a destruction.
 *
 * @param queue for any new events the model creates.
 */
void
handle_before_any_simulations_event (EVT_event_queue_t * queue)
{
  GPtrArray *reasons;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
         "----- ENTER handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif

  reasons = g_ptr_array_sized_new (1);
  g_ptr_array_add (reasons, "Ring");
  EVT_event_enqueue (queue, EVT_new_declaration_of_destruction_reasons_event (reasons));

  /* Note that we don't clean up the GPtrArray.  It will be freed along with
   * the declaration event after all interested sub-models have processed the
   * event. */

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
         "----- EXIT handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Special structure for use with the callback function below.
 */
typedef struct
{
  local_data_t *local_data;
  HRD_herd_list_t *herds;
  HRD_herd_t *herd1;
  int day;
  EVT_event_queue_t *queue;
} callback_t;



/**
 * Check whether herd 2 should be part of the destruction ring.
 */
void
check_and_choose (int id, gpointer arg)
{
  callback_t *callback_data;
  HRD_herd_t *herd2;
  HRD_herd_t *herd1;
  local_data_t *local_data;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER check_and_choose (%s)", MODEL_NAME);
#endif

  callback_data = (callback_t *) arg;
  herd2 = HRD_herd_list_get (callback_data->herds, id);

  /* Are herd 1 and herd 2 the same? */
  herd1 = callback_data->herd1;
  if (herd1 == herd2)
    goto end;

  local_data = callback_data->local_data;

  /* Is herd 2 the production type we're interested in, and not already
   * destroyed?
   *
   * In the experimental version 'Riverton', "naturally immune" units have
   * died out and no longer exist, so they don't need to be destroyed. */
  if (
      local_data->to_production_type[herd2->production_type] == FALSE
      || herd2->status == Destroyed
      #ifdef RIVERTON
      || herd2->status == NaturallyImmune
      #endif
  )
    goto end;

#if DEBUG
  g_debug ("unit \"%s\" within radius, ordering unit destroyed", herd2->official_id);
#endif
  EVT_event_enqueue (callback_data->queue,
                     EVT_new_request_for_destruction_event (herd2,
                                                            callback_data->day,
                                                            "Ring",
                                                            local_data->priority));

end:
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT check_and_choose (%s)", MODEL_NAME);
#endif
  return;
}



void
ring_destroy (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
              HRD_herd_t * herd, int day, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  callback_t callback_data;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER ring_destroy (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  callback_data.local_data = local_data;
  callback_data.herds = herds;
  callback_data.herd1 = herd;
  callback_data.day = day;
  callback_data.queue = queue;

  /* Find the distances to other units. */
  spatial_search_circle_by_id (herds->spatial_index, herd->index,
                               local_data->radius + EPSILON,
                               check_and_choose, &callback_data);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT ring_destroy (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a detection by ordering destruction actions.
 *
 * @param self the model.
 * @param herds a list of herds.
 * @param event a detection event.
 * @param queue for any new events the model creates.
 */
void
handle_detection_event (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
                        EVT_detection_event_t * event, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  HRD_herd_t *herd;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER handle_detection_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;

  if (local_data->from_production_type[herd->production_type] == TRUE)
    ring_destroy (self, herds, herd, event->day, queue);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT handle_detection_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Runs this model.
 *
 * @param self the model.
 * @param herds a herd list.
 * @param zones a zone list.
 * @param event the event that caused the model to run.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
run (struct naadsm_model_t_ *self, HRD_herd_list_t * herds, ZON_zone_list_t * zones,
     EVT_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_BeforeAnySimulations:
      handle_before_any_simulations_event (queue);
      break;
    case EVT_Detection:
      handle_detection_event (self, herds, &(event->u.detection), queue);
      break;
    default:
      g_error
        ("%s has received a %s event, which it does not listen for.  This should never happen.  Please contact the developer.",
         MODEL_NAME, EVT_event_type_name[event->type]);
    }

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT run (%s)", MODEL_NAME);
#endif
}



/**
 * Resets this model after a simulation run.
 *
 * @param self the model.
 */
void
reset (struct naadsm_model_t_ *self)
{
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER reset (%s)", MODEL_NAME);
#endif

  /* Nothing to do. */

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT reset (%s)", MODEL_NAME);
#endif
}



/**
 * Reports whether this model is listening for a given event type.
 *
 * @param self the model.
 * @param event_type an event type.
 * @return TRUE if the model is listening for the event type.
 */
gboolean
is_listening_for (struct naadsm_model_t_ *self, EVT_event_type_t event_type)
{
  int i;

  for (i = 0; i < self->nevents_listened_for; i++)
    if (self->events_listened_for[i] == event_type)
      return TRUE;
  return FALSE;
}



/**
 * Reports whether this model has any pending actions to carry out.
 *
 * @param self the model.
 * @return TRUE if the model has pending actions.
 */
gboolean
has_pending_actions (struct naadsm_model_t_ * self)
{
  return FALSE;
}



/**
 * Reports whether this model has any pending infections to cause.
 *
 * @param self the model.
 * @return TRUE if the model has pending infections.
 */
gboolean
has_pending_infections (struct naadsm_model_t_ * self)
{
  return FALSE;
}



/**
 * Returns a text representation of this model.
 *
 * @param self the model.
 * @return a string.
 */
char *
to_string (struct naadsm_model_t_ *self)
{
  GString *s;
  gboolean already_names;
  unsigned int i;
  char *chararray;
  local_data_t *local_data;

  local_data = (local_data_t *) (self->model_data);
  s = g_string_new (NULL);
  g_string_printf (s, "<%s for ", MODEL_NAME);
  already_names = FALSE;
  for (i = 0; i < local_data->production_types->len; i++)
    if (local_data->to_production_type[i] == TRUE)
      {
        if (already_names)
          g_string_append_printf (s, ",%s",
                                  (char *) g_ptr_array_index (local_data->production_types, i));
        else
          {
            g_string_append_printf (s, "%s",
                                    (char *) g_ptr_array_index (local_data->production_types, i));
            already_names = TRUE;
          }
      }

  g_string_append_printf (s, "\n  priority=%i\n", local_data->priority);
  g_string_append_printf (s, "  radius=%g>", local_data->radius);

  /* don't return the wrapper object */
  chararray = s->str;
  g_string_free (s, FALSE);
  return chararray;
}



/**
 * Prints this model to a stream.
 *
 * @param stream a stream to write to.
 * @param self the model.
 * @return the number of characters printed (not including the trailing '\\0').
 */
int
local_fprintf (FILE * stream, struct naadsm_model_t_ *self)
{
  char *s;
  int nchars_written;

  s = to_string (self);
  nchars_written = fprintf (stream, "%s", s);
  free (s);
  return nchars_written;
}



/**
 * Prints this model.
 *
 * @param self the model.
 * @return the number of characters printed (not including the trailing '\\0').
 */
int
local_printf (struct naadsm_model_t_ *self)
{
  return local_fprintf (stdout, self);
}



/**
 * Frees this model.  Does not free the production type names.
 *
 * @param self the model.
 */
void
local_free (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);
  g_free (local_data->from_production_type);
  g_free (local_data->to_production_type);
  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new ring destruction model.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *m;
  local_data_t *local_data;
  scew_element *e;
  scew_attribute *attr;
  gboolean success;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER new (%s)", MODEL_NAME);
#endif

  m = g_new (naadsm_model_t, 1);
  local_data = g_new (local_data_t, 1);

  m->name = MODEL_NAME;
  m->events_listened_for = events_listened_for;
  m->nevents_listened_for = NEVENTS_LISTENED_FOR;
  m->outputs = g_ptr_array_new ();
  m->model_data = local_data;
  m->run = run;
  m->reset = reset;
  m->is_listening_for = is_listening_for;
  m->has_pending_actions = has_pending_actions;
  m->has_pending_infections = has_pending_infections;
  m->to_string = to_string;
  m->printf = local_printf;
  m->fprintf = local_fprintf;
  m->free = local_free;

  /* Make sure the right XML subtree was sent. */
  g_assert (strcmp (scew_element_name (params), MODEL_NAME) == 0);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "setting \"from\" production types");
#endif
  local_data->production_types = herds->production_type_names;
  local_data->from_production_type =
    naadsm_read_prodtype_attribute (params, "from-production-type", herds->production_type_names);

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "setting \"to\" production types");
#endif
  /* Temporary support for older parameter files that only had a
   * "production-type" attribute and implied "from-any" functionality. */
  attr = scew_attribute_by_name (params, "production-type");
  if (attr != NULL)
    local_data->to_production_type =
      naadsm_read_prodtype_attribute (params, "production-type", herds->production_type_names);
  else
    local_data->to_production_type =
      naadsm_read_prodtype_attribute (params, "to-production-type", herds->production_type_names);

  e = scew_element_by_name (params, "priority");
  if (e != NULL)
    {
      local_data->priority = (int) round (PAR_get_unitless (e, &success));
      if (success == FALSE)
        {
          g_warning ("%s: setting priority to 1", MODEL_NAME);
          local_data->priority = 1;
        }
      if (local_data->priority < 1)
        {
          g_warning ("%s: priority cannot be less than 1, setting to 1", MODEL_NAME);
          local_data->priority = 1;
        }
    }
  else
    {
      g_warning ("%s: priority missing, setting to 1", MODEL_NAME);
      local_data->priority = 1;
    }

  e = scew_element_by_name (params, "radius");
  if (e != NULL)
    {
      local_data->radius = PAR_get_length (e, &success);
      if (success == FALSE)
        {
          g_warning ("%s: setting radius to 0", MODEL_NAME);
          local_data->radius = 0;
        }
      /* Radius must be positive. */
      if (local_data->radius < 0)
        {
          g_warning ("%s: radius cannot be negative, setting to 0", MODEL_NAME);
          local_data->radius = 0;
        }
    }
  else
    {
      g_warning ("%s: radius missing, setting to 0", MODEL_NAME);
      local_data->radius = 0;
    }

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT new (%s)", MODEL_NAME);
#endif

  return m;
}

/* end of file ring-destruction-model.c */
