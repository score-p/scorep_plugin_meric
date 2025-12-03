/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include <meric_ext.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "utils.h"

const std::unordered_map<std::string, unsigned int> domain_id_by_name = {
    { "A64FX", ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_A64FX },
    { "RAPL",  ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_RAPL  },
    { "NVML",  ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_NVML  },
    { "ROCM",  ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_ROCM  },
    { "HDEEM", ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_HDEEM },
    { "HWMON", ExtlibEnergy::Domains::EXTLIB_ENERGY_DOMAIN_HWMON }
};


const std::unordered_map<unsigned int, std::string> domain_name_by_id =
    map_inverse<std::string, unsigned int>( domain_id_by_name );


struct extlib_deleter
{
    void
    operator()( ExtlibEnergy* energy_domains ) const
    {
        extlib_close( energy_domains );
    }
};


using ExtlibEnergyPtr = std::unique_ptr<ExtlibEnergy, extlib_deleter>;


ExtlibEnergyPtr
init_meric_extlib( const std::vector<unsigned int>& requested_domains );
