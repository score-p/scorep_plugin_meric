/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "extlib_wrapper.h"

#include <scorep/plugin/plugin.hpp>

#include <iostream>

int
main()
{
    // Silence warnings and info emitted by the plugin
    scorep::plugin::log::set_min_severity_level( nitro::log::severity_level::error );

    const auto   domains     = ExtlibWrapper( ExtlibWrapper::all_domain_ids() ).query_available_counters();
    unsigned int num_domains = domains.size();

    if ( num_domains == 0 )
    {
        std::cout << "No domains could be enabled" << std::endl;
        return 0;
    }

    unsigned int num_counters = 0;
    for ( const auto& it : domains )
    {
        num_counters += it.second.counter_id_by_name.size();
        std::cout << it.second << "\n";
    }

    if ( num_counters == 0 )
    {
        std::cout << "No counters are available" << std::endl;
    }

    return 0;
}
