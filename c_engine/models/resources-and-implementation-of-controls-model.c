/** @file resources-and-implementation-of-controls-model.c
 * Module that simulates the actions and resources of government authorities in
 * an outbreak.
 *
 * This module has several responsibilities, detailed in the sections below.
 *
 * <b>Identifying an outbreak</b>
 *
 * When this module hears the first Detection event, it
 * <ol>
 *   <li>
 *     announces a PublicAnnouncement event
 *   <li>
 *     starts counting down days until a destruction program can begin
 *   <li>
 *     starts counting detections until a vaccination program begins
 * </ol>
 *
 * <b>Vaccinating units</b>
 *
 * This module picks up RequestForVaccination events and announces
 * CommitmentToVaccinate events.  The unit is placed in a waiting list (queue)
 * with a priority.  The first day on which a unit can be vaccinated is the
 * beginning of the authorities' vaccination program <em>or</em> the day after
 * the request for vaccination is made, whichever is later.
 *
 * See below for an explanation of how priorities work.
 *
 * If a unit to be vaccinated was vaccinated recently (within the number of
 * days specified in the parameters) the request will be discarded.
 *
 * <b>Destroying units</b>
 *
 * This module picks up RequestForDestruction events and announces
 * CommitmentToDestroy events.  The unit is placed in a waiting list (queue)
 * with a priority.  The first day on which a unit can be destroyed is the
 * beginning of the authorities' destruction program <em>or</em> the day after
 * the request for destruction is made, whichever is later.
 *
 * NB: This module never announces RequestForDestruction or
 * RequestForVaccination events.  Other modules (e.g.,
 * basic-destruction-model.c, ring-destruction-model.c) simulate policies that
 * decide which units are destroyed or vaccinated.  Those policy modules can be
 * included or excluded from simulations to try out different combinations of
 * policies.
 *
 * <b>The priority system</b>
 *
 * Vaccination and destruction follow a strict priority order.  Units on a
 * waiting list are prioritized by <i>production type</i>, <i>reason</i> for
 * vaccination or destruction, and <i>time waiting</i>.
 *
 * The model description document has a discussion with examples of how the
 * priority system works.  The discussion below is about how it is implemented
 * in code.  (It specifically discusses destruction, but the same points apply
 * to vaccination.)
 *
 * The implementation works like this: each request for destruction is placed
 * in one of several queues.  The order of the queues takes care of
 * prioritizing by <i>production type</i> and <i>reason</i>.  The queues are
 * processed in slightly different ways to take into account <i>time
 * waiting</i>.
 *
 * Consider a scenario where there are 3 production types (cattle, pigs, sheep)
 * and 4 possible reasons for destruction (basic, ring, trace direct, trace
 * indirect).  Suppose that time waiting is in last place, as in
 *
 * production type (cattle > pigs > sheep) > reason (basic > trace direct >
 * ring > trace indirect) > time waiting
 *
 * There would be 12 queues, numbered as follows:
 * -# basic destruction / cattle
 * -# trace direct destruction / cattle
 * -# ring destruction / cattle
 * -# trace indirect destruction / cattle
 * -# basic destruction / pigs
 * -# trace direct destruction / pigs
 * -# ring destruction / pigs
 * -# trace indirect destruction / pigs
 * -# basic destruction / sheep
 * -# trace direct destruction / sheep
 * -# ring destruction / sheep
 * -# trace indirect destruction / sheep
 *
 * On each day, this module will destroy every unit on list 1, then every unit
 * on list 2, etc., until the destruction capacity runs out.  Every waiting
 * cattle unit will be destroyed before any waiting pig unit, and among cattle
 * units, every unit that was chosen by the "basic" destruction rule will be
 * destroyed before any unit that was chosen by the "trace" destruction rule.
 * Time waiting is taken care of by the fact that each of the 12 lists is a
 * queue: new requests enter the queue at one end, requests that have been
 * waiting the longest pop out the other end.
 *
 * If reason for destruction is given priority over production type, as in
 *
 * reason (basic > trace direct > ring > trace indirect) > production type
 * (cattle > pigs > sheep) > time waiting
 *
 * The order of the queues would be:
 * -# basic destruction / cattle
 * -# basic destruction / pigs
 * -# basic destruction / sheep
 * -# trace direct destruction / cattle
 * -# trace direct destruction / pigs
 * -# trace direct destruction / sheep
 * -# ring destruction / cattle
 * -# ring destruction / pigs
 * -# ring destruction / sheep
 * -# trace indirect destruction / cattle
 * -# trace indirect destruction / pigs
 * -# trace indirect destruction / sheep
 *
 * Again, this situation is handled by destroying every unit on list 1, then
 * every unit on list 2, etc.
 *
 * Now suppose that time waiting is given first priority:
 *
 * time waiting > reason (basic > trace direct > ring > trace indirect) >
 * production type (cattle > pigs > sheep)
 *
 * The order of the queues would still be:
 * -# basic destruction / cattle
 * -# basic destruction / pigs
 * -# basic destruction / sheep
 * -# trace direct destruction / cattle
 * -# trace direct destruction / pigs
 * -# trace direct destruction / sheep
 * -# ring destruction / cattle
 * -# ring destruction / pigs
 * -# ring destruction / sheep
 * -# trace indirect destruction / cattle
 * -# trace indirect destruction / pigs
 * -# trace indirect destruction / sheep
 *
 * But what the program will do in this case is scan all 12 queues for the one
 * with the unit that has been waiting the longest, destroy that unit, and
 * repeat.  If 2 queues contain units that have been waiting for the same
 * amount of time, the one higher-up in the queue order is taken, thus making
 * reason and production type the 2nd and 3rd priorities.
 *
 * If time waiting is given second priority:
 *
 * reason (basic > trace direct > ring > trace indirect) > time waiting >
 * production type (cattle > pigs > sheep)
 *
 * The order of the queues would be exactly as above again:
 * -# | basic destruction / cattle
 * -# | basic destruction / pigs
 * -# | basic destruction / sheep
 * -# trace direct destruction / cattle
 * -# trace direct destruction / pigs
 * -# trace direct destruction / sheep
 * -# ring destruction / cattle
 * -# ring destruction / pigs
 * -# ring destruction / sheep
 * -# trace indirect destruction / cattle
 * -# trace indirect destruction / pigs
 * -# trace indirect destruction / sheep
 *
 * The program will scan the first 3 queues for the one with the unit that has
 * been waiting the longest, destroy that unit, and repeat.  If the first 3
 * queues are empty and destruction capacity still remains, the program will
 * move on to the next 3 queues.
 *
 * As a final example, if you want the same priorities:
 *
 * reason (basic > trace direct > ring > trace indirect) > time waiting >
 * production type (cattle > pigs > sheep)
 *
 * but you wish to exclude sheep from destruction, the order of the queues
 * would be:
 * -# | basic destruction / cattle
 * -# | basic destruction / pigs
 * -# |
 * -# trace direct destruction / cattle
 * -# trace direct destruction / pigs
 * -#
 * -# ring destruction / cattle
 * -# ring destruction / pigs
 * -#
 * -# trace indirect destruction / cattle
 * -# trace indirect destruction / pigs
 * -# &nbsp;
 *
 * Note the queue numbers.  This is done so that the program still knows how to
 * group the lists: 3 production types means group by 3's.
 *
 * There is another wrinkle to the data structures that maintainers should be
 * aware of.  The queues shown above are GQueue objects, which store
 * RequestForDestruction objects, as shown in the diagram below.  There are
 * additional pointers into the GQueue, stored in an array called
 * destruction_status.  The destruction_status array holds one pointer per
 * unit.  If the unit is awaiting destruction, the pointer points to the GList
 * node that stores the request to destroy that unit; if the unit is not
 * awaiting destruction, the pointer is null.
 *
 * @image html priority_queues.png
 *
 * The destruction_status array is useful for quickly finding out whether a
 * unit is awaiting destruction, so that when another request is made to
 * destroy that unit, and the new request has higher priority, the old request
 * can be discarded.  The corresponding array for vaccination is also useful
 * when a unit that is awaiting vaccination is destroyed and the request for
 * vaccination can be discarded.  The destruction_status and vaccination_status
 * arrays must be updated whenever requests are pushed into or popped from the
 * queues.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   School of Computer Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date June 2003
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
#define new resources_and_implementation_of_controls_model_new
#define run resources_and_implementation_of_controls_model_run
#define reset resources_and_implementation_of_controls_model_reset
#define events_listened_for resources_and_implementation_of_controls_model_events_listened_for
#define is_listening_for resources_and_implementation_of_controls_model_is_listening_for
#define has_pending_actions resources_and_implementation_of_controls_model_has_pending_actions
#define has_pending_infections resources_and_implementation_of_controls_model_has_pending_infections
#define to_string resources_and_implementation_of_controls_model_to_string
#define local_printf resources_and_implementation_of_controls_model_printf
#define local_fprintf resources_and_implementation_of_controls_model_fprintf
#define local_free resources_and_implementation_of_controls_model_free
#define handle_new_day_event resources_and_implementation_of_controls_model_handle_new_day_event
#define handle_declaration_of_destruction_reasons_event resources_and_implementation_of_controls_model_handle_declaration_of_destruction_reasons_event
#define handle_declaration_of_vaccination_reasons_event resources_and_implementation_of_controls_model_handle_declaration_of_vaccination_reasons_event
#define handle_detection_event resources_and_implementation_of_controls_model_handle_detection_event
#define handle_request_for_destruction_event resources_and_implementation_of_controls_model_handle_request_for_destruction_event
#define handle_request_for_vaccination_event resources_and_implementation_of_controls_model_handle_request_for_vaccination_event
#define handle_request_for_zone_focus_event resources_and_implementation_of_controls_model_handle_request_for_zone_focus_event
#define handle_vaccination_event resources_and_implementation_of_controls_model_handle_vaccination_event

#include "model.h"
#include "model_util.h"
#include <limits.h>

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#include "naadsm.h"

#include "resources-and-implementation-of-controls-model.h"

#if !HAVE_ROUND && HAVE_RINT
#  define round rint
#endif

/* Temporary fix -- "round" and "rint" are in the math library on Red Hat 7.3,
 * but they're #defined so AC_CHECK_FUNCS doesn't find them. */
