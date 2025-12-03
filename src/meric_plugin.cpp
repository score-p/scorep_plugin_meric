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

#include <chrono>


using scorep::plugin::logging;


static std::string
all_domain_names()
{
    std::stringstream ss;
    for ( auto item : ExtlibWrapper::domain_id_by_name )
    {
        ss << item.first << ", ";
    }
    return ss.str();
}


std::vector<unsigned int>
meric_plugin::requested_domain_names( std::string env_str )
{
    // Parse the DOMAINS environment variable for the domains the user requests
    std::vector<unsigned int> requested_domains;
    if ( env_str == "" )
    {
        logging::warn() << "No energy domains requested. Set " << scorep::environment_variable::name( "DOMAINS" ) << " to " << all_domain_names() << " or ALL";
        return {};
    }
    if ( env_str == "ALL" )
    {
        for ( auto item : ExtlibWrapper::domain_id_by_name )
        {
            requested_domains.emplace_back( item.second );
        }
        return requested_domains;
    }
    // expecting a comma-separated list of energy domains
    for ( std::string name : split_string( env_str, ',' ) )
    {
        const auto it = ExtlibWrapper::domain_id_by_name.find( name );
        if ( it != ExtlibWrapper::domain_id_by_name.end() )
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


meric_plugin::meric_plugin() :
    measurement( std::chrono::microseconds( stoi( scorep::environment_variable::get( "INTERVAL_US", "50000" ) ) ) )
{
    logging::debug() << "Measurement interval: " << measurement.interval().count() << " microseconds";

    std::string               env_requested_domains = scorep::environment_variable::get( "DOMAINS", "ALL" );
    std::vector<unsigned int> requested_domains     = requested_domain_names( env_requested_domains );
    for ( auto id : requested_domains )
    {
        logging::debug() << "Requested " << id << " , " << ExtlibWrapper::domain_name_by_id.at( id );
    }
    this->extlib         = ExtlibWrapper( requested_domains );
    this->domain_by_name = this->extlib.query_available_counters();


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
    measurement.start( std::move( this->extlib ), get_handles() );
}


void
meric_plugin::stop()
{
    this->extlib = measurement.stop();
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


std::unordered_map<std::string, domain_info> meric_plugin::available_domains_and_counters()
{
    std::vector<unsigned int> requested_domains = requested_domain_names( "ALL" );
    return ExtlibWrapper( requested_domains ).query_available_counters();
}



SCOREP_METRIC_PLUGIN_CLASS( meric_plugin, "meric" )
