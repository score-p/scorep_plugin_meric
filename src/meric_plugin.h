/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include "MeasurementThread.h"
#include "ExtlibWrapper.h"

#include <scorep/plugin/plugin.hpp>

#include <vector>

namespace MericPlugin
{
template <typename P, typename Policies>
using meric_object_id = scorep::plugin::policy::object_id<Metric, P, Policies>;


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
    add_metric( Metric& metric );

    void
    start();

    void
    stop();

    template <typename C>
    void
    get_all_values( Metric& metric,
                    C&      cursor );

private:

    MeasurementThread measurement;
    ExtlibWrapper     extlib;

    std::unordered_map<std::string, ExtlibWrapper::Domain> domain_by_name;

private:
    static std::vector<unsigned int>
    requested_domain_ids( std::string env_str );
};
}
