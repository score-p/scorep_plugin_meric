/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "meric_extlib.h"

#include <scorep/plugin/plugin.hpp>

using scorep::plugin::logging;


// We only need space for one measurement at a time, but a little bit
// extra does not hurt.
static constexpr unsigned int extlib_reserve_for_total_measurements = 3;


ExtlibEnergyPtr
init_meric_extlib( const std::vector<unsigned int>& requested_domains )
{
    ExtlibEnergyPtr energy_domains( new ExtlibEnergy( { 0 } ) );
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
            logging::warn() << "Domain '" << domain_name_by_id.at( domain ) << "' was requested but could not be enabled";
        }
    }
    return energy_domains;
}