double round (double x);

/** This must match an element name in the DTD. */
#define MODEL_NAME "resources-and-implementation-of-controls-model"



#define NEVENTS_LISTENED_FOR 8
EVT_event_type_t events_listened_for[] =
  { EVT_NewDay, EVT_Detection, EVT_DeclarationOfDestructionReasons,
  EVT_RequestForDestruction, EVT_DeclarationOfVaccinationReasons,
  EVT_RequestForVaccination, EVT_Vaccination, EVT_RequestForZoneFocus
};



#define DESTROYED 0



/** Specialized information for this model. */
typedef struct
{
  unsigned int nherds;          /* Number of herds. */
  unsigned int nprod_types;     /* Number of production types. */

  gboolean outbreak_known; /**< TRUE once the authorities are aware of the
    outbreak; FALSE otherwise. */
  int first_detection_day; /** The day of the first detection.  Only defined if
    outbreak_known is TRUE. */

  /* Parameters concerning destruction. */
  unsigned int ndestruction_reasons; /**< Number of distinct reasons for
    destruction. */
  GPtrArray *destruction_reasons; /**< A temporary array used when counting the
    number of distinct reasons for destruction.  It stores the reasons declared
    so far, so that they will not be double-counted. */
  int destruction_program_delay; /**< The number of days between
    recognizing and outbreak and beginning a destruction program. */
  int destruction_program_begin_day; /**< The day of the simulation on which
    the destruction program begins.  Only defined if outbreak_known is TRUE. */
  REL_chart_t *destruction_capacity; /**< The maximum number of herds the
    authorities can destroy in a day. */
  gboolean destruction_capacity_goes_to_0; /**< A flag indicating that at some
    point destruction capacity drops to 0 and remains there. */
  int destruction_capacity_0_day; /**< The day on which the destruction
    capacity drops to 0.  Only defined if destruction_capacity_goes_to_0 =
    TRUE. */
  gboolean no_more_destructions; /**< A flag indicating that on this day and
    forward, there is no capacity to do destructions.  Useful for deciding
    whether a simulation can exit early even if there destructions queued up. */
  GList **destruction_status; /**< A pointer for each unit.  If the unit is not
    awaiting destruction, the pointer will be NULL.  If the unit is awaiting
    destruction, the pointer is to a node in pending_destructions.  Note that
    the pointer is to a GList structure in a GQueue; the actual
    RequestForDestruction event can be found by following the GList structure's
    "data" pointer. */
  unsigned int nherds_destroyed_today; /**< The number of herds the authorities
    have destroyed on a given day. */
  GPtrArray *pending_destructions; /**< Prioritized lists of herds to be
    destroyed.  Each item is a pointer to a GQueue, and each item in the GQueue
    is a RequestForDestruction event. */
  int destruction_prod_type_priority;
  int destruction_time_waiting_priority;
  int destruction_reason_priority;
  GHashTable *destroyed_today; /**< Records the units destroyed today.  Useful
    for ignoring new requests for destruction or vaccination coming in from
    other modules that don't know which units are being destroyed today.  The
    key is a herd pointer (HRD_herd_t *) and the value is unimportant (presence
    or absence of a key is all we ever test). */

  /* Parameters concerning vaccination. */
  unsigned int nvaccination_reasons; /**< Number of distinct reasons for
    vaccination. */
  GPtrArray *vaccination_reasons; /**< A temporary array used when counting the
    number of distinct reasons for vaccination.  It stores the reasons declared
    so far, so that they will not be double-counted. */
  unsigned int vaccination_program_threshold; /**< The number of diseased herds
    that must be detected before vaccination will begin. */
  GHashTable *detected_herds; /**< The diseased herds detected so far.  Keys
    are herds (HRD_herd_t *), values are unimportant (presence of a key is all
    we ever test). */
  unsigned int ndetected_herds; /**< The number of diseased herds detected so
    far. */
  REL_chart_t *vaccination_capacity; /**< The maximum number of herds the
    authorities can vaccinate in a day. */
  gboolean vaccination_capacity_goes_to_0; /**< A flag indicating that at some
    point vaccination capacity drops to 0 and remains there. */
  int vaccination_capacity_0_day; /**< The day on which the vaccination
    capacity drops to 0.  Only defined if vaccination_capacity_goes_to_0 =
    TRUE. */
  gboolean no_more_vaccinations; /**< A flag indicating that on this day and
    forward, there is no capacity to do vaccinations.  Useful for deciding
    whether a simulation can exit early even if there vaccinations queued up. */
  GHashTable *vaccination_status; /**< A hash table keyed by pointers to units.
    If a unit is not awaiting vaccination, it will not be present in the table.
    If a unit is awaiting vaccination, its associated data item will be a
    GQueue storing pointers to nodes in pending_vaccinations.  (A unit can be
    in pending_vaccinations more than once.) */
  unsigned int nherds_vaccinated_today; /**< The number of herds the
    authorities have vaccinated on a given day. */
  GPtrArray *pending_vaccinations; /**< Prioritized lists of herds to be
    vaccinated.  Each item is a pointer to a GQueue, and each item in the
    GQueue is a RequestForVaccination event. */
  int *day_last_vaccinated; /**< Records the day when each herd
   was last vaccinated.  Also prevents double-counting units against the
   vaccination capacity. */
  int *min_next_vaccination_day;
  int vaccination_prod_type_priority;
  int vaccination_time_waiting_priority;
  int vaccination_reason_priority;
  GHashTable *detected_today; /**< Records the units detected today.  Useful
    for cancelling vaccination of detected units. */
}
local_data_t;



/**
 * Cancels a vaccination.
 *
 * @param herd a herd.
 * @param day the current simulation day.
 * @param vaccination_status from local_data.
 * @param pending_vaccinations from local_data.
 * @param queue for any new events the function creates.
 */
