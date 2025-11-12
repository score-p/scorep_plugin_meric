/*
 * Copyright (c) 2015-2016, Technische Universität Dresden, Germany
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
    This file contains an example plugin written with the Score-P metric plugin C++ wrapper.

    This plugin is an asynchronous metric plugin. It will record values once on a certain thread
    within a certain process.
 */

#include <scorep/plugin/plugin.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_map>

using namespace scorep::plugin::policy;

using scorep::plugin::logging;

using TVPair = std::pair<scorep::chrono::ticks, double>;


struct energy_metric
{
    energy_metric( const std::string& name )
        : name( name ), ref( scorep::chrono::measurement_clock::now(), 0 )
    {
    }

    energy_metric( const energy_metric& ) = delete;
    /* copy-assign */
    energy_metric&
    operator=( const energy_metric& ) = delete;

    /* move constructor */
    energy_metric( energy_metric&& ) = default;
    /* move assignment */
    energy_metric&
    operator=( energy_metric&& ) = default;

    bool
    operator==( const energy_metric& other ) const
    {
        return this->name == other.name;
    }

    TVPair
    read() const
    {
        const auto timestamp = scorep::chrono::measurement_clock::now();
        return TVPair( timestamp, 10. * ( timestamp - ref.first ).count() );
    }

    std::string name;
    TVPair      ref;
};


namespace std
{
inline ostream&
operator<<( ostream& s, const energy_metric& metric )
{
    s << "(" << metric.name << ")";
    return s;
}

template <>
struct hash<energy_metric>
{
    size_t inline
    operator()( const energy_metric& metric ) const
    {
        return std::hash<std::string>{} ( metric.name );
    }
};
};



class meric_measurement
{
public:
    meric_measurement( std::chrono::microseconds interval ) : interval( interval )
    {
    }

    void
    start( const std::vector<energy_metric>& handles )
    {
        data.clear();
        for ( auto& handle : handles )
        {
            data.insert( std::make_pair( std::ref( const_cast<energy_metric&>( handle ) ),
                                         std::vector<TVPair>() ) );
        }
        active             = true;
        measurement_thread = std::thread([ this ](){
            this->collect_readings();
        } );
    }

    void
    stop()
    {
        active = false;
        if ( measurement_thread.joinable() )
        {
            measurement_thread.join();
        }
    }

    std::vector<TVPair>&
    readings( energy_metric& handle )
    {
        return data[ handle ];
    }

private:
    void
    collect_readings()
    {
        while ( active )
        {
            for ( auto& item : data )
            {
                const auto& metric   = item.first.get();
                auto&       sequence = item.second;
                sequence.emplace_back( metric.read() );
            }
            std::this_thread::sleep_for( interval );
        }
    }

    std::unordered_map<std::reference_wrapper<energy_metric>,
                       std::vector<TVPair>,
                       std::hash<energy_metric>,
                       std::equal_to<energy_metric> > data;

    std::thread               measurement_thread;
    bool                      active;
    std::chrono::microseconds interval;
};


template <typename P, typename Policies>
using meric_object_id = object_id<energy_metric, P, Policies>;

class meric_plugin : public scorep::plugin::base<meric_plugin,
                                                 async, per_host, post_mortem, scorep_clock, meric_object_id>
{
public:
    meric_plugin() :
        measurement( std::chrono::microseconds( stoi( scorep::environment_variable::get( "interval_us", "50000" ) ) ) )
    {
        scorep::plugin::log::set_min_severity_level( nitro::log::severity_level::debug );
    }

    // Convert a named metric (may contain wildcards or so) to a vector of
    // actual metrics (may have a different name)
    std::vector<scorep::plugin::metric_property>
    get_metric_properties( const std::string& metric_name )
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
    add_metric( energy_metric& metric )
    {
        logging::info() << "add metric called with: " << metric.name;
    }

    // start your measurement in this method
    void
    start()
    {
        begin = scorep::chrono::measurement_clock::now();
        measurement.start( get_handles() );
    }

    // stop your measurement in this method
    void
    stop()
    {
        end = scorep::chrono::measurement_clock::now();
        measurement.stop();
    }

    // Will be called post mortem by the measurement environment
    // You return all values measured.
    template <typename C>
    void
    get_all_values( energy_metric& metric, C& cursor )
    {
        logging::info() << "get_all_values called with: " << metric.name;

        // write the collected data to the cursor.
        for ( auto& tvpair : measurement.readings( metric ) )
        {
            cursor.write( tvpair.first, tvpair.second );
        }
    }

private:
    scorep::chrono::ticks begin, end;
    meric_measurement     measurement;
};

SCOREP_METRIC_PLUGIN_CLASS( meric_plugin, "meric" )
