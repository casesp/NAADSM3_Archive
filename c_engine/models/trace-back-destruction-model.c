/** @file trace-back-destruction-model.c
 * Module that simulates a policy of destroying units that have had contact
 * with a diseased unit. DEPRECATED: maintained ONLY for backward compatibility!
 *
 * NOTE: This module is DEPRECATED, and is included only for purposes of backward
 * compatibility with parameter files from NAADSM 3.0 - 3.1.x.  Any new 
 * development should be done elsewhere: see trace-model and contact-recorder.
 * 
 * This module has two responsibilities, detailed in the sections below.
 *
 * <b>Collecting trace back information</b>
 *
 * This module records all exposures that match the contact type (direct or
 * indirect) specified in the parameters.
 *
 * <b>Trace backs</b>
 *
 * When a unit is detected as diseased, this module identifies units to which
 * the diseased unit sent animals (direct contact) or products or material
 * (indirect contact).  This is tracing out.  The module requests the
 * destruction of each of those contact units.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date September 2003
 *
 * Copyright &copy; University of Guelph, 2003-2009
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * @todo Move tracking of exposures to the contact modules and create
 *   RequestForTrace and TraceResult events?
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define new trace_back_destruction_model_new
#define run trace_back_destruction_model_run
#define reset trace_back_destruction_model_reset
#define events_listened_for trace_back_destruction_model_events_listened_for
#define is_listening_for trace_back_destruction_model_is_listening_for
#define has_pending_actions trace_back_destruction_model_has_pending_actions
#define has_pending_infections trace_back_destruction_model_has_pending_infections
#define to_string trace_back_destruction_model_to_string
#define local_printf trace_back_destruction_model_printf
#define local_fprintf trace_back_destruction_model_fprintf
#define local_free trace_back_destruction_model_free
#define handle_before_any_simulations_event trace_back_destruction_model_handle_before_any_simulations_event
#define handle_new_day_event trace_back_destruction_model_handle_new_day_event
#define handle_exposure_event trace_back_destruction_model_handle_exposure_event
#define handle_detection_event trace_back_destruction_model_handle_detection_event

#include "model.h"
#include "model_util.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#include "trace-back-destruction-model.h"

#if !HAVE_ROUND && HAVE_RINT
#  define round rint
#endif

/* Temporary fix -- "round" and "rint" are in the math library on Red Hat 7.3,
 * but they're #defined so AC_CHECK_FUNCS doesn't find them. */
double round (double x);

#include "naadsm.h"

/** This must match an element name in the DTD. */
#define MODEL_NAME "trace-back-destruction-model"



#define NEVENTS_LISTENED_FOR 4
EVT_event_type_t events_listened_for[] =
  { EVT_BeforeAnySimulations, EVT_NewDay, EVT_Exposure, EVT_Detection };



/** Specialized information for this model. */
typedef struct
{
  NAADSM_contact_type contact_type;
  const char *contact_type_name;
  gboolean *production_type;
  GPtrArray *production_types;
  int priority;
  unsigned int nherds;          /* Number of units.  Stored here because it is also the length of the trace_out and trace_in lists. */
  double trace_success;         /* Probability of tracing a contact. */
  int trace_period;             /* Number of days back we are interesting in tracing back. */
  gboolean quarantine_only;
  GSList **trace_out;           /* Lists of exposures originating <i>from</i> each unit. */
  GHashTable *detections_today;
}
local_data_t;



/**
 * Wraps EVT_free_event so that it can be used in GLib calls.
 *
 * @param data a pointer to an EVT_event_t structure, but cast to a gpointer.
 * @param user_data not used, pass NULL.
 */
void
EVT_free_event_as_GFunc (gpointer data, gpointer user_data)
{
  EVT_free_event ((EVT_event_t *) data);
}



/**
 * Before any simulations, this module declares all the reasons for which it
 * may request a destruction.
 *
 * @param self this module.
 * @param queue for any new events the model creates.
 */