void
cancel_vaccination (HRD_herd_t * herd, int day,
                    GHashTable * vaccination_status, GPtrArray * pending_vaccinations,
                    EVT_event_queue_t * queue)
{
  GQueue *locations_in_queue;
  GList *link;
  EVT_event_t *request;
  EVT_request_for_vaccination_event_t *details;
  GQueue *q;
  EVT_event_t *cancellation_event;
  
#if DEBUG
  g_debug ("----- ENTER cancel_vaccination (%s)", MODEL_NAME);
#endif
    
  /* If the unit is on the vaccination waiting list, remove it. */
  locations_in_queue = (GQueue *) g_hash_table_lookup (vaccination_status, herd);
  if (locations_in_queue != NULL)
    {
      while (!g_queue_is_empty (locations_in_queue))
        {
          link = (GList *) g_queue_pop_head (locations_in_queue);
          /* Delete both the RequestForVaccination structure and the GQueue link
           * that holds it. */
          request = (EVT_event_t *) (link->data);
          details = &(request->u.request_for_vaccination);
          cancellation_event = EVT_new_vaccination_canceled_event (herd, day, details->day_commitment_made);

          q = (GQueue *) g_ptr_array_index (pending_vaccinations, details->priority - 1);
          EVT_free_event (request);
          g_queue_delete_link (q, link);

          EVT_event_enqueue (queue, cancellation_event);
        }
      g_hash_table_remove (vaccination_status, herd);
    }

#if DEBUG
  g_debug ("----- EXIT cancel_vaccination (%s)", MODEL_NAME);
#endif
  return;
}



void
destroy (struct naadsm_model_t_ *self, HRD_herd_t * herd,
         int day, char *reason, int day_commitment_made,
         EVT_event_queue_t * queue)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER destroy (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* Destroy the unit. */

  #ifdef RIVERTON
  /* In Riverton, if the unit is already Destroyed or NaturallyImmune, 
   * it should not be destroyed again. */
  g_assert ( (herd->status != Destroyed) && (herd->status != NaturallyImmune) );
  #else
  /* In the standard version, if the unit is already Destroyed, it should not
   * be destroyed again. */
  g_assert (herd->status != Destroyed);
  #endif

#if DEBUG
  g_debug ("destroying unit \"%s\"", herd->official_id);
#endif
  HRD_destroy (herd);
  EVT_event_enqueue (queue, EVT_new_destruction_event (herd, day, reason, day_commitment_made));
  g_hash_table_insert (local_data->destroyed_today, herd, herd);
  local_data->nherds_destroyed_today++;

  /* Take the unit off the vaccination waiting list, if needed. */
  cancel_vaccination (herd, day, local_data->vaccination_status,
                      local_data->pending_vaccinations, queue);

#if DEBUG
  g_debug ("----- EXIT destroy (%s)", MODEL_NAME);
#endif
  return;
}



void
destroy_by_priority (struct naadsm_model_t_ *self, int day,
                     EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  unsigned int destruction_capacity;
  EVT_event_t *pending_destruction;
  EVT_request_for_destruction_event_t *details;
  unsigned int npriorities;
  unsigned int priority;
  int request_day, oldest_request_day;
  int oldest_request_index;
  GQueue *q;

#if DEBUG
  g_debug ("----- ENTER destroy_by_priority (%s)", MODEL_NAME);
#endif

  /* Eliminate compiler warnings about uninitialized values */
  oldest_request_day = INT_MAX;

  local_data = (local_data_t *) (self->model_data);

  /* Look up the destruction capacity (which may increase as the outbreak
   * progresses). */
  destruction_capacity =
    (unsigned int)
    round (REL_chart_lookup
           (day - local_data->first_detection_day - 1, local_data->destruction_capacity));

  /* Check whether the destruction capacity has dropped to 0 for good. */
  if (local_data->destruction_capacity_goes_to_0
      && (day - local_data->first_detection_day - 1) >=
      local_data->destruction_capacity_0_day)
    {
      local_data->no_more_destructions = TRUE;
#if DEBUG
      g_debug ("no more destructions after this day");
#endif
    }

  /* We use the destruction lists in different ways according to the user-
   * specified priority scheme.
   *
   * If time waiting has first priority,
   */
  if (local_data->destruction_time_waiting_priority == 1)
    {
      npriorities = local_data->pending_destructions->len;
      while (local_data->nherds_destroyed_today < destruction_capacity)
        {
          /* Find the herd that has been waiting the longest.  Favour herds
           * higher up in the lists. */
          oldest_request_index = -1;
          for (priority = 0; priority < npriorities; priority++)
            {
              q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, priority);
              if (g_queue_is_empty (q))
                continue;

              pending_destruction = (EVT_event_t *) g_queue_peek_head (q);

              /* When we put the destruction on the waiting list, we stored the
               * day it was requested. */
              request_day = pending_destruction->u.request_for_destruction.day;
              if (oldest_request_index == -1 || request_day < oldest_request_day)
                {
                  oldest_request_index = priority;
                  oldest_request_day = request_day;
                }
            }
          /* If we couldn't find any request that can be carried out today,
           * stop the loop. */
          if (oldest_request_index < 0)
            break;

          q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, oldest_request_index);
          pending_destruction = (EVT_event_t *) g_queue_pop_head (q);
          details = &(pending_destruction->u.request_for_destruction);
          local_data->destruction_status[details->herd->index] = NULL;

          destroy (self, details->herd, day, details->reason, details->day_commitment_made, queue);
          EVT_free_event (pending_destruction);
        }
    }                           /* end case where time waiting has 1st priority. */

  else if (local_data->destruction_time_waiting_priority == 2)
    {
      int start, end, step;

      npriorities = local_data->pending_destructions->len;
      if (local_data->destruction_prod_type_priority == 1)
        step = local_data->ndestruction_reasons;
      else
        step = local_data->nprod_types;
      start = 0;
      end = MIN (start + step, npriorities);

      while (local_data->nherds_destroyed_today < destruction_capacity)
        {
          /* Find the herd that has been waiting the longest.  Favour herds
           * higher up in the lists. */
          oldest_request_index = -1;
          for (priority = start; priority < end; priority++)
            {
              q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, priority);
              if (g_queue_is_empty (q))
                continue;

              pending_destruction = (EVT_event_t *) g_queue_peek_head (q);

              /* When we put the destruction on the waiting list, we stored the
               * day it was requested. */
              request_day = pending_destruction->u.request_for_destruction.day;
              if (oldest_request_index == -1 || request_day < oldest_request_day)
                {
                  oldest_request_index = priority;
                  oldest_request_day = request_day;
                }
            }
          /* If we couldn't find any request that can be carried out today,
           * advance to the next block of lists. */
          if (oldest_request_index < 0)
            {
              start += step;
              if (start >= npriorities)
                break;
              end = MIN (start + step, npriorities);
              continue;
            }

          q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, oldest_request_index);
          pending_destruction = (EVT_event_t *) g_queue_pop_head (q);
          details = &(pending_destruction->u.request_for_destruction);
          local_data->destruction_status[details->herd->index] = NULL;

          destroy (self, details->herd, day, details->reason, details->day_commitment_made, queue);
          EVT_free_event (pending_destruction);
        }
    }                           /* end case where time waiting has 2nd priority. */

  else
    {
      npriorities = local_data->pending_destructions->len;
      for (priority = 0;
           priority < npriorities && local_data->nherds_destroyed_today < destruction_capacity;
           priority++)
        {
          q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, priority);
#if DEBUG
          if (!g_queue_is_empty (q))
            g_debug ("destroying priority %i units", priority + 1);
#endif
          while (!g_queue_is_empty (q) && local_data->nherds_destroyed_today < destruction_capacity)
            {
              pending_destruction = (EVT_event_t *) g_queue_pop_head (q);
              details = &(pending_destruction->u.request_for_destruction);
              local_data->destruction_status[details->herd->index] = NULL;

              destroy (self, details->herd, day, details->reason, details->day_commitment_made, queue);
              EVT_free_event (pending_destruction);
            }
        }
    }                           /* end case where time waiting has 3rd priority. */

#if DEBUG
  g_debug ("----- EXIT destroy_by_priority (%s)", MODEL_NAME);
#endif
}



