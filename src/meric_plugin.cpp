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

std::vector<unsigned int>
meric_plugin::requested_domains_from_env() const
{
    // Parse the DOMAINS environment variable for the domains the user requests
    std::string               env_requested_domains = scorep::environment_variable::get( "DOMAINS", "ALL" );
    std::vector<unsigned int> requested_domains;
    if ( env_requested_domains == "" )
    {
        logging::warn() << "No energy domains requested. Set " << scorep::environment_variable::name( "DOMAINS" ) << " to " << all_domain_names() << " or ALL";
        return {};
    }
    if ( env_requested_domains == "ALL" )
    {
        for ( auto item : domain_id_by_name )
        {
            requested_domains.emplace_back( item.second );
        }
        return requested_domains;
    }
    // expecting a comma-separated list of energy domains
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
    return requested_domains;
}

// We only need space for one measurement at a time, but a little bit
// extra does not hurt.
static constexpr unsigned int extlib_reserve_for_total_measurements = 3;

void
meric_plugin::init_meric_extlib( const std::vector<unsigned int>& requested_domains, ExtlibEnergy* energy_domains ) const
{
    // Try to enable the requested domains
    for ( const unsigned int domain : requested_domains )
    {
        EXTLIB_ENERGY_ENABLE_DOMAIN( *energy_domains, domain );
    }
    EXTLIB_ENERGY_USE_RESERVED_MEMORY( *energy_domains, extlib_reserve_for_total_measurements );
    extlib_init( energy_domains, /*is_detailed = */ true );
    // Warn if any requested domains could not be enabled
    for ( const unsigned int domain : requested_domains )
    {
        if ( not EXTLIB_ENERGY_HAS_DOMAIN( *energy_domains, domain ) )
        {
            logging::warn() << "Domain '" << domain_name_by_id.at( domain ) << "' was requested but could not be enabled";
        }
    }
}

void
meric_plugin::finalize_meric_extlib( ExtlibEnergy* energy_domains ) const
{
    extlib_close( energy_domains );
}


std::unordered_map<std::string, domain_info>
meric_plugin::query_available_counters( ExtlibEnergy* energy_domains ) const
{
    std::unordered_map<std::string, domain_info> domain_by_name;
    // Try to read an energy timestamp, which contains information on the available
    // counters for each domain.
    ExtlibEnergyTimeStamp* ts = extlib_read_energy_measurements( energy_domains );
    if ( ts == nullptr )
    {
        // Reading the timestamp failed: The plug-in is not usable.
        throw std::runtime_error( "Could not read an energy timestamp with MERIC" );
    }
    // Store names of all available counters for all enabled domains
    for ( unsigned int domain_idx = 0; domain_idx < EXTLIB_NUM_DOMAINS; ++domain_idx )
    {
        const auto& domain = ts->domain_data[ domain_idx ];
        if ( EXTLIB_ENERGY_HAS_DOMAIN( *energy_domains, domain.domain_id ) )
        {
            std::unordered_map<std::string, unsigned int> counter_id_by_name;
            for ( unsigned counter_idx = 0; counter_idx < domain.arr_size; ++counter_idx )
            {
                counter_id_by_name.emplace( domain.counter_name[ counter_idx ], counter_idx );
            }
            const auto& name = domain_name_by_id.at( domain.domain_id );
            domain_by_name[ name ] = {
                .id                 = domain.domain_id,
                .idx                = domain_idx,
                .counter_id_by_name = std::move( counter_id_by_name )
            };
        }
    }
    return domain_by_name;
}


meric_plugin::meric_plugin() :
    measurement( std::chrono::microseconds( stoi( scorep::environment_variable::get( "INTERVAL_US", "50000" ) ) ) ),
    energy_domains(
{
    0
} )
{
    logging::debug() << "Measurement interval: " << measurement.interval().count() << " microseconds";

    std::vector<unsigned int> requested_domains = requested_domains_from_env();
    init_meric_extlib( requested_domains, &this->energy_domains );
    this->domain_by_name = query_available_counters( &this->energy_domains );


    // Debug output
    unsigned int num_available_counters = 0;
    for ( auto domain_it : domain_by_name )
    {
        logging::debug() << domain_it.second;
        num_available_counters += domain_it.second.counter_id_by_name.size();
    }
    if ( num_available_counters == 0 )
    {
        logging::warn() << "No counters are available";
    }
}

meric_plugin::~meric_plugin()
{
    finalize_meric_extlib( &energy_domains );
}


std::vector<scorep::plugin::metric_property>
meric_plugin::get_metric_properties( const std::string& metric_name )
{
    logging::debug() << "get metric properties called with: " << metric_name;
    std::vector<std::string>                                     domain_and_counter = split_string( metric_name, ':' );
    if ( domain_and_counter.size() != 2 )
    {
        logging::warn() << "Metric '" << metric_name << "' has the wrong format. Expected 'DOMAIN:COUNTER'";
        return {};
    }
    const std::string& domain_name  = domain_and_counter[ 0 ];
    const std::string& counter_name = domain_and_counter[ 1 ];
    const auto         domain       = this->domain_by_name.find( domain_name );
    if ( domain == this->domain_by_name.end() )
    {
        logging::warn() << "Domain '" << domain_name << "' is not enabled";
        return {};
    }
    const auto counter = domain->second.counter_id_by_name.find( counter_name );
    if ( counter == domain->second.counter_id_by_name.end() )
    {
        logging::warn() << "Counter '" << counter_name << "' is not available for domain '" << domain_name << "'";
        return {};
    }

    make_handle( metric_name, domain->second.idx, domain->second.id, domain_name, counter->second, counter_name );

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
    logging::info() << "add metric called with: " << metric.name();
}

void
meric_plugin::start()
{
    measurement.start( &this->energy_domains, get_handles() );
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
    logging::info() << "get_all_values called with: " << metric.name();

    // write the collected data to the cursor.
    for ( auto& tvpair : measurement.readings( metric ) )
    {
        cursor.write( tvpair.first, tvpair.second );
    }
}

std::ostream&
operator<<( std::ostream& os, const domain_info& domain )
{
    os << " Available " << domain_name_by_id.at( domain.id ) << " counters: ";
    for ( auto counter_it : domain.counter_id_by_name )
    {
        os << counter_it.first << ", ";
    }
    return os;
}


SCOREP_METRIC_PLUGIN_CLASS( meric_plugin, "meric" )
