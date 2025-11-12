/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include "energy_metric.h"

#include <chrono>
#include <vector>
#include <unordered_map>
#include <thread>

class meric_measurement
{
public:
    meric_measurement( std::chrono::microseconds interval );

    void
    start( const std::vector<energy_metric>& handles );

    void
    stop();

    std::vector<TVPair>&
    readings( energy_metric& handle );

private:
    void
    collect_readings();

    std::unordered_map<std::reference_wrapper<energy_metric>,
                       std::vector<TVPair>,
                       std::hash<energy_metric>,
                       std::equal_to<energy_metric> > data;

    std::thread               measurement_thread;
    bool                      active;
    std::chrono::microseconds interval;
};