void
vaccinate (struct naadsm_model_t_ *self, HRD_herd_t * herd,
           int day, char *reason, int day_commitment_made,
           int min_days_before_next, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  unsigned int last_vaccination, days_since_last_vaccination;

#if DEBUG
  g_debug ("----- ENTER vaccinate (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  #ifdef RIVERTON
  /* In Riverton, if the unit is Destroyed or NaturallyImmune, 
   * it should not be vaccinated. */
  g_assert ( (herd->status != Destroyed) && (herd->status != NaturallyImmune) );
  #else
  /* In the standard version, if the unit is already Destroyed, 
   * it should not be vaccinated. */
  g_assert (herd->status != Destroyed);
  #endif

  /* If the unit has already been vaccinated recently, we can ignore the
   * request. */
  if (day < local_data->min_next_vaccination_day[herd->index])
    {
      last_vaccination = local_data->day_last_vaccinated[herd->index];
      days_since_last_vaccination = day - last_vaccination;
#if DEBUG
      g_debug ("unit \"%s\" was vaccinated %u days ago, will not re-vaccinate",
               herd->official_id, days_since_last_vaccination);
#endif
      EVT_event_enqueue (queue, EVT_new_vaccination_canceled_event (herd, day, day_commitment_made));
      goto end;
    }

  /* Vaccinate the unit. */
#if DEBUG
  g_debug ("vaccinating unit \"%s\"", herd->official_id);
#endif
  EVT_event_enqueue (queue, EVT_new_vaccination_event (herd, day, reason, day_commitment_made));
  local_data->nherds_vaccinated_today++;
  local_data->min_next_vaccination_day[herd->index] = day + min_days_before_next;

end:
#if DEBUG
  g_debug ("----- EXIT vaccinate (%s)", MODEL_NAME);
#endif
  return;
}



void
vaccinate_by_priority (struct naadsm_model_t_ *self, int day,
                       EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  unsigned int vaccination_capacity;
  EVT_event_t *pending_vaccination;
  EVT_request_for_vaccination_event_t *details;
  unsigned int npriorities;
  unsigned int priority;
  int request_day, oldest_request_day;
  int oldest_request_index;
  GQueue *q;
  GQueue *locations_in_queue;
  GList *link_to_delete;

#if DEBUG
  g_debug ("----- ENTER vaccinate_by_priority (%s)", MODEL_NAME);
#endif

  /* Eliminate compiler warnings about uninitialized values */
  oldest_request_day = INT_MAX;

  local_data = (local_data_t *) (self->model_data);

  /* Look up the vaccination capacity (which may increase as the outbreak
   * progresses). */
  vaccination_capacity =
    (unsigned int)
    round (REL_chart_lookup
           (day - local_data->first_detection_day - 1, local_data->vaccination_capacity));

  /* Check whether the vaccination capacity has dropped to 0 for good. */
  if (local_data->vaccination_capacity_goes_to_0
      && (day - local_data->first_detection_day) >=
      local_data->vaccination_capacity_0_day)
    {
      local_data->no_more_vaccinations = TRUE;
#if DEBUG
      g_debug ("no more vaccinations after this day");
#endif
    }

  /* We use the vaccination lists in different ways according to the user-
   * specified priority scheme.
   *
   * If time waiting has first priority,
   */
  if (local_data->vaccination_time_waiting_priority == 1)
    {
      npriorities = local_data->pending_vaccinations->len;
      while (local_data->nherds_vaccinated_today < vaccination_capacity)
        {
          /* Find the herd that has been waiting the longest.  Favour herds
           * higher up in the lists. */
          oldest_request_index = -1;
          for (priority = 0; priority < npriorities; priority++)
            {
              q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, priority);
              if (g_queue_is_empty (q))
                continue;

              pending_vaccination = (EVT_event_t *) g_queue_peek_head (q);

              /* When we put the vaccination on the waiting list, we stored the
               * day it was requested. */
              request_day = pending_vaccination->u.request_for_vaccination.day;
              if (oldest_request_index == -1 || request_day < oldest_request_day)
                {
                  oldest_request_index = priority;
                  oldest_request_day = request_day;
                }
            }
          /* If we couldn't find any request that can be carried out today,
           * stop the loop. */
          if (oldest_request_index < 0)
            break;

          q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, oldest_request_index);
          link_to_delete = g_queue_peek_head_link (q);
          pending_vaccination = (EVT_event_t *) g_queue_pop_head (q);
          details = &(pending_vaccination->u.request_for_vaccination);

          vaccinate (self, details->herd, day, details->reason, details->day_commitment_made,
                     details->min_days_before_next, queue);
          locations_in_queue = (GQueue *) g_hash_table_lookup (local_data->vaccination_status, details->herd);
          g_queue_remove (locations_in_queue, link_to_delete);
          if (g_queue_is_empty (locations_in_queue))
            g_hash_table_remove (local_data->vaccination_status, details->herd);
          EVT_free_event (pending_vaccination);
        }
    }                           /* end case where time waiting has 1st priority. */

  else if (local_data->vaccination_time_waiting_priority == 2)
    {
      int start, end, step;

      npriorities = local_data->pending_vaccinations->len;
      if (local_data->vaccination_prod_type_priority == 1)
        step = local_data->nvaccination_reasons;
      else
        step = local_data->nprod_types;
      start = 0;
      end = MIN (start + step, npriorities);

      while (local_data->nherds_vaccinated_today < vaccination_capacity)
        {
          /* Find the herd that has been waiting the longest.  Favour herds
           * higher up in the lists. */
          oldest_request_index = -1;
          for (priority = start; priority < end; priority++)
            {
              q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, priority);
              if (g_queue_is_empty (q))
                continue;

              pending_vaccination = (EVT_event_t *) g_queue_peek_head (q);

              /* When we put the vaccination on the waiting list, we stored the
               * day it was requested. */
              request_day = pending_vaccination->u.request_for_vaccination.day;
              if (oldest_request_index == -1 || request_day < oldest_request_day)
                {
                  oldest_request_index = priority;
                  oldest_request_day = request_day;
                }
            }
          /* If we couldn't find any request that can be carried out today,
           * advance to the next block of lists. */
          if (oldest_request_index < 0)
            {
              start += step;
              if (start >= npriorities)
                break;
              end = MIN (start + step, npriorities);
              continue;
            }

          q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, oldest_request_index);
          link_to_delete = g_queue_peek_head_link (q);
          pending_vaccination = (EVT_event_t *) g_queue_pop_head (q);
          details = &(pending_vaccination->u.request_for_vaccination);

          vaccinate (self, details->herd, day, details->reason, details->day_commitment_made,
                     details->min_days_before_next, queue);
          locations_in_queue = g_hash_table_lookup (local_data->vaccination_status, details->herd);
          g_queue_remove (locations_in_queue, link_to_delete);
          if (g_queue_is_empty (locations_in_queue))
            g_hash_table_remove (local_data->vaccination_status, details->herd);
          EVT_free_event (pending_vaccination);
        }
    }                           /* end case where time waiting has 2nd priority. */

  else
    {
      npriorities = local_data->pending_vaccinations->len;
      for (priority = 0;
           priority < npriorities && local_data->nherds_vaccinated_today < vaccination_capacity;
           priority++)
        {
          q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, priority);
#if DEBUG
          if (!g_queue_is_empty (q))
            g_debug ("vaccinating priority %i units", priority + 1);
#endif
          while (!g_queue_is_empty (q)
                 && local_data->nherds_vaccinated_today < vaccination_capacity)
            {
              link_to_delete = g_queue_peek_head_link (q);
              pending_vaccination = (EVT_event_t *) g_queue_pop_head (q);
              details = &(pending_vaccination->u.request_for_vaccination);

              vaccinate (self, details->herd, day, details->reason, details->day_commitment_made,
                         details->min_days_before_next, queue);
              locations_in_queue = g_hash_table_lookup (local_data->vaccination_status, details->herd);
              g_queue_remove (locations_in_queue, link_to_delete);
              if (g_queue_is_empty (locations_in_queue))
                g_hash_table_remove (local_data->vaccination_status, details->herd);
              EVT_free_event (pending_vaccination);
            }
        }
    }                           /* end case where time waiting has 3rd priority. */

#if DEBUG
  g_debug ("----- EXIT vaccinate_by_priority (%s)", MODEL_NAME);
#endif
}



