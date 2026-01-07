/*
 * SPDX-FileCopyrightText: (c) 2018 Technische Universität Dresden <tu-dresden.de>
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once

#include "Metric.h"
#include "ExtlibWrapper.h"

#include <scorep/chrono/chrono.hpp>

#include <chrono>
#include <thread>
#include <unordered_map>
#include <vector>


namespace MericPlugin
{
class MeasurementThread
{
    using TVPair = std::pair<scorep::chrono::ticks, double>;
public:
    MeasurementThread( std::chrono::microseconds interval );

    void
    start( ExtlibWrapper              extlib,
           const std::vector<Metric>& handles );

    ExtlibWrapper
    stop();

    std::vector<TVPair>&
    readings( Metric& handle );

    inline const std::chrono::microseconds
    interval() const
    {
        return _interval;
    };

private:
    void
    collect_readings();

    std::unordered_map<std::reference_wrapper<Metric>,
                       std::vector<TVPair>,
                       std::hash<Metric>,
                       std::equal_to<Metric> > data;

    std::thread               measurement_thread;
    bool                      active;
    std::chrono::microseconds _interval;
    ExtlibWrapper             extlib;
};
}
