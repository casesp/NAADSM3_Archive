/** @file model.h
 * Common interface for sub-models.
 *
 * The simulator design treats sub-models as interchangeable, mix-and-match
 * parts &mdash; "building blocks".  A "simulation" is then the sum of the
 * actions of the active sub-models.
 *
 * @image html building_blocks.png
 *
 * <small>(Image copyright information below)</small>
 *
 * Each module has an "is_singleton" function that returns TRUE if just one
 * instance of the module can exist in memory, or FALSE if multiple instances
 * of the module can exist in memory.  It also has a "new" function that
 * creates and initializes an instance of the module.  The new function must
 * fill in a set of function pointers so that the module's functions can be
 * called in an object-oriented style, e.g., <code>model->printf(model)</code>.
 * See the naadsm_model_t_ structure for a list of these functions.
 *
 * <b>Singleton modules</b>
 *
 * Originally, the idea was that elements in the XML parameter file would map
 * neatly onto module instances in memory.  For example, each disease-model
 * element in the parameter file &mdash; one per production type &mdash; would
 * cause one instance of the module defined in disease-model.c to be created.
 *
 * "Singleton" modules were added to address some problems with that initial
 * idea.
 *
 * First, there are cases where information should be shared among all
 * instances of a module.  For example, airborne spread calculations use a
 * histogram of herd sizes.  It doesn't make sense to keep multiple copies of
 * the histogram in memory, one stored with the parameters for spread from
 * production type A to production type B, one stored with the parameters for
 * spread from production type B to production type C, and so on.  This problem
 * is solved by making airborne spread a "singleton" module where a single
 * object in memory stores both the herd size histogram (global data)
 * <i>and</i> blocks of parameters for each production type combination.
 *
 * Second, the introduction of zones has complicated the parameterization of
 * some modules.  Before zones, the parameters for direct and indirect contact
 * spread depended on source and recipient production type.  Now, there are
 * some parameters that depend on source and recipient production type, and
 * some parameters that depend on source production type and zone.  By making
 * contact spread a singleton module, it is possible to read in different forms
 * of contact-spread-model elements and store the parameters correctly.  See
 * the test cases
 * <a href="test-xml-zones-movement_control_1.html">zones/movement_control_1</a>
 * and
 * <a href="test-xml-zones-detect_1_zones.html">zones/detect_1_zones.xml</a>
 * for examples of how this looks.
 *
 * <small>Image copyright information:
 * <ul>
 *  <li>disease image (virus) is "Bluetongue virus" by Flickr user AJC1.
 *    Released under a Creative Commons Attribution-Noncommercial 2.0 Generic
 *    license.</li>
 *  <li>airborne spread image (sky) is "Landscape Under a Stormy Sky" by Vincent
 *    Van Gogh.  Public domain image (copyright expired).</li>
 *  <li>direct contact image (cow) is "When bovines attack" by Flickr user tricky.
 *    Released under a Creative Commons Attribution-Noncommercial-Share Alike
 *    2.0 Generic license.</li>
 *  <li>indirect contact image (truck) is "truck" by Flickr user rickabbo.
 *    Released under a Creative Commons Attribution 2.0 Generic license.</li>
 *  <li>detection image (Sherlock Holmes items) is by Wikimedia Commons user
 *    Alterego.  Released under the GNU Free Documentation License.</li>
 *  <li>vaccination image (vials) is "Let me stabilize you" by Flickr user
 *    Pulpolux.  Released under a Creative Commons Attribution-Noncommercial
 *    2.0 Generic license.</li>
 *  <li>destruction image (guillotine) is "Death Hangs in the Air" by Flickr user
 *    buck82.  Released under a Creative Commons Attribution-Noncommercial 2.0
 *    Generic license.</li>
 * </ul>
 * </small>
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Grid Computing Research Group<br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.93
 * @date March 2003
 *
 * Copyright &copy; University of Guelph, 2003-2009
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef MODEL_H
#define MODEL_H

#include "herd.h"
#include "event.h"
#include "parameter.h"
#include "reporting.h"
#include "zone.h"
#include "rng.h"



struct naadsm_model_t_;



/** 
 * Type of a function that returns whether a model is a singleton (only one
 * instance should ever exist in memory) or not.
 */
typedef gboolean (*naadsm_model_is_singleton_t) (void);



/** Type of a function that creates and sets parameters for a model. */
typedef struct naadsm_model_t_ *(*naadsm_model_new_t) (PAR_parameter_t *,
                                                       HRD_herd_list_t *,
                                                       projPJ,
                                                       ZON_zone_list_t *);



/** Type of a function that sets parameters for a model. */
typedef void (*naadsm_model_set_params_t) (struct naadsm_model_t_ *, PAR_parameter_t *);



/** Type of a function that frees a model. */
typedef void (*naadsm_model_free_t) (struct naadsm_model_t_ *);



/** Type of a function that runs a model. */
typedef void (*naadsm_model_run_t) (struct naadsm_model_t_ *,
                                    HRD_herd_list_t *, ZON_zone_list_t *,
                                    EVT_event_t *, RAN_gen_t *, EVT_event_queue_t *);



/** Type of a function that resets a model after one simulation run. */
typedef void (*naadsm_model_reset_t) (struct naadsm_model_t_ *);



/**
 * Type of a function that reports whether a model is listening for a given
 * event type.
 */
typedef gboolean (*naadsm_model_is_listening_for_t) (struct naadsm_model_t_ *, EVT_event_type_t);



/**
 * Type of a function that reports whether a model has any pending actions to
 * carry out.
 */
typedef gboolean (*naadsm_model_has_pending_actions_t) (struct naadsm_model_t_ *);



/**
 * Type of a function that reports whether a model has any pending infections
 * to cause.
 */
typedef gboolean (*naadsm_model_has_pending_infections_t) (struct naadsm_model_t_ *);



/** Type of a function that returns a string representation of a model. */
typedef char *(*naadsm_model_to_string_t) (struct naadsm_model_t_ *);



/** Type of a function that prints a model. */
typedef int (*naadsm_model_printf_t) (struct naadsm_model_t_ *);



/** Type of a function that prints a model to a stream. */
typedef int (*naadsm_model_fprintf_t) (FILE *, struct naadsm_model_t_ *);



/** A sub-model. */
typedef struct naadsm_model_t_
{
  char *name; /**< A short name for the model. */
  EVT_event_type_t *events_listened_for; /**< A list of events the model listens for. */
  unsigned int nevents_listened_for; /**< Length of events_listened_for. */
  GPtrArray *outputs; /**< A list of the model's output variables. */
  void *model_data; /**< Specialized information for the particular model. */
  naadsm_model_set_params_t set_params; /**< A function that sets parameters
    for the model. */
  naadsm_model_run_t run; /**< A function that runs the model. */
  naadsm_model_reset_t reset; /**< A function that resets the model after one simulation run. */
  naadsm_model_is_listening_for_t is_listening_for; /**< A function that reports whether the model is listening for a given event type.*/
  naadsm_model_has_pending_actions_t has_pending_actions; /**< A function that reports whether the model has any pending actions to carry out.*/
  naadsm_model_has_pending_infections_t has_pending_infections; /**< A function
    that reports whether the model has any pending infections to cause. */
  naadsm_model_to_string_t to_string; /**< A function that returns a string representation of the model. */
  naadsm_model_printf_t printf; /**< A function that prints the model. */
  naadsm_model_fprintf_t fprintf; /**< A function that prints the model to a stream. */
  naadsm_model_free_t free; /**< A function that frees the model. */
}
naadsm_model_t;

#endif /* !MODEL_H */