static void
clear_all_pending_vaccinations (local_data_t *local_data)
{
  guint npriorities, i;
  GQueue *q;

  npriorities = local_data->pending_vaccinations->len;
  g_hash_table_remove_all (local_data->vaccination_status);
  for (i = 0; i < npriorities; i++)
    {
      q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, i);
      while (!g_queue_is_empty (q))
        EVT_free_event (g_queue_pop_head (q));
    }
  return;
}



/**
 * Responds to a new day event by carrying out any queued destructions or
 * vaccinations.
 *
 * @param self the model.
 * @param event a new day event.
 * @param herds a herd list.
 * @param queue for any new events the model creates.
 */
void
handle_new_day_event (struct naadsm_model_t_ *self,
                      EVT_new_day_event_t * event,
                      HRD_herd_list_t * herds,
                      EVT_event_queue_t * queue)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_new_day_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  g_hash_table_remove_all (local_data->detected_today);
  g_hash_table_remove_all (local_data->destroyed_today);

  /* Destroy any waiting herds, as many as possible before destruction capacity
   * runs out. */
  local_data->nherds_destroyed_today = 0;
  if (local_data->outbreak_known && (event->day >= local_data->destruction_program_begin_day))
    destroy_by_priority (self, event->day, queue);

  local_data->nherds_vaccinated_today = 0;
  if (local_data->ndetected_herds < local_data->vaccination_program_threshold)
    {
      /* If we haven't passed the threshold for vaccination yet, remove any
       * requests for vaccination that happened as a result of detections
       * yesterday. */
      #if DEBUG
        g_debug ("# detections so far (%u) < vaccination threshold (%u), deleting yesterday's requests to vaccinate",
                 local_data->ndetected_herds, local_data->vaccination_program_threshold);
      #endif
      clear_all_pending_vaccinations (local_data);
    }
  else
    {
      /* Vaccinate any waiting herds, as many as possible before vaccination
       * capacity runs out. */
      vaccinate_by_priority (self, event->day, queue);
    }