void
handle_before_any_simulations_event (struct naadsm_model_t_ * self,
                                     EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  GPtrArray *reasons;

#if DEBUG
  g_debug ("----- ENTER handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  reasons = g_ptr_array_sized_new (1);
  if (local_data->contact_type == NAADSM_DirectContact)
    g_ptr_array_add (reasons, "DirFwd");
  else if (local_data->contact_type == NAADSM_IndirectContact)
    g_ptr_array_add (reasons, "IndFwd");
  EVT_event_enqueue (queue, EVT_new_declaration_of_destruction_reasons_event (reasons));

  /* Note that we don't clean up the GPtrArray.  It will be freed along with
   * the declaration event after all interested sub-models have processed the
   * event. */

#if DEBUG
  g_debug ("----- EXIT handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * At the beginning of each new day, this module clears out its record of
 * yesterday's detections.
 *
 * @param self this module.
 */
void
handle_new_day_event (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_new_day_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  g_hash_table_remove_all (local_data->detections_today);

#if DEBUG
  g_debug ("----- EXIT handle_new_day_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Decides whether a record of one exposure is successfully traced or not.
 * Creates a TraceResult event either way.
 */
static void
handle_record (EVT_event_t *exposure_record,
               int day,
               local_data_t *local_data,
               RAN_gen_t *rng,
               EVT_event_queue_t *queue)
{
  EVT_exposure_event_t *trace;
  double r;
  
  trace = &(exposure_record->u.exposure);
  /* Is the trace successful?  Queue a trace result event either way, but set
   * the "traced" flag differently. */
  r = RAN_num (rng);
  EVT_event_enqueue (queue,
                     EVT_new_trace_result_event (trace->exposing_herd,
                                                 trace->exposed_herd,
                                                 trace->contact_type,
                                                 NAADSM_TraceForwardOrOut,
                                                 day, day, (r < local_data->trace_success)));
  if (r >= local_data->trace_success)
    {
#if DEBUG
      g_debug ("r (%.2f) >= P (%.2f) : unsuccessful trace", r, local_data->trace_success);
#endif
      ;
    }
  else
    {
#if DEBUG
      g_debug ("r (%.2f) < P (%.2f) : successful trace", r, local_data->trace_success);
#endif
      /* In the experimental version 'Riverton', "naturally immune" units have
       * died out and no longer exist, so they don't need to be destroyed. */
      if (
          trace->exposed_herd->status != Destroyed
          #ifdef RIVERTON
          && trace->exposed_herd->status != NaturallyImmune
          #endif
      )
        {
          if (local_data->quarantine_only)
            {
              HRD_quarantine (trace->exposed_herd);
            }
          else
            {
              EVT_event_t *destr_event;
              if (local_data->contact_type == NAADSM_DirectContact)
                {
                  destr_event = EVT_new_request_for_destruction_event
                    (trace->exposed_herd, day, "DirFwd", local_data->priority);
                }
              else if (local_data->contact_type == NAADSM_IndirectContact)
                {
                  destr_event = EVT_new_request_for_destruction_event
                    (trace->exposed_herd, day, "IndFwd", local_data->priority);
                }
              else
                {
                  g_error
                    ("An unrecognized contact type has occurred in %s.  This should never happen.  Please contact the developer.",
                     MODEL_NAME);
                }

              EVT_event_enqueue (queue, destr_event);
            } /* end of case where destruction (not just quarantine) is requested */
        } /* end of case where traced herd is not destroyed/dead */
    } /* end of case where trace is successful */

  return;
}



/**
 * Responds to a contact exposure event by copying the event and storing it in
 * this model's trace back list.  We check the production types and contact
 * type in deciding whether to store the event.
 *
 * @param self the model.
 * @param e an exposure event.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
handle_exposure_event (struct naadsm_model_t_ *self, EVT_event_t * e,
                       RAN_gen_t *rng, EVT_event_queue_t *queue)
{
  local_data_t *local_data;
  EVT_exposure_event_t *event;
  EVT_event_t *event_copy;
  unsigned int exposing_herd_index;

#if DEBUG
  g_debug ("----- ENTER handle_exposure_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  event = &(e->u.exposure);
  
  if (NAADSM_AirborneSpread != event->contact_type)
    {
      if (NAADSM_UnspecifiedInfectionType  == event->contact_type)
        g_error( "Contact type is unspecified in trace-back-destruction-model.handle_exposure_event" );
      else if (event->contact_type == local_data->contact_type
          && local_data->production_type[event->exposed_herd->production_type] == TRUE)
        {
    #if DEBUG
          g_debug ("recording exposure from unit \"%s\" -> unit \"%s\" on day %i",
                   event->exposing_herd->official_id, event->exposed_herd->official_id, event->day);
    #endif
          event_copy = EVT_clone_event (e);
    
          exposing_herd_index = event->exposing_herd->index;
          local_data->trace_out[exposing_herd_index] =
            g_slist_prepend (local_data->trace_out[exposing_herd_index], event_copy);
          if (g_hash_table_lookup (local_data->detections_today, event->exposing_herd) != NULL)
            handle_record (e, event->day, local_data, rng, queue);
        }
      else
        {
          #if DEBUG
            g_debug ("wrong contact type (%i, this sub-model concerned with %i)",
                   event->contact_type, local_data->contact_type);
          #endif
        }
    }

#if DEBUG
  g_debug ("----- EXIT handle_exposure_event (%s)", MODEL_NAME);
#endif
}



/**
 * Searches for outgoing exposures from the given herd.  This function also
 * cleans up exposure records that are older than what we need to store.
 */ 
static void
search_records (struct naadsm_model_t_ *self, HRD_herd_t * herd,
                int day, HRD_herd_list_t * herds,
                RAN_gen_t * rng, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  GSList *iter;
  EVT_exposure_event_t *trace;
  int days_ago;

#if DEBUG
  g_debug ("----- ENTER trace_back (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  EVT_event_enqueue (queue,
                     EVT_new_attempt_to_trace_event (herd, day,
                                                     local_data->contact_type,
                                                     NAADSM_TraceForwardOrOut,
                                                     local_data->trace_period));
  g_hash_table_insert (local_data->detections_today, herd, GINT_TO_POINTER(1));

  /* Trace out (find units that have received animals from this one).  Note
   * that this process may issue multiple TraceResults for the same pair of
   * herds if A has had contact with B several times in the time period of
   * interest. */
  for (iter = local_data->trace_out[herd->index]; iter != NULL; iter = g_slist_next (iter))
    {
      trace = &(((EVT_event_t *) (iter->data))->u.exposure);

      days_ago = day - trace->day;

      /**************************************************/
      /* This block is commented out to allow           */
      /* traces on the same day that detection occurs.  */
      /**************************************************
      if (days_ago == 0)
        {
         continue;
        }
      **************************************************/

      if (days_ago > local_data->trace_period)
        {
          /* Destroy all following trace records, because they're not needed
           * anymore.  (The current trace record isn't needed either, but we
           * don't delete it because this is a singly-linked list -- we can't
           * set the previous record's next pointer to null.) */
          g_slist_foreach (iter->next, EVT_free_event_as_GFunc, NULL);
          g_slist_free (iter->next);
          iter->next = NULL;

          break;
        }

#if DEBUG
      g_debug ("record of contact with unit \"%s\" on day %hu (%i days ago)",
               trace->exposed_herd->official_id, trace->day, days_ago);
#endif

      handle_record ((EVT_event_t *) (iter->data), day, local_data, rng, queue);
    } /* end of loop over trace records */

#if DEBUG
  g_debug ("----- EXIT trace_back (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a detection by ordering destruction actions.
 *
 * @param self the model.
 * @param herds the list of herds.
 * @param event a report event.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
handle_detection_event (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
                        EVT_detection_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
#if DEBUG
  g_debug ("----- ENTER handle_detection_event (%s)", MODEL_NAME);
#endif

  search_records (self, event->herd, event->day, herds, rng, queue);

#if DEBUG
  g_debug ("----- EXIT handle_detection_event (%s)", MODEL_NAME);
#endif
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
  g_debug ("----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_BeforeAnySimulations:
      handle_before_any_simulations_event (self, queue);
      break;
    case EVT_NewDay:
      handle_new_day_event (self);
      break;
    case EVT_Exposure:
      handle_exposure_event (self, event, rng, queue);
      break;
    case EVT_Detection:
      handle_detection_event (self, herds, &(event->u.detection), rng, queue);
      break;
    default:
      g_error
        ("%s has received a %s event, which it does not listen for.  This should never happen.  Please contact the developer.",
         MODEL_NAME, EVT_event_type_name[event->type]);
    }

#if DEBUG
  g_debug ("----- EXIT run (%s)", MODEL_NAME);
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
  local_data_t *local_data;
  unsigned int i;

#if DEBUG
  g_debug ("----- ENTER reset (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  for (i = 0; i < local_data->nherds; i++)
    {
      g_slist_foreach (local_data->trace_out[i], EVT_free_event_as_GFunc, NULL);
      g_slist_free (local_data->trace_out[i]);
      local_data->trace_out[i] = NULL;
    }

#if DEBUG
  g_debug ("----- EXIT reset (%s)", MODEL_NAME);
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
  g_string_sprintf (s, "<%s for %s exposures of ", MODEL_NAME, local_data->contact_type_name);
  already_names = FALSE;
  for (i = 0; i < local_data->production_types->len; i++)
    if (local_data->production_type[i] == TRUE)
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

  g_string_sprintfa (s, "\n  priority=%i\n", local_data->priority);
  g_string_sprintfa (s, "  trace-success=%g\n", local_data->trace_success);
  g_string_sprintfa (s, "  trace-period=%i>", local_data->trace_period);

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
 * Frees this model.  Does not free the contact type name or production type
 * names.
 *
 * @param self the model.
 */
void
local_free (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;
  unsigned int i;

#if DEBUG
  g_debug ("----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);
  g_free (local_data->production_type);

  for (i = 0; i < local_data->nherds; i++)
    {
      g_slist_foreach (local_data->trace_out[i], EVT_free_event_as_GFunc, NULL);
      g_slist_free (local_data->trace_out[i]);
    }
  g_free (local_data->trace_out);

  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new trace-back destruction model.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *m;
  local_data_t *local_data;
  scew_element const *e;
  scew_attribute *attr;
  XML_Char const *attr_text;
  gboolean success;

#if DEBUG
  g_debug ("----- ENTER new (%s)", MODEL_NAME);
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
  g_debug ("setting contact type");
#endif
  attr = scew_attribute_by_name (params, "contact-type");
  g_assert (attr != NULL);
  attr_text = scew_attribute_value (attr);
  if (strcmp (attr_text, "direct") == 0)
    local_data->contact_type = NAADSM_DirectContact;
  else if (strcmp (attr_text, "indirect") == 0)
    local_data->contact_type = NAADSM_IndirectContact;
  else
    g_assert_not_reached ();
  local_data->contact_type_name = NAADSM_contact_type_abbrev[local_data->contact_type];

#if DEBUG
  g_debug ("setting production types");
#endif
  local_data->production_types = herds->production_type_names;
  local_data->production_type =
    naadsm_read_prodtype_attribute (params, "production-type", herds->production_type_names);

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

  e = scew_element_by_name (params, "trace-success");
  if (e != NULL)
    {
      local_data->trace_success = PAR_get_probability (e, &success);
      if (success == FALSE)
        {
          g_warning ("%s: settting probability of trace success to 1", MODEL_NAME);
          local_data->trace_success = 1;
        }
    }
  else
    {
      g_warning ("%s: probability of trace success missing, setting to 1", MODEL_NAME);
      local_data->trace_success = 1;
    }

  e = scew_element_by_name (params, "trace-period");
  if (e != NULL)
    {
      local_data->trace_period = (int) round (PAR_get_time (e, &success));
      if (success == FALSE)
        {
          g_warning ("%s: setting period of interest to 1 week", MODEL_NAME);
          local_data->trace_period = 7;
        }
    }
  else
    {
      g_warning ("%s: period of interest missing, setting 1 to week", MODEL_NAME);
      local_data->trace_period = 7;
    }

  e = scew_element_by_name (params, "quarantine-only");
  if (e != NULL)
    {
      local_data->quarantine_only = PAR_get_boolean (e, &success);
      if (success == FALSE)
        {
          local_data->quarantine_only = FALSE;
        }
    }
  else
    {
      local_data->quarantine_only = FALSE;
    }

  local_data->nherds = HRD_herd_list_length (herds);

  /* No exposures have been tracked yet. */
  local_data->trace_out = g_new0 (GSList *, local_data->nherds);

  local_data->detections_today = g_hash_table_new (g_direct_hash, g_int_equal);

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return m;
}

/* end of file trace-back-destruction-model.c */
