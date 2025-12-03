/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include "meric_measurement.h"
#include <meric_ext.h>

#include <scorep/plugin/plugin.hpp>

#include <vector>
#include <iostream>
#include <memory>


struct domain_info
{
    unsigned int                                  id;  // The value in the Domains enum
    unsigned int                                  idx; // The index in an ExtlibEnergyTimeStamp.domain_data

    std::unordered_map<std::string, unsigned int> counter_id_by_name;
};


std::ostream&
operator<<( std::ostream&      os,
            const domain_info& domain );



template <typename P, typename Policies>
using meric_object_id = scorep::plugin::policy::object_id<energy_metric, P, Policies>;


class meric_plugin : public scorep::plugin::base<meric_plugin,
                                                 scorep::plugin::policy::async,
                                                 scorep::plugin::policy::per_host,
                                                 scorep::plugin::policy::post_mortem,
                                                 scorep::plugin::policy::scorep_clock,
                                                 meric_object_id>
{
public:
    meric_plugin();

    std::vector<scorep::plugin::metric_property>
    get_metric_properties( const std::string& metric_name );

    void
    add_metric( energy_metric& metric );

    void
    start();

    void
    stop();

    template <typename C>
    void
    get_all_values( energy_metric& metric,
                    C&             cursor );

    static std::unordered_map<std::string, domain_info>
    available_domains_and_counters();

private:

    meric_measurement measurement;
    ExtlibEnergyPtr   energy_domains;

    std::unordered_map<std::string, domain_info> domain_by_name;

private:
    static std::vector<unsigned int>
    requested_domain_names( std::string env_str );

    static ExtlibEnergyPtr
    init_meric_extlib( const std::vector<unsigned int>& requested_domains );

    static std::unordered_map<std::string, domain_info>
    query_available_counters( ExtlibEnergy* energy_domains );
};
