/********************************************************************\
 * policy.h -- Implement Accounting Policy                          *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
\********************************************************************/
/** @addtogroup Engine
    @{ */
/** @addtogroup Policy Accounting Policy (FIFO/LIFO)
 *  This file implements Accounting Policy.  The Accounting Policy
 *  determines how Splits are assigned to Lots.  The contents
 *  of a Lot determines the Gains on that Lot.  The default policy
 *  is the FIFO policy: the first thing bought is also the first
 *  thing sold.
 @{ */

/** @file policy.h
 *  @brief Implement Accounting Policy.
 *  @author Created by Linas Vepstas August 2003
 *  @author Copyright (c) 2003,2004 Linas Vepstas <linas@linas.org>
 */

#ifndef XACC_POLICY_H
#define XACC_POLICY_H

typedef struct gncpolicy_s GNCPolicy;

/** Valid Policy List
 *  Provides a glist of glists for implemented policies. For each implemented
 *  policy, this glist contains: name, description, hint, as follows:
 *    glist(
 *       glist("fifo", "First In First Out", "Use oldest lots first.")
 *       glist("lifo", "Last In First Out", "Use newest lots first.")
 *       etc.
 *         )
 *  Both levels of lists must be freed with g_list_free().
 */
GList * gnc_get_valid_policy_list (void);

/** Valid Policy
 *  Uses the Valid Policy List to determine if a policy name is valid.
 */
gboolean gnc_valid_policy (const gchar *name);

/** First-in, First-out Policy
 *  This policy will create FIFO Lots.  FIFO Lots have the following
 *  properties:
 *  -- The lot is started with the earliest posted split that isn't
 *     a part of another lot already.
 *  -- Splits are added to the lot in date order, with earliest splits
 *     added first.
 *  -- All splits in the lot share the same transaction currency as
 *     the split that opened the lot.
 */
GNCPolicy *xaccGetFIFOPolicy (void);

#endif /* XACC_POLICY_H */
/** @} */
/** @} */
