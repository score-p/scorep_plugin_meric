/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "meric_plugin.h"

#include <scorep/plugin/plugin.hpp>

#include <vector>
#include <chrono>


using scorep::plugin::logging;


meric_plugin::meric_plugin() :
    measurement( std::chrono::microseconds( stoi( scorep::environment_variable::get( "INTERVAL_US", "50000" ) ) ) )
{
    scorep::plugin::log::set_min_severity_level( nitro::log::severity_level::debug );
}

std::vector<scorep::plugin::metric_property>
meric_plugin::get_metric_properties( const std::string& metric_name )
{
    logging::info() << "get metric properties called with: " << metric_name;

    make_handle( metric_name, metric_name );

    // Must use the same name here as for the handle you made earlier.
    return { scorep::plugin::metric_property(
                 metric_name,
                 "Meric energy counter",
                 "J"
                 ).absolute_point().value_double().decimal() };
}

void
meric_plugin::add_metric( energy_metric& metric )
{
    logging::info() << "add metric called with: " << metric.name;
}

void
meric_plugin::start()
{
    measurement.start( get_handles() );
}

void
meric_plugin::stop()
{
    measurement.stop();
}

template <typename C>
void
meric_plugin::get_all_values( energy_metric& metric, C& cursor )
{
    logging::info() << "get_all_values called with: " << metric.name;

    // write the collected data to the cursor.
    for ( auto& tvpair : measurement.readings( metric ) )
    {
        cursor.write( tvpair.first, tvpair.second );
    }
}


SCOREP_METRIC_PLUGIN_CLASS( meric_plugin, "meric" )
