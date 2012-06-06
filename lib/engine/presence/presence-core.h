
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         presence-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *                          (c) 2008 by Damien Sandras
 *   description          : declaration of the main
 *                          presentity managing object
 *
 */

#ifndef __PRESENCE_CORE_H__
#define __PRESENCE_CORE_H__

#include "services.h"
#include "cluster.h"
#include "account-core.h"

namespace Ekiga
{
  class PersonalDetails;

/**
 * @defgroup presence Presence
 * @{
 */


  class PresentityDecorator: public virtual GmRefCounted
  {
  public:

    /** The destructor.
     */
    virtual ~PresentityDecorator () {}

    /** Completes the menu for actions available on an uri
     * @param The presentity for which the action could be made available.
     * @param The uri for which actions could be made available.
     * @param A MenuBuilder object to populate.
     */
    virtual bool populate_menu (PresentityPtr /*presentity*/,
				const std::string /*uri*/,
				MenuBuilder &/*builder*/) = 0;
  };

  class PresenceFetcher: public virtual GmRefCounted
  {
  public:

    /** The destructor.
     */
    virtual ~PresenceFetcher () {}

    /** Triggers presence fetching for the given uri
     * (notice: the PresenceFetcher should count how many times it was
     * requested presence for an uri, in case several presentities share it)
     * @param The uri for which to fetch presence information.
     */
    virtual void fetch (const std::string /*uri*/) = 0;

    /** Stops presence fetching for the given uri
     * (notice that if some other presentity asked for presence information
     * on the same uri, the fetching should go on until the last of them is
     * gone)
     * @param The uri for which to stop fetching presence information.
     */
    virtual void unfetch (const std::string /*uri*/) = 0;

    /** Those signals are emitted whenever this presence fetcher gets
     * presence information about an uri it was required to handle.
     * The information is given as a pair of strings (uri, data).
     */
    sigc::signal2<void, std::string, std::string> presence_received;
    sigc::signal2<void, std::string, std::string> status_received;
  };

  class PresencePublisher: public virtual GmRefCounted
  {
  public:

    virtual ~PresencePublisher () {}

    virtual void publish (const PersonalDetails& details) = 0;
  };

  /** Core object for the presence support.
   *
   * The presence core has several goals:
   *  - one of them is of course to list presentities, and know what happens to
   *    them;
   *  - another one is that we may want to store presentities somewhere as dead
   *    data, but still be able to gain presence information and actions on
   *    them.
   *
   * This is achieved by using three types of helpers:
   *  - the abstract class PresentityDecorator, which allows to enable actions
   *    on presentities based on uris;
   * - the abstract class PresenceFetcher, through which it is possible to gain
   *   presence information: they allow the PresenceCore to declare some
   *   presence information is needed about an uri, or now unneeded;
   * - finally, a simple callback-based api allows to add detecters for
   *   supported uris: this allows for example a Presentity to know if it
   *   should declare an uri as "foo@bar" or as "prtcl:foo@bar".
   */

