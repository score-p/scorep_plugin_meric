/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "meric_plugin.h"
#include "utils.h"

#include <scorep/plugin/plugin.hpp>

#include <meric_ext.h>

#include <vector>
#include <chrono>


using scorep::plugin::logging;


using Domains = ExtlibEnergy::Domains;

static const std::unordered_map<std::string, unsigned int> domain_id_by_name = {
    { "A64FX", Domains::EXTLIB_ENERGY_DOMAIN_A64FX },
    { "RAPL",  Domains::EXTLIB_ENERGY_DOMAIN_RAPL  },
    { "NVML",  Domains::EXTLIB_ENERGY_DOMAIN_NVML  },
    { "ROCM",  Domains::EXTLIB_ENERGY_DOMAIN_ROCM  },
    { "HDEEM", Domains::EXTLIB_ENERGY_DOMAIN_HDEEM },
    { "HWMON", Domains::EXTLIB_ENERGY_DOMAIN_HWMON }
};

static const std::unordered_map<unsigned int, std::string> domain_name_by_id =
    map_inverse<std::string, unsigned int>( domain_id_by_name );


static std::string
all_domain_names()
{
    std::stringstream ss;
    for ( auto item : domain_id_by_name )
    {
        ss << item.first << ", ";
    }
    return ss.str();
}


std::vector<std::string>
split_string( const std::string& str, char delim = ' ' )
{
    if ( str == "" )
    {
        return {};
    }
    size_t                   prev = 0;
    size_t                   pos  = 0;
    std::vector<std::string> split;
    while ( pos != str.npos )
    {
        pos = str.find( delim, prev );
        if ( pos != str.npos )
        {
            if ( pos - prev > 0 ) // two consecutive delimiters: do not add the empty string
            {
                split.emplace_back( str.substr( prev, pos - prev ) );
            }
        }
        else
        {
            if ( prev != str.size() ) // trailing delimiter: do not add the empty string
            {
                split.emplace_back( str.substr( prev ) );
            }
        }
        prev = pos + 1;
    }
    return split;
}


meric_plugin::meric_plugin() :
    measurement( std::chrono::microseconds( stoi( scorep::environment_variable::get( "INTERVAL_US", "50000" ) ) ) ),
    energy_domains(
{
    0
} )
{
    scorep::plugin::log::set_min_severity_level( nitro::log::severity_level::debug );

    logging::debug() << "Measurement interval: " << measurement.interval().count() << "microseconds";

    // Parse the DOMAINS environment variable for the domains the user requests
    std::string               env_requested_domains = scorep::environment_variable::get( "DOMAINS", "ALL" );
    std::vector<unsigned int> requested_domains;
    if ( env_requested_domains == "" )
    {
        logging::warn() << "No energy domains requested. Set " << scorep::environment_variable::name( "DOMAINS" ) << " to " << all_domain_names() << " or ALL";
    }
    else if ( env_requested_domains == "ALL" )
    {
        for ( auto item : domain_id_by_name )
        {
            requested_domains.emplace_back( item.second );
        }
    }
    else // expecting a comma-separated list of energy domains
    {
        for ( std::string name : split_string( env_requested_domains, ',' ) )
        {
            const auto it = domain_id_by_name.find( name );
            if ( it != domain_id_by_name.end() )
            {
                requested_domains.emplace_back( it->second );
            }
            else
            {
                logging::warn() << "Unsupported domain name '" << name << "' in " << scorep::environment_variable::name( "DOMAINS" ) << " Set to " << all_domain_names() << " or ALL";
            }
        }
    }

    // Try to enable the requested domains
    for ( const unsigned int domain : requested_domains )
    {
        EXTLIB_ENERGY_ENABLE_DOMAIN( energy_domains, domain );
    }
    extlib_init( &energy_domains, /*is_detailed = */ true );
    // Warn if any requested domains could not be enabled
    for ( const unsigned int domain : requested_domains )
    {
        if ( not EXTLIB_ENERGY_HAS_DOMAIN( energy_domains, domain ) )
        {
            logging::warn() << "Domain '" << domain_name_by_id.at( domain ) << "' was requested but could not be enabled";
        }
    }

    // Try to read an energy timestamp, which contains information on the available
    // counters for each domain.
    ExtlibEnergyTimeStamp* ts = extlib_read_energy_measurements( &energy_domains );
    if ( ts == nullptr )
    {
        // Reading the timestamp failed: The plug-in is not usable.
        throw std::runtime_error( "Could not read an energy timestamp with MERIC" );
    }
    // Store names of all available counters for all enabled domains
    for ( const auto& domain: ts->domain_data )
    {
        if ( EXTLIB_ENERGY_HAS_DOMAIN( energy_domains, domain.domain_id ) )
        {
            counters_by_domain[ domain.domain_id ] = {};
            counters_by_domain[ domain.domain_id ].reserve( domain.arr_size );
            for ( unsigned counter_id = 0; counter_id < domain.arr_size; ++counter_id )
            {
                counters_by_domain[ domain.domain_id ].emplace_back( domain.counter_name[ counter_id ] );
            }
        }
    }
    // Debug output
    unsigned int num_available_counters = 0;
    for ( auto it : counters_by_domain )
    {
        std::stringstream ss;
        for ( auto counter: it.second )
        {
            ss << counter << ", ";
            ++num_available_counters;
        }
        logging::debug() << " Available " << domain_name_by_id.at( it.first ) << " counters: " << ss.str();
    }
    if ( num_available_counters == 0 )
    {
        logging::warn() << "No counters are available";
    }
}

meric_plugin::~meric_plugin()
{
    extlib_close( &energy_domains );
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