#if DEBUG
  g_debug ("----- EXIT handle_new_day_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a declaration of destruction reasons by recording the potential
 * reasons for destruction.
 *
 * @param self the model.
 * @param event a declaration of destruction reasons event.
 */
void
handle_declaration_of_destruction_reasons_event (struct naadsm_model_t_ *self,
                                                 EVT_declaration_of_destruction_reasons_event_t *
                                                 event)
{
  local_data_t *local_data;
  unsigned int n, m, i, j;
  char *reason;
#if DEBUG
  GString *s;
#endif

#if DEBUG
  g_debug ("----- ENTER handle_declaration_of_destruction_reasons_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* Copy the list of potential reasons for destruction.  (Note that we just
   * copy the pointers to the C strings, assuming that they are static strings.)
   * If any potential reason is not already present in our list, add to our
   * count of distinct reasons. */
  n = event->reasons->len;
  for (i = 0; i < n; i++)
    {
      reason = (char *) g_ptr_array_index (event->reasons, i);

      m = local_data->destruction_reasons->len;
      for (j = 0; j < m; j++)
        {
          if (strcasecmp (reason, g_ptr_array_index (local_data->destruction_reasons, j)) == 0)
            break;
        }
      if (j == m)
        {
          /* We haven't encountered this reason before; add its name to the
           * list. */
          g_ptr_array_add (local_data->destruction_reasons, reason);
          local_data->ndestruction_reasons++;
#if DEBUG
          g_debug ("  adding new reason \"%s\"", reason);
#endif
        }
    }
#if DEBUG
  s = g_string_new ("  list of reasons now={");
  n = local_data->destruction_reasons->len;
  for (i = 0; i < n; i++)
    g_string_append_printf (s, i == 0 ? "\"%s\"" : ",\"%s\"",
                            (char *) g_ptr_array_index (local_data->destruction_reasons, i));
  g_string_append_c (s, '}');
  g_debug ("%s", s->str);
  g_string_free (s, TRUE);
#endif

#if DEBUG
  g_debug ("----- EXIT handle_declaration_of_destruction_reasons_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a declaration of vaccination reasons by recording the potential
 * reasons for vaccination.
 *
 * @param self the model.
 * @param event a declaration of vaccination reasons event.
 */
void
handle_declaration_of_vaccination_reasons_event (struct naadsm_model_t_ *self,
                                                 EVT_declaration_of_vaccination_reasons_event_t *
                                                 event)
{
  local_data_t *local_data;
  unsigned int n, m, i, j;
  char *reason;
#if DEBUG
  GString *s;
#endif

#if DEBUG
  g_debug ("----- ENTER handle_declaration_of_vaccination_reasons_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* Copy the list of potential reasons for vaccination.  (Note that we just
   * copy the pointers to the C strings, assuming that they are static strings.)
   * If any potential reason is not already present in our list, add to our
   * count of distinct reasons. */
  n = event->reasons->len;
  for (i = 0; i < n; i++)
    {
      reason = (char *) g_ptr_array_index (event->reasons, i);

      m = local_data->vaccination_reasons->len;
      for (j = 0; j < m; j++)
        {
          if (strcasecmp (reason, g_ptr_array_index (local_data->vaccination_reasons, j)) == 0)
            break;
        }
      if (j == m)
        {
          /* We haven't encountered this reason before; add its name to the
           * list. */
          g_ptr_array_add (local_data->vaccination_reasons, reason);
          local_data->nvaccination_reasons++;
#if DEBUG
          g_debug ("  adding new reason \"%s\"", reason);
#endif
        }
    }
#if DEBUG
  s = g_string_new ("  list of reasons now={");
  n = local_data->vaccination_reasons->len;
  for (i = 0; i < n; i++)
    g_string_append_printf (s, i == 0 ? "\"%s\"" : ",\"%s\"",
                            (char *) g_ptr_array_index (local_data->vaccination_reasons, i));
  g_string_append_c (s, '}');
  g_debug ("%s", s->str);
  g_string_free (s, TRUE);
#endif

#if DEBUG
  g_debug ("----- EXIT handle_declaration_of_vaccination_reasons_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to the first detection event by announcing an outbreak and
 * initiating a destruction program.
 *
 * @param self the model.
 * @param event a detection event.
 * @param queue for any new events the model creates.
 */
void
handle_detection_event (struct naadsm_model_t_ *self,
                        EVT_detection_event_t * event, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  GQueue *locations_in_queue;
  GList *link;
  EVT_event_t *request;
  EVT_request_for_vaccination_event_t *details;

#if DEBUG
  g_debug ("----- ENTER handle_detection_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  g_hash_table_insert (local_data->detected_herds, event->herd, GINT_TO_POINTER(1));
  local_data->ndetected_herds = g_hash_table_size (local_data->detected_herds);
  if (!local_data->outbreak_known)
    {
      local_data->outbreak_known = TRUE;
      local_data->first_detection_day = event->day;

#if DEBUG
      g_debug ("announcing outbreak");
#endif
      EVT_event_enqueue (queue, EVT_new_public_announcement_event (event->day));

      local_data->destruction_program_begin_day =
        event->day + local_data->destruction_program_delay + 1;
#if DEBUG
      g_debug ("destruction program delayed %hu days (will begin on day %hu)",
               local_data->destruction_program_delay, local_data->destruction_program_begin_day);
#endif
    }

  if (local_data->ndetected_herds == local_data->vaccination_program_threshold)
    {
#if DEBUG
      g_debug ("%u detections, vaccination program begins", local_data->ndetected_herds);
#endif
      ;
    }

  /* If the unit is awaiting vaccination, and the request(s) can be canceled by
   * detection, remove the unit from the waiting list. */
  herd = event->herd;
  locations_in_queue = (GQueue *) g_hash_table_lookup (local_data->vaccination_status, herd);
  if (locations_in_queue != NULL)
    {
      /* Because a unit can be in the vaccination queue more than once, we get
       * back a list of places this unit occurs in the queue.  Check just the
       * first entry to see if the vaccination(s) can be canceled. */
#if DEBUG
      g_debug ("XXX unit \"%s\" in vaccination queue %u times", herd->official_id,
               g_queue_get_length(locations_in_queue));
      g_assert (g_queue_get_length(locations_in_queue) > 0);
#endif
      link = (GList *) g_queue_peek_head (locations_in_queue);
      request = (EVT_event_t *) (link->data);
      details = &(request->u.request_for_vaccination);
      if (details->cancel_on_detection)
        {
          cancel_vaccination (herd, event->day,
                              local_data->vaccination_status,
                              local_data->pending_vaccinations,
                              queue);
        }
    }
#if DEBUG
  else
    {
      g_debug ("XXX unit \"%s\": locations_in_queue was NULL", herd->official_id);
    }
#endif

  /* Store today's detections, because some vaccinations may be canceled by
   * detections. */
  g_hash_table_insert (local_data->detected_today, herd, herd);

#if DEBUG
  g_debug ("----- EXIT handle_detection_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to an unclaimed request for destruction event by committing to do
 * the destruction.
 *
 * @param self the model.
 * @param e a request for destruction event.  The event is copied if needed, so
 *   the original structure may be freed after the call to this function.
 * @param queue for any new events the model creates.
 */
void
handle_request_for_destruction_event (struct naadsm_model_t_ *self,
                                      EVT_event_t * e, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  EVT_request_for_destruction_event_t *event, *old_request;
  HRD_herd_t *herd;
  unsigned int i;
  GQueue *q;
  EVT_event_t *event_copy;
  gboolean replace;

#if DEBUG
  g_debug ("----- ENTER handle_request_for_destruction_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  event = &(e->u.request_for_destruction);

  /* If this herd is being destroyed today, then ignore the request. */
  herd = event->herd;
  if (g_hash_table_lookup (local_data->destroyed_today, herd) != NULL)
    goto end;

  /* There may be more than one request to destroy the same unit.  If this is
   * the first request for this unit, just put it onto the appropriate waiting
   * list. */
  i = herd->index;
  if (local_data->destruction_status[i] == NULL)
    {
#if DEBUG
        g_debug ("no existing request to (potentially) replace");
        g_debug ("authorities commit to destroy unit \"%s\"", herd->official_id);
#endif
      EVT_event_enqueue (queue, EVT_new_commitment_to_destroy_event (herd, event->day));
      /* If the list of pending destruction queues is not long enough (that is,
       * this event has a lower priority than any we've seen before), extend
       * the list of pending destruction queues. */
      while (local_data->pending_destructions->len < event->priority)
        g_ptr_array_add (local_data->pending_destructions, g_queue_new ());

      q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, event->priority - 1);
      event_copy = EVT_clone_event (e);
      event_copy->u.request_for_destruction.day_commitment_made = event->day;
      g_queue_push_tail (q, event_copy);
      /* Store a pointer to the GQueue link that contains this request. */
      local_data->destruction_status[i] = g_queue_peek_tail_link (q);
    }
  else
    {
      /* If this is not the first request to destroy this unit, we must decide
       * decide whether or not to replace the existing request.  We replace the
       * existing request if the new request is higher in the priority
       * system. */

      old_request =
        &(((EVT_event_t *) (local_data->destruction_status[i]->data))->u.request_for_destruction);

      if (local_data->destruction_time_waiting_priority == 1)
        {
          /* Replace the old request if this new request has the same time
           * waiting and a higher priority number.  (The less-than sign in
           * comparing the priority numbers is intentional -- 1 is "higher"
           * than 2.) */

          replace = (event->day == old_request->day) && (event->priority < old_request->priority);
        }
      else if (local_data->destruction_time_waiting_priority == 3)
        {
          /* Replace the old request if this new request has a higher priority
           * number.  (The less-than sign in comparing the priority numbers is
           * intentional -- 1 is "higher" than 2.) */

          replace = (event->priority < old_request->priority);
        }
      else
        {
          /* Replace the old request if this new request is in a higher "block"
           * of priority numbers, or if the new request has the same time
           * waiting and a higher priority number. */

          int step;
          int old_request_block, event_block;

          if (local_data->destruction_prod_type_priority == 1)
            step = local_data->ndestruction_reasons;
          else
            step = local_data->nprod_types;

          /* Integer division... */
          old_request_block = (old_request->priority - 1) / step;
          event_block = (event->priority - 1) / step;

          replace = (event_block < old_request_block)
            || ((event->day == old_request->day) && (event->priority < old_request->priority));
        }
#if DEBUG
      g_debug ("current request %s old one", replace ? "replaces" : "does not replace");
      if (replace)
        {
          char *s;

          s = EVT_event_to_string ((EVT_event_t *) (local_data->destruction_status[i]->data));
          g_debug ("old request = %s", s);
          g_free (s);

          s = EVT_event_to_string (e);
          g_debug ("new request = %s", s);
          g_free (s);
        }
#endif

      if (replace)
        {
          GList *old_link;

          /* Delete both the old RequestForDestruction structure and the GQueue
           * link that holds it. */
          q =
            (GQueue *) g_ptr_array_index (local_data->pending_destructions,
                                          old_request->priority - 1);
          old_link = local_data->destruction_status[i];
          EVT_free_event ((EVT_event_t *) (old_link->data));
          old_request = NULL;
          g_queue_delete_link (q, old_link);

          /* Add the new request to the appropriate GQueue. */
          q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, event->priority - 1);
          event_copy = EVT_clone_event (e);
          event_copy->u.request_for_destruction.day_commitment_made = event->day;
          g_queue_push_tail (q, event_copy);
          local_data->destruction_status[i] = g_queue_peek_tail_link (q);
        }
    }

end:
#if DEBUG
  g_debug ("----- EXIT handle_request_for_destruction_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Responds to a request for vaccination event by committing to do the
 * vaccination.
 *
 * @param self the model.
 * @param e a request for vaccination event.  The event is copied if needed, so
 *   the original structure may be freed after the call to this function.
 * @param queue for any new events the model creates.
 */
void
handle_request_for_vaccination_event (struct naadsm_model_t_ *self,
                                      EVT_event_t * e, EVT_event_queue_t * queue)
{
  local_data_t *local_data;
  EVT_request_for_vaccination_event_t *event;
  HRD_herd_t *herd;
  unsigned int i;
  GQueue *q;
  GQueue *locations_in_queue;
  EVT_event_t *event_copy;
  
#if DEBUG
  g_debug ("----- ENTER handle_request_for_vaccination_event (%s)", MODEL_NAME);
#endif
    
  local_data = (local_data_t *) (self->model_data);
  event = &(e->u.request_for_vaccination);

  /* If this herd has been destroyed today, or this herd has been detected
   * and we do not want to vaccinate detected herds, then ignore the request. */
  herd = event->herd;
  if ((event->cancel_on_detection == TRUE
       && g_hash_table_lookup (local_data->detected_today, herd) != NULL)
      || g_hash_table_lookup (local_data->destroyed_today, herd) != NULL)
    goto end;

  if (TRUE)
    {
      /* There may be more than one request to vaccinate the same unit.  We
       * keep all of them. */
      i = herd->index;
#if DEBUG
      g_debug ("authorities commit to vaccinate unit \"%s\"", herd->official_id);
#endif
      EVT_event_enqueue (queue, EVT_new_commitment_to_vaccinate_event (herd, event->day));
      /* If the list of pending vaccination queues is not long enough (that is,
       * this event has a lower priority than any we've seen before), extend
       * the list of pending vaccination queues. */
      while (local_data->pending_vaccinations->len < event->priority)
        g_ptr_array_add (local_data->pending_vaccinations, g_queue_new ());

      q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, event->priority - 1);
      event_copy = EVT_clone_event (e);
      event_copy->u.request_for_vaccination.day_commitment_made = event->day;
      g_queue_push_tail (q, event_copy);
      /* Store a pointer to the GQueue link that contains this request. */
      locations_in_queue = (GQueue *) g_hash_table_lookup (local_data->vaccination_status, herd);
      if (locations_in_queue == NULL)
        {
          locations_in_queue = g_queue_new ();
          g_hash_table_insert (local_data->vaccination_status, herd, locations_in_queue);
        }
      g_queue_push_tail (locations_in_queue, g_queue_peek_tail_link (q));
    }

end:
#if DEBUG
  g_debug ("----- EXIT handle_request_for_vaccination_event (%s)", MODEL_NAME);
#endif

  return;
}



/**
 * Responds to a vaccination event by noting the day on which the herd was
 * vaccinated.
 *
 * @param self the model.
 * @param event a vaccination event.
 */
void
handle_vaccination_event (struct naadsm_model_t_ *self, EVT_vaccination_event_t * event)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_vaccination_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  local_data->day_last_vaccinated[event->herd->index] = event->day;

#if DEBUG
  g_debug ("----- EXIT handle_vaccination_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a request for zone focus event by adding a new zone focus (to
 * come into the effect on the next simulation day) to the zone list.
 *
 * @param self the model.
 * @param event a request for zone focus event.
 * @param zones the zone list.
 */
void
handle_request_for_zone_focus_event (struct naadsm_model_t_ *self,
                                     EVT_request_for_zone_focus_event_t * event,
                                     ZON_zone_list_t * zones)
{
  local_data_t *local_data;
  HRD_herd_t *herd;

#if DEBUG
  g_debug ("----- ENTER handle_request_for_zone_focus_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;
#if DEBUG
  g_debug ("adding pending zone focus at x=%g, y=%g", herd->x, herd->y);
#endif
  ZON_zone_list_add_focus (zones, herd->x, herd->y);

#ifdef USE_SC_GUILIB
  sc_make_zone_focus( event->day, herd );
#else
  if( NULL != naadsm_make_zone_focus )
    naadsm_make_zone_focus (herd->index);
#endif

#if DEBUG
  g_debug ("----- EXIT handle_request_for_zone_focus_event (%s)", MODEL_NAME);
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
    case EVT_NewDay:
      handle_new_day_event (self, &(event->u.new_day), herds, queue);
      break;
    case EVT_DeclarationOfDestructionReasons:
      handle_declaration_of_destruction_reasons_event (self,
                                                       &(event->u.
                                                         declaration_of_destruction_reasons));
      break;
    case EVT_DeclarationOfVaccinationReasons:
      handle_declaration_of_vaccination_reasons_event (self,
                                                       &(event->u.
                                                         declaration_of_vaccination_reasons));
      break;
    case EVT_Detection:
      handle_detection_event (self, &(event->u.detection), queue);
      break;
    case EVT_RequestForDestruction:
      handle_request_for_destruction_event (self, event, queue);
      break;
    case EVT_RequestForVaccination:
      handle_request_for_vaccination_event (self, event, queue);
      break;
    case EVT_Vaccination:
      handle_vaccination_event (self, &(event->u.vaccination));
      break;
    case EVT_RequestForZoneFocus:
      handle_request_for_zone_focus_event (self, &(event->u.request_for_zone_focus), zones);
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
  unsigned int npriorities;
  GQueue *q;
  int i;

  local_data = (local_data_t *) (self->model_data);
#if DEBUG
  g_debug ("----- ENTER reset (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  local_data->outbreak_known = FALSE;
  g_hash_table_remove_all (local_data->detected_herds);
  local_data->ndetected_herds = 0;
  local_data->destruction_program_begin_day = 0;

  npriorities = local_data->pending_destructions->len;
  for (i = 0; i < local_data->nherds; i++)
    local_data->destruction_status[i] = NULL;
  local_data->nherds_destroyed_today = 0;
  for (i = 0; i < npriorities; i++)
    {
      q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, i);
      while (!g_queue_is_empty (q))
        EVT_free_event (g_queue_pop_head (q));
    }
  local_data->no_more_destructions = FALSE;
  g_hash_table_remove_all (local_data->destroyed_today);

  /* Empty the prioritized pending vaccinations lists. */
  clear_all_pending_vaccinations (local_data);
  local_data->nherds_vaccinated_today = 0;

  for (i = 0; i < local_data->nherds; i++)
    {
      local_data->day_last_vaccinated[i] = 0;
      local_data->min_next_vaccination_day[i] = 0;
    }
  local_data->no_more_vaccinations = FALSE;
  g_hash_table_remove_all (local_data->detected_today);

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
  local_data_t *local_data;
  unsigned int npriorities;
  GQueue *q;
  int i;

  local_data = (local_data_t *) (self->model_data);

  /* We only bother checking for pending vaccinations if the vaccination
   * program threshold has been passed and if there is or will be capacity to
   * vaccinate. */
  if (local_data->ndetected_herds >= local_data->vaccination_program_threshold
      && !local_data->no_more_vaccinations)
    {
      npriorities = local_data->pending_vaccinations->len;
      for (i = 0; i < npriorities; i++)
        {
          q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, i);
          if (!g_queue_is_empty (q))
            {
              return TRUE;
            }
        }
    }

  if (!local_data->no_more_destructions)
    {
      npriorities = local_data->pending_destructions->len;
      for (i = 0; i < npriorities; i++)
        {
          q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, i);
          if (!g_queue_is_empty (q))
            {
              return TRUE;
            }
        }
    }

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
  char *substring, *chararray;
  local_data_t *local_data;

  local_data = (local_data_t *) (self->model_data);
  s = g_string_new (NULL);
  g_string_sprintf (s, "<%s\n", MODEL_NAME);

  g_string_sprintfa (s, "  destruction-program-delay=%i\n", local_data->destruction_program_delay);

  substring = REL_chart_to_string (local_data->destruction_capacity);
  g_string_sprintfa (s, "  destruction-capacity=%s\n", substring);
  g_free (substring);

  g_string_sprintfa (s, "  vaccination-program-threshold=%u\n",
                     local_data->vaccination_program_threshold);

  substring = REL_chart_to_string (local_data->vaccination_capacity);
  g_string_sprintfa (s, "  vaccination-capacity=%s>", substring);
  g_free (substring);

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
 * Frees this model.
 *
 * @param self the model.
 */
void
local_free (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;
  unsigned int npriorities;
  GQueue *q;
  int i;

#if DEBUG
  g_debug ("----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);
  REL_free_chart (local_data->destruction_capacity);
  g_free (local_data->destruction_status);
  npriorities = local_data->pending_destructions->len;
  for (i = 0; i < npriorities; i++)
    {
      q = (GQueue *) g_ptr_array_index (local_data->pending_destructions, i);
      while (!g_queue_is_empty (q))
        EVT_free_event (g_queue_pop_head (q));
      g_queue_free (q);
    }
  g_ptr_array_free (local_data->pending_destructions, TRUE);
  g_hash_table_destroy (local_data->destroyed_today);

  /* We destroy the array of pointers but not the C strings they were pointing
   * to; those we assume are static strings. */
  g_ptr_array_free (local_data->destruction_reasons, TRUE);
  g_ptr_array_free (local_data->vaccination_reasons, TRUE);

  REL_free_chart (local_data->vaccination_capacity);
  clear_all_pending_vaccinations (local_data);
  g_hash_table_destroy (local_data->vaccination_status);
  npriorities = local_data->pending_vaccinations->len;
  for (i = 0; i < npriorities; i++)
    {
      q = (GQueue *) g_ptr_array_index (local_data->pending_vaccinations, i);
      g_queue_free (q);
    }
  g_ptr_array_free (local_data->pending_vaccinations, TRUE);
  g_hash_table_destroy (local_data->detected_herds);
  g_hash_table_destroy (local_data->detected_today);
  g_free (local_data->day_last_vaccinated);
  g_free (local_data->min_next_vaccination_day);
  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new authorities model.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *m;
  local_data_t *local_data;
  scew_element *e;
  gboolean success;
  char *tmp;
  double dummy;

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

  e = scew_element_by_name (params, "destruction-program-delay");
  if (e != NULL)
    {
      local_data->destruction_program_delay = (int) (PAR_get_time (e, &success));
      if (success == FALSE)
        {
          g_warning ("%s: setting destruction program delay to 0 days", MODEL_NAME);
          local_data->destruction_program_delay = 0;
        }
    }
  else
    {
      g_warning ("%s: destruction program delay missing, setting to 0 days", MODEL_NAME);
      local_data->destruction_program_delay = 0;
    }

  e = scew_element_by_name (params, "destruction-capacity");
  if (e != NULL)
    {
      local_data->destruction_capacity = PAR_get_relationship_chart (e);
    }
  else
    {
      g_warning ("%s: destruction capacity missing, setting to 0", MODEL_NAME);
      local_data->destruction_capacity = REL_new_point_chart (0);
    }
  /* Set a flag if the destruction capacity chart at some point drops to 0 and
   * stays there. */
  local_data->destruction_capacity_goes_to_0 =
    REL_chart_zero_at_right (local_data->destruction_capacity, &dummy);
  if (local_data->destruction_capacity_goes_to_0)
    {
      local_data->destruction_capacity_0_day = (int) ceil (dummy) - 1;
#if DEBUG
      g_debug ("destruction capacity drops to 0 on and after day %i",
               local_data->destruction_capacity_0_day);
#endif
    }

  e = scew_element_by_name (params, "destruction-priority-order");
  if (e != NULL)
    {
      tmp = PAR_get_text (e);
      if (strcasecmp (tmp, "production type,reason,time waiting") == 0)
        {
          local_data->destruction_prod_type_priority = 1;
          local_data->destruction_reason_priority = 2;
          local_data->destruction_time_waiting_priority = 3;
        }
      else if (strcasecmp (tmp, "production type,time waiting,reason") == 0)
        {
          local_data->destruction_prod_type_priority = 1;
          local_data->destruction_reason_priority = 3;
          local_data->destruction_time_waiting_priority = 2;
        }
      else if (strcasecmp (tmp, "reason,production type,time waiting") == 0)
        {
          local_data->destruction_prod_type_priority = 2;
          local_data->destruction_reason_priority = 1;
          local_data->destruction_time_waiting_priority = 3;
        }
      else if (strcasecmp (tmp, "reason,time waiting,production type") == 0)
        {
          local_data->destruction_prod_type_priority = 3;
          local_data->destruction_reason_priority = 1;
          local_data->destruction_time_waiting_priority = 2;
        }
      else if (strcasecmp (tmp, "time waiting,reason,production type") == 0)
        {
          local_data->destruction_prod_type_priority = 3;
          local_data->destruction_reason_priority = 2;
          local_data->destruction_time_waiting_priority = 1;
        }
      else if (strcasecmp (tmp, "time waiting,production type,reason") == 0)
        {
          local_data->destruction_prod_type_priority = 2;
          local_data->destruction_reason_priority = 3;
          local_data->destruction_time_waiting_priority = 1;
        }
      else
        {
          g_warning
            ("%s: assuming destruction priority order reason > production type > time waiting",
             MODEL_NAME);
          local_data->destruction_reason_priority = 1;
          local_data->destruction_prod_type_priority = 2;
          local_data->destruction_time_waiting_priority = 3;
        }
      g_free (tmp);
    }
  else
    {
      g_warning ("%s: assuming destruction priority order reason > production type > time waiting",
                 MODEL_NAME);
      local_data->destruction_reason_priority = 1;
      local_data->destruction_prod_type_priority = 2;
      local_data->destruction_time_waiting_priority = 3;
    }

  e = scew_element_by_name (params, "vaccination-program-delay");
  if (e != NULL)
    {
      local_data->vaccination_program_threshold =
        (unsigned int) round (PAR_get_unitless (e, &success));
      if (success == FALSE)
        {
          g_warning ("%s: will begin vaccination after first detection", MODEL_NAME);
          local_data->vaccination_program_threshold = 1;
        }
    }
  else
    {
      g_warning
        ("%s: begin after detections parameter missing, will begin vaccination after first detection",
         MODEL_NAME);
      local_data->vaccination_program_threshold = 1;
    }

  e = scew_element_by_name (params, "vaccination-capacity");
  if (e != NULL)
    {
      local_data->vaccination_capacity = PAR_get_relationship_chart (e);
    }
  else
    {
      g_warning ("%s: vaccination capacity missing, setting to 0", MODEL_NAME);
      local_data->vaccination_capacity = REL_new_point_chart (0);
    }
  /* Set a flag if the vaccination capacity chart at some point drops to 0 and
   * stays there. */
  local_data->vaccination_capacity_goes_to_0 =
    REL_chart_zero_at_right (local_data->vaccination_capacity, &dummy);
  if (local_data->vaccination_capacity_goes_to_0)
    {
      local_data->vaccination_capacity_0_day = (int) ceil (dummy);
#if DEBUG
      g_debug ("vaccination capacity drops to 0 on and after the %ith day since 1st detection",
               local_data->vaccination_capacity_0_day + 1);
#endif
    }

  e = scew_element_by_name (params, "vaccination-priority-order");
  if (e != NULL)
    {
      tmp = PAR_get_text (e);
      if (strcasecmp (tmp, "production type,reason,time waiting") == 0)
        {
          local_data->vaccination_prod_type_priority = 1;
          local_data->vaccination_reason_priority = 2;
          local_data->vaccination_time_waiting_priority = 3;
        }
      else if (strcasecmp (tmp, "production type,time waiting,reason") == 0)
        {
          local_data->vaccination_prod_type_priority = 1;
          local_data->vaccination_reason_priority = 3;
          local_data->vaccination_time_waiting_priority = 2;
        }
      else if (strcasecmp (tmp, "reason,production type,time waiting") == 0)
        {
          local_data->vaccination_prod_type_priority = 2;
          local_data->vaccination_reason_priority = 1;
          local_data->vaccination_time_waiting_priority = 3;
        }
      else if (strcasecmp (tmp, "reason,time waiting,production type") == 0)
        {
          local_data->vaccination_prod_type_priority = 3;
          local_data->vaccination_reason_priority = 1;
          local_data->vaccination_time_waiting_priority = 2;
        }
      else if (strcasecmp (tmp, "time waiting,reason,production type") == 0)
        {
          local_data->vaccination_prod_type_priority = 3;
          local_data->vaccination_reason_priority = 2;
          local_data->vaccination_time_waiting_priority = 1;
        }
      else if (strcasecmp (tmp, "time waiting,production type,reason") == 0)
        {
          local_data->vaccination_prod_type_priority = 2;
          local_data->vaccination_reason_priority = 3;
          local_data->vaccination_time_waiting_priority = 1;
        }
      else
        {
          g_warning
            ("%s: assuming vaccination priority order reason > production type > time waiting",
             MODEL_NAME);
          local_data->vaccination_reason_priority = 1;
          local_data->vaccination_prod_type_priority = 2;
          local_data->vaccination_time_waiting_priority = 3;
        }
      g_free (tmp);
    }
  else
    {
      g_warning ("%s: assuming vaccination priority order reason > production type > time waiting",
                 MODEL_NAME);
      local_data->vaccination_reason_priority = 1;
      local_data->vaccination_prod_type_priority = 2;
      local_data->vaccination_time_waiting_priority = 3;
    }

  local_data->nherds = HRD_herd_list_length (herds);
  local_data->nprod_types = herds->production_type_names->len;

  /* No outbreak has been detected yet. */
  local_data->outbreak_known = FALSE;
  local_data->destruction_program_begin_day = 0;
  local_data->ndetected_herds = 0;

  /* No herds have been destroyed or slated for destruction yet. */
  local_data->destruction_status = g_new0 (GList *, local_data->nherds);
  local_data->nherds_destroyed_today = 0;
  local_data->pending_destructions = g_ptr_array_new ();
  local_data->no_more_destructions = FALSE;
  local_data->destroyed_today = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* No herds have been vaccinated or slated for vaccination yet. */
  local_data->vaccination_status = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, g_queue_free_as_GDestroyNotify);
  local_data->nherds_vaccinated_today = 0;
  local_data->pending_vaccinations = g_ptr_array_new ();
  local_data->day_last_vaccinated = g_new0 (int, local_data->nherds);
  local_data->min_next_vaccination_day = g_new0 (int, local_data->nherds);
  local_data->no_more_vaccinations = FALSE;
  local_data->detected_herds = g_hash_table_new (g_direct_hash, g_direct_equal);
  local_data->detected_today = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* We don't yet know how many distinct reasons for destruction or vaccination
   * requests there may be.  We will rely on other sub-models to tell us. */
  local_data->ndestruction_reasons = 0;
  local_data->destruction_reasons = g_ptr_array_new ();
  local_data->nvaccination_reasons = 0;
  local_data->vaccination_reasons = g_ptr_array_new ();

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return m;
}

/* end of file resources-and-implementation-of-controls-model.c */
