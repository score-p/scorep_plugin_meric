/*
 * Copyright (c) 2015-2016, Technische Universität Dresden, Germany
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
    This file contains an example plugin written with the Score-P metric plugin C++ wrapper.

    This plugin is an asynchronous metric plugin. It will record values once on a certain thread
    within a certain process.
 */
#include "meric_plugin.h"

#include <scorep/plugin/plugin.hpp>

#include <vector>
#include <chrono>


using scorep::plugin::logging;


meric_plugin::meric_plugin() :
    measurement( std::chrono::microseconds( stoi( scorep::environment_variable::get( "interval_us", "50000" ) ) ) )
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
