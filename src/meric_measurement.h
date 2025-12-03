/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include "energy_metric.h"

#include <meric_ext.h>

#include <chrono>
#include <vector>
#include <unordered_map>
#include <thread>


struct extlib_deleter
{
    void
    operator()( ExtlibEnergy* energy_domains ) const
    {
        extlib_close( energy_domains );
    }
};

using ExtlibEnergyPtr = std::unique_ptr<ExtlibEnergy, extlib_deleter>;



class meric_measurement
{
    using TVPair = std::pair<scorep::chrono::ticks, double>;
public:
    meric_measurement( std::chrono::microseconds interval );

    void
    start( ExtlibEnergyPtr                   energy_domains,
           const std::vector<energy_metric>& handles );

    ExtlibEnergyPtr
    stop();

    std::vector<TVPair>&
    readings( energy_metric& handle );

    inline const std::chrono::microseconds
    interval() const
    {
        return _interval;
    };

private:
    void
    collect_readings();

    std::unordered_map<std::reference_wrapper<energy_metric>,
                       std::vector<TVPair>,
                       std::hash<energy_metric>,
                       std::equal_to<energy_metric> > data;

    std::thread               measurement_thread;
    bool                      active;
    std::chrono::microseconds _interval;
    ExtlibEnergyPtr           energy_domains;
};