  /*
   * FIXME : couldn't a chain of responsibility be used there instead of a
   *         special registering magic?
   */
  class PresenceCore:
    public Service
  {
  public:

    /** The constructor.
     */
    PresenceCore (ServiceCore& core);

    /** The destructor.
     */
    ~PresenceCore ();

    /*** Service Implementation ***/
  public:
    /** Returns the name of the service.
     * @return The service name.
     */
    const std::string get_name () const
    { return "presence-core"; }

    /** Returns the description of the service.
     * @return The service description.
     */
    const std::string get_description () const
    { return "\tPresence managing object"; }

    /*** API to list presentities ***/
  public:

    /** Adds a cluster to the PresenceCore service.
     * @param The cluster to be added.
     */
    void add_cluster (ClusterPtr cluster);

    /** Triggers a callback for all Ekiga::Cluster clusters of the
     * PresenceCore service.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    void visit_clusters (sigc::slot1<bool, ClusterPtr > visitor);

    /** This signal is emitted when an Ekiga::Cluster has been added
     * to the PresenceCore Service.
     */
    sigc::signal1<void, ClusterPtr > cluster_added;

    /** Those signals are forwarding the heap_added, heap_updated
     * and heap_removed from the given Cluster.
     *
     */
    sigc::signal2<void, ClusterPtr , HeapPtr > heap_added;
    sigc::signal2<void, ClusterPtr , HeapPtr > heap_updated;
    sigc::signal2<void, ClusterPtr , HeapPtr > heap_removed;

    /** Those signals are forwarding the presentity_added, presentity_updated
     * and presentity_removed from the given Heap of the given Cluster.
     */
    sigc::signal3<void, ClusterPtr , HeapPtr , PresentityPtr > presentity_added;
    sigc::signal3<void, ClusterPtr , HeapPtr , PresentityPtr > presentity_updated;
    sigc::signal3<void, ClusterPtr , HeapPtr , PresentityPtr > presentity_removed;

  private:

    std::set<ClusterPtr > clusters;
    void on_heap_added (HeapPtr heap, ClusterPtr cluster);
    void on_heap_updated (HeapPtr heap, ClusterPtr cluster);
    void on_heap_removed (HeapPtr heap, ClusterPtr cluster);
    void on_presentity_added (HeapPtr heap,
			      PresentityPtr presentity,
			      ClusterPtr cluster);
    void on_presentity_updated (HeapPtr heap,
				PresentityPtr presentity,
				ClusterPtr cluster);
    void on_presentity_removed (HeapPtr heap,
				PresentityPtr presentity,
				ClusterPtr cluster);

    /*** API to act on presentities ***/
  public:

    /** Adds a decorator to the pool of presentity decorators.
     * @param The presentity decorator.
     */
    void add_presentity_decorator (gmref_ptr<PresentityDecorator> decorator);

    /** Populates a menu with the actions available on a given uri.
     * @param The uri for which the decoration is needed.
     * @param The builder to populate.
     */
    bool populate_presentity_menu (PresentityPtr presentity,
				   const std::string uri,
				   MenuBuilder &builder);

  private:

    std::list<gmref_ptr<PresentityDecorator> > presentity_decorators;

    /*** API to help presentities get presence ***/
  public:

    /** Adds a fetcher to the pool of presentce fetchers.
     * @param The presence fetcher.
     */
    void add_presence_fetcher (gmref_ptr<PresenceFetcher> fetcher);

    /** Tells the PresenceCore that someone is interested in presence
     * information for the given uri.
     * @param: The uri for which presence is requested.
     */
    void fetch_presence (const std::string uri);

    /** Tells the PresenceCore that someone becomes uninterested in presence
     * information for the given uri.
     * @param: The uri for which presence isn't requested anymore.
     */
    void unfetch_presence (const std::string uri);

    /** Those signals are emitted whenever information has been received
     * about an uri ; the information is a pair of strings (uri, information).
     */
    sigc::signal2<void, std::string, std::string> presence_received;
    sigc::signal2<void, std::string, std::string> status_received;

  private:

    std::list<gmref_ptr<PresenceFetcher> > presence_fetchers;
    void on_presence_received (const std::string uri,
			       const std::string presence);
    void on_status_received (const std::string uri,
			     const std::string status);
    struct uri_info
    {
      uri_info (): count(0), presence("unknown"), status("")
      { }

      int count;
      std::string presence;
      std::string status;
    };

    std::map<std::string, uri_info> uri_infos;

    /* help publishing presence */
  public:

    void add_presence_publisher (gmref_ptr<PresencePublisher> publisher);

  private:

    std::list<gmref_ptr<PresencePublisher> > presence_publishers;
    void publish (gmref_ptr<PersonalDetails> details);
    void on_personal_details_updated (PersonalDetails &details);
    void on_registration_event (Ekiga::BankPtr bank,
				Ekiga::AccountPtr account,
				Ekiga::Account::RegistrationState state,
                                std::string info,
                                gmref_ptr<Ekiga::PersonalDetails> details);

    /*** API to control which uri are supported by runtime ***/
  public:

    /** Decides whether an uri is supported by the PresenceCore
     * @param The uri to test for support
     * @return True if the uri is supported
     */
    bool is_supported_uri (const std::string uri) const;

    /** Adds an uri tester to the PresenceCore
     * @param The tester
     */
    void add_supported_uri (sigc::slot1<bool,std::string> tester);

  private:

    std::set<sigc::slot1<bool, std::string> > uri_testers;

    /*** Misc ***/
  public:

    /** Create the menu of the actions available in the PresenceCore.
     * @param A MenuBuilder object to populate.
     */
    bool populate_menu (MenuBuilder &builder);

    /** This chain allows the PresenceCore to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;

    std::list<sigc::connection> conns;
  };

/**
 * @}
 */

};

#endif
