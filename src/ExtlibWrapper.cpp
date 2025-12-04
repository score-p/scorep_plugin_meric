/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "ExtlibWrapper.h"

#include <scorep/plugin/plugin.hpp>


using scorep::plugin::logging;

namespace MericPlugin
{
const std::unordered_map<std::string, unsigned int> ExtlibWrapper::domain_id_by_name = {
    { "A64FX", ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_A64FX },
    { "RAPL",  ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_RAPL  },
    { "NVML",  ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_NVML  },
    { "ROCM",  ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_ROCM  },
    { "HDEEM", ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_HDEEM },
    { "HWMON", ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_HWMON }
};


const std::unordered_map<unsigned int, std::string> ExtlibWrapper::domain_name_by_id =
    map_inverse<std::string, unsigned int>( ExtlibWrapper::domain_id_by_name );


std::vector<std::string> ExtlibWrapper::all_domain_names()
{
    return map_keys( ExtlibWrapper::domain_id_by_name );
}


std::vector<unsigned int> ExtlibWrapper::all_domain_ids()
{
    return map_keys( ExtlibWrapper::domain_name_by_id );
}



// We only need space for one measurement at a time, but a little bit
// extra does not hurt.
static constexpr unsigned int extlib_reserve_for_total_measurements = 3;


ExtlibWrapper::ExtlibWrapper( const std::vector<unsigned int>& requested_domains ) :
    energy_domains( ExtlibEnergyPtr( new ExtlibEnergy( { 0 } ) ) )
{
    // Try to enable the requested domains
    for ( const unsigned int domain : requested_domains )
    {
        EXTLIB_ENERGY_ENABLE_DOMAIN( *energy_domains, domain );
    }
    EXTLIB_ENERGY_USE_RESERVED_MEMORY( *energy_domains, extlib_reserve_for_total_measurements );
    extlib_init( energy_domains.get(), /*is_detailed = */ true );
    // Warn if any requested domains could not be enabled
    for ( const unsigned int domain : requested_domains )
    {
        if ( not EXTLIB_ENERGY_HAS_DOMAIN( *energy_domains, domain ) )
        {
            logging::warn() << "Domain '" << ExtlibWrapper::domain_name_by_id.at( domain ) << "' was requested but could not be enabled";
        }
    }
}


std::unordered_map<std::string, ExtlibWrapper::Domain>
ExtlibWrapper::query_enabled_domains()
{
    std::unordered_map<std::string, Domain> domain_by_name;
    // Try to read an energy timestamp, which contains information on the available
    // counters for each domain.
    ExtlibEnergyTimeStamp* ts = extlib_read_energy_measurements( energy_domains.get() );
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
            std::unordered_map<std::string, unsigned int> counter_idx_by_name;
            for ( unsigned counter_idx = 0; counter_idx < domain.arr_size; ++counter_idx )
            {
                counter_idx_by_name.emplace( domain.counter_name[ counter_idx ], counter_idx );
            }
            const auto& name = ExtlibWrapper::domain_name_by_id.at( domain.domain_id );
            domain_by_name[ name ] = {
                .id                  = domain.domain_id,
                .idx                 = domain_idx,
                .counter_idx_by_name = std::move( counter_idx_by_name )
            };
        }
    }
    return domain_by_name;
}


ExtlibWrapper::TimeStamp
ExtlibWrapper::read()
{
    return TimeStamp( extlib_read_energy_measurements( energy_domains.get() ),
                      /* deleter=*/ &extlib_free_energy_timestamp );
}


ExtlibWrapper::TimeStamp
ExtlibWrapper::calc_energy_consumption( TimeStamp begin, TimeStamp end )
{
    return TimeStamp( extlib_calc_energy_consumption( begin.get(), end.get() ),
                      /*deleter=*/ &extlib_free_energy_timestamp );
}
}
